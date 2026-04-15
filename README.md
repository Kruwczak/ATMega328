# ATmega328p - Kody i Projekty

Repozytorium zawiera zbiór programów oraz instrukcje dotyczące programowania i obsługi mikrokontrolera **ATmega328p** w wersji "gołego" układu (standalone).

## Mapowanie Pinów (Pinout)

Poniższa tabela ułatwia identyfikację pinów podczas pracy z Arduino IDE. Porównuje ona fizyczną nóżkę procesora, nazwę portu w rejestrach AVR oraz przypisany numer pinu w środowisku Arduino.

| Fizyczna nóżka | Nazwa (PORT) | Funkcja | Arduino pin |
| :---: | :--- | :--- | :---: |
| 1 | PC6 | RESET | — |
| 2 | PD0 | RX | 0 |
| 3 | PD1 | TX | 1 |
| 4 | PD2 | INT0 | 2 |
| 5 | PD3 | PWM | 3 |
| 6 | PD4 | — | 4 |
| 7 | VCC | Zasilanie (3.3V / 5V) | — |
| 8 | GND | Masa | — |
| 9 | PB6 | XTAL1 (Kwarc) | — |
| 10 | PB7 | XTAL2 (Kwarc) | — |
| 11 | PD5 | PWM | 5 |
| 12 | PD6 | PWM | 6 |
| 13 | PD7 | — | 7 |
| 14 | PB0 | — | 8 |
| 15 | PB1 | PWM | 9 |
| 16 | PB2 | SS | 10 |
| 17 | PB3 | MOSI | 11 |
| 18 | PB4 | MISO | 12 |
| 19 | PB5 | SCK | 13 |
| 20 | AVCC | Zasilanie ADC | — |
| 21 | AREF | Referencja ADC | — |
| 22 | GND | Masa | — |
| 23 | PC0 | ADC0 | A0 (14) |
| 24 | PC1 | ADC1 | A1 (15) |
| 25 | PC2 | ADC2 | A2 (16) |
| 26 | PC3 | ADC3 | A3 (17) |
| 27 | PC4 | SDA | A4 (18) |
| 28 | PC5 | SCL | A5 (19) |



## Programowanie przez Raspberry Pi (ISP)

Możesz wgrać program na "goły" procesor ATmega328p wykorzystując złącze GPIO w Raspberry Pi. Działa ono wtedy jako programator ISP.

### Schemat podłączenia (6 kabli)

Połącz piny Raspberry Pi z procesorem według poniższego schematu:

| ATmega328p (Fizyczny Pin) | Funkcja | Raspberry Pi (Pin fizyczny) | Numer GPIO |
| :---: | :--- | :---: | :---: |
| 7 | VCC (Zasilanie) | 1 | 3.3V |
| 8 lub 22 | GND (Masa) | 6 | Ground |
| 1 | RESET | 22 | GPIO 25 |
| 17 | MOSI | 19 | GPIO 10 |
| 18 | MISO | 21 | GPIO 9 |
| 19 | SCK | 23 | GPIO 11 |



### Jak wgrać program?

1.  **Przygotowanie pliku w Arduino IDE:**
    * Napisz swój kod w Arduino IDE.
    * W menu wybierz `Szkic` -> `Eksportuj skompilowany program`.
    * W folderze z Twoim projektem pojawi się plik z rozszerzeniem `.hex`.

2.  **Instalacja AVRDUDE na Raspberry Pi:**
    ```bash
    sudo apt-get update
    sudo apt-get install avrdude
    ```

3.  **Wgrywanie kodu:**
    Prześlij plik `.hex` na Raspberry Pi i użyj komendy:
    ```bash
    # Test połączenia
    sudo avrdude -p m328p -c linuxgpio -v

    # Wgrywanie pliku (zastąp 'nazwa_pliku.hex' swoją nazwą)
    sudo avrdude -p m328p -c linuxgpio -U flash:w:nazwa_pliku.hex:i
    ```

## Korzystanie z Arduino IDE
Podczas pisania kodu w Arduino IDE dla gołego procesora, pamiętaj, że standardowo większość funkcji działa tak samo jak na Arduino Uno. Jeśli Twój układ nie posiada zewnętrznego kwarcu (16MHz), będziesz musiał wgrać odpowiednie "Core" (np. *MiniCore*) i ustawić taktowanie na wewnętrzny oscylator 8MHz.

---
