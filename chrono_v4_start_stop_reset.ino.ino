#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BTN_START 3  // PB3 (pin 2)
#define BTN_RESET 4  // PB4 (pin 3)

volatile bool running = false;
volatile bool updateDisp = false;

volatile uint8_t hundredths = 0;
volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0;

void setup() {
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  TinyWireM.begin();
  oled.begin();
  oled.clear();
  oled.setFont(FONT6X8);
  oled.setCursor(0, 0);
  oled.print("Chronometar");
  oled.setCursor(0, 2);
  oled.print("00:00:00:00");

  // Прекъсване по пинове
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT3) | (1 << PCINT4);

  // Таймерна настройка: CTC, clk/64, 100 Hz
  noInterrupts();
  TCCR1 = 0x0B;
  OCR1C = 99;
  TIMSK |= (1 << OCIE1A);
  interrupts();
}

// Прекъсване при натискане на бутон
ISR(PCINT0_vect) {
  static uint8_t last = 0xFF;
  _delay_ms(50);  // анти-дребезг

  uint8_t state = PINB;

  if ((state & (1 << BTN_START)) && !(last & (1 << BTN_START))) {
    running = !running;  // Превключване
  }

  if ((state & (1 << BTN_RESET)) && !(last & (1 << BTN_RESET))) {
    hundredths = 0;
    seconds = 0;
    minutes = 0;
    hours = 0;
    updateDisp = true;
  }

  last = state;
}

// Прекъсване от таймера
ISR(TIMER1_COMPA_vect) {
  if (running) {
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
    updateDisp = true;
  }
}

void loop() {
  if (updateDisp) {
    updateDisp = false;
    char buf[13];
    sprintf(buf, "%02d:%02d:%02d:%02d", hours, minutes, seconds, hundredths);
    oled.setCursor(0, 2);
    oled.print(buf);
  }
}
