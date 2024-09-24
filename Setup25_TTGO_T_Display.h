#include <Wire.h>                  // Подключаем библиотеку для работы с I2C
#include <ScioSense_ENS160.h>      // Библиотека для работы с ENS160
#include <TFT_eSPI.h>              // Библиотека TFT_eSPI для работы с экраном
#include <SD.h>                    // Библиотека для работы с SD-картой
#include <SPI.h>                   // Для работы с SD по SPI
#include <Adafruit_AHTX0.h>

// Определяем пины для SD-карты
#define SD_CS 33
#define SD_SCK 25
#define SD_MOSI 26
#define SD_MISO 27

TFT_eSPI tft = TFT_eSPI();         // Создаем объект для работы с дисплеем
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Объект для работы с ENS160
Adafruit_AHTX0 aht;
File dataFile;                     // Переменная для работы с файлом

void setup() {
  Serial.begin(115200);
  // Инициализация дисплея
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  // Инициализация I2C
  Wire.begin(21, 22);  // SDA на 21 пине, SCL на 22 пине

  // Инициализация ENS160
  Serial.print("ENS160...");
  ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");
  if (ens160.available()) {
    // Print ENS160 versions
    Serial.print("\tRev: ");
    Serial.print(ens160.getMajorRev());
    Serial.print(".");
    Serial.print(ens160.getMinorRev());
    Serial.print(".");
    Serial.println(ens160.getBuild());

    Serial.print("\tStandard mode ");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
  }
  
  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  // Инициализация SPI для SD-карты
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Инициализация SD-карты
  if (!SD.begin(SD_CS)) {
    tft.setCursor(10, 30);
    tft.println("SD card failed :(");
    while (1);
  }
  tft.setCursor(10, 30);
  tft.println("SD card initialized");

  // Создание файла и запись заголовков для CSV
  dataFile = SD.open("/data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,eCO2,TVOC,HP0,HP1,HP2,HP3,TEMP,HUM");
    dataFile.close();
  }
}

void loop() {
  if (ens160.available()) {
    ens160.measure(true);
    ens160.measureRaw(true);

    // Получаем данные с ENS160
    uint16_t ensCO2 = ens160.geteCO2();    // Получаем уровень CO2
    uint16_t ensTVOC = ens160.getTVOC();   // Получаем уровень TVOC
    uint32_t ensHR0 = ens160.getHP0();     // Получаем уровень HP0
    uint32_t ensHR1 = ens160.getHP1();     // Получаем уровень HP1
    uint32_t ensHR2 = ens160.getHP2();     // Получаем уровень HP2
    uint32_t ensHR3 = ens160.getHP3();     // Получаем уровень HP3

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    int8_t temperature = temp.temperature;
    int hum = humidity.relative_humidity;

    // Очищаем область вывода для обновления данных
    tft.fillRect(0, 0, 240, 140, TFT_BLACK);

    // Вывод данных с ENS160 на одной строке
    tft.setCursor(15, 10);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);  // Названия переменных синим цветом
    tft.print("eCO2:  ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // Значения зелёным цветом
    tft.printf("%d", ensCO2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("       TVOC: ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", ensTVOC);
    
    // Вывод данных HP0 и HP1 на одной строке
    tft.setCursor(15, 35);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("HP0:   ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", ensHR0);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("     HP1:  ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", ensHR1);

    // Вывод данных HP2 и HP3 на одной строке
    tft.setCursor(15, 60);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("HP2:   ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", ensHR2);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("     HP3:  ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", ensHR3);

        // Вывод данных temp и humidity на одной строке
    tft.setCursor(15, 85);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("TEMP:  ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", temperature);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.print("        RH:   ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.printf("%d", hum);


    // Запись данных на SD-карту в формате CSV (в новый ряд)
    dataFile = SD.open("/data.csv", FILE_APPEND);
    if (dataFile) {
      // Записываем данные в строку CSV: время и значения датчиков
      dataFile.printf("%lu,%d,%d,%d,%d,%d,%d,%d,%d\n", millis() / 1000, ensCO2, ensTVOC, ensHR0, ensHR1, ensHR2, ensHR3, temperature, hum);
      dataFile.close();  // Закрываем файл после записи
    } else {
      tft.setCursor(10, 85);
      tft.println("Error opening file");
    }
    delay(1000);  // Обновляем данные каждую секунду
  }
}
