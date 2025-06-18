#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include <avr/interrupt.h>

// Променливи за време
volatile uint8_t hundredths = 0;
volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0;

volatile bool updateDisplay = false;

void setup() {
  // OLED и I2C инициализация
  TinyWireM.begin();
  oled.begin();
  oled.clear();
  oled.setFont(FONT6X8);

  oled.setCursor(0, 0);
  oled.print("Chronometar");
  oled.setCursor(0, 2);
  oled.print("00:00:00:00");

  // Настройка на таймер: CTC режим, clk/64, 100 прекъсвания в секунда
  noInterrupts();
  TCCR1 = 0x0B;        // CTC режим + предделител /64
  OCR1C = 99;          // (8 MHz / 64) / 100 = 1250 → OCR1C = 99
  TIMSK |= (1 << OCIE1A);  // Разрешаване на прекъсване при съвпадение
  interrupts();
}

// Таймерно прекъсване на всеки 10ms (100Hz)
ISR(TIMER1_COMPA_vect) {
  hundredths++;
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
  updateDisplay = true;
}

void loop() {
  if (updateDisplay) {
    updateDisplay = false;

    char buf[13];
    sprintf(buf, "%02d:%02d:%02d:%02d", hours, minutes, seconds, hundredths);

    oled.setCursor(0, 2);
    oled.print(buf);
  }
}
