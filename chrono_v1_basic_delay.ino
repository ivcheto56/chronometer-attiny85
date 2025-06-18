#include <TinyWireM.h>       // I2C за ATtiny85
#include <Tiny4kOLED.h>      // OLED библиотека
#include <util/delay.h>      // За _delay_ms()

// Променливи за време
uint8_t hundredths = 0;
uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;

void setup() {
  // Стартиране на I2C
  TinyWireM.begin();

  // Инициализация на OLED дисплея
  oled.begin();
  oled.clear();
  oled.setFont(FONT6X8);

  // Начален надпис
  oled.setCursor(0, 0);
  oled.print("Chronometar");

  // Извеждане на начално време
  oled.setCursor(0, 2);
  oled.print("00:00:00:00");
}

void loop() {
  // Забавяне от 10 милисекунди
  _delay_ms(10);
  hundredths++;

  // Управление на времето
  if (hundredths >= 100) {
    hundredths = 0;
    seconds++;
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours++;
        if (hours >= 100) hours = 0;
      }
    }
  }

  // Форматиране на време
  char buf[13];
  sprintf(buf, "%02d:%02d:%02d:%02d", hours, minutes, seconds, hundredths);

  // Обновяване на дисплея
  oled.setCursor(0, 2);
  oled.print(buf);
}
