  #include <TinyWireM.h>                                            // I2C библиотека за ATtiny85
  #include <Tiny4kOLED.h>                                           // Библиотека за OLED дисплея
  #include <avr/interrupt.h>                                        // За използване на прекъсвания
  #include <util/delay.h>                                           // за _delay_ms()

  // Флагове за обновяване на дисплея и състояние на хронометъра
  volatile bool updateDisp = false;
  volatile bool running = false;

  // Променливи за време: стотни, секунди, минути, часове
  volatile uint8_t hundredths = 0;
  volatile uint8_t seconds = 0;
  volatile uint8_t minutes = 0;
  volatile uint8_t hours = 0;

  // Дефиниране на пиновете за бутоните
 #define BTN_START 3                                                // PB3
 #define BTN_RESET 4                                                // PB4

  void setup() 
  {
  // Настройка на пиновете за бутоните като вход с вътрешен pull-up
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  // Стартиране на I2C комуникация
  TinyWireM.begin();
  
  // Инициализация на дисплея
  oled.begin();
  oled.clear();
  oled.setFont(FONT6X8);                                             // Задаване на шрифт
  oled.setCursor(0, 0);                                              // Позиция на курсора
  oled.print("Chronometar");                                         // Начален надпис
  oled.setCursor(0, 2);
  oled.print("00:00:00:00");                                         // Начална стойност на времето
  
  // Активиране на прекъсвания по промяна на пинове PB3 и PB4
  GIMSK |= (1 << PCIE);                                              // Глобално разрешаване на прекъсвания по пинове
  PCMSK |= (1 << PCINT3) | (1 << PCINT4);                            // Разрешаване на прекъсване за PB3 и PB4

  noInterrupts();                                                    // Временно спиране на прекъсвания
  TCCR1 = 0x0B;                                                      //Настройка на Timer1: CTC режим, делител 64
  OCR1C = 99; // Сравнителна стойност за ~100 Hz (10 ms интервал)
  TIMSK |= (1 << OCIE1A);                                            // Активиране на прекъсване при съвпадение (COMPA)
  interrupts();                                                      // Разрешаване на прекъсванията
  }

  // Прекъсване при промяна на състоянието на бутоните (PB3, PB4)
  ISR(PCINT0_vect) 
  {
  static uint8_t last = 0xFF;
  _delay_ms(50);                                                     // Анти-дребезг (debounce)
  uint8_t state = PINB;                                              // Четене на текущото състояние на пиновете

  // Проверка дали бутон START е натиснат (преминаване от HIGH към LOW)
  if ((state & (1 << BTN_START)) && !(last & (1 << BTN_START)))
  running = !running;                                                // Промяна на състоянието на хронометъра (старт/пауза)

  // Проверка дали бутон RESET е натиснат
  if ((state & (1 << BTN_RESET)) && !(last & (1 << BTN_RESET))) 
  {
  // Нулиране на всички времеви стойности
  hundredths = 0;
  seconds = 0;
  minutes = 0;
  hours = 0;
  updateDisp = true;                                                // Заявка за обновяване на дисплея
  }

  last = state;                                                     // Запазване на текущото състояние за следваща проверка
  }

  // Прекъсване от таймер – активира се на всеки 10 ms (~100 Hz)
  ISR(TIMER1_COMPA_vect) 
  {
  if (running) 
  {
  hundredths++;                                                      // Увеличаване на стотни
  if (hundredths >= 100)                                             // При достигане на 100 стотни = 1 секунда
  {
  hundredths = 0;
  seconds++;
  if (seconds >= 60)                                                 // При 60 секунди = 1 минута
  {
  seconds = 0;
  minutes++;
  if (minutes >= 60)                                                 // При 60 минути = 1 час
  {
  minutes = 0;
  hours++;
  if (hours >= 100)                                                  // Ограничаваме до 99 часа
  hours = 0;                                       
  }
  }
  }
    updateDisp = true;                                               // Заявка за обновяване на дисплея
  }
  }

  void loop() 
  {
  // Ако флагът за обновяване е активен – обнови дисплея
  if (updateDisp) 
  {
  updateDisp = false;                                                // Зануляване на флага
  char buf[13];
  // Форматиране на времето в ЧЧ:ММ:СС:сс
  sprintf(buf, "%02d:%02d:%02d:%02d", hours, minutes, seconds, hundredths);
  oled.setCursor(0, 2);                                              // Позиция за отпечатване на времето
  oled.print(buf);
  }
  }
