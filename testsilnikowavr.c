// ============================================================
//  TEST SILNIKOW — ATmega328P  (avr-gcc, bez Arduino)
//  Kompilacja:
//    avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -O2 -o motor_test.elf motor_test.c
//    avr-objcopy -O ihex motor_test.elf motor_test.hex
//    avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:motor_test.hex
// ============================================================

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// ============================================================
//  Mapowanie pinów Arduino → porty AVR
//
//  Pin  2 → PD2      Pin  3 → PD3 (OC2B, PWM)
//  Pin  5 → PD5 (OC0B, PWM)
//  Pin  6 → PD6 (OC0A, PWM)
//  Pin  8 → PB0      Pin  9 → PB1
//  Pin 13 → PB5 (LED)
// ============================================================

// Silnik 1
#define M1_PWM_DDR   DDRD
#define M1_PWM_PORT  PORTD
#define M1_PWM_BIT   PD5        // OC0B → Timer0, kanał B

#define M1_A_DDR     DDRB
#define M1_A_PORT    PORTB
#define M1_A_BIT     PB0        // Arduino pin 8

#define M1_B_DDR     DDRB
#define M1_B_PORT    PORTB
#define M1_B_BIT     PB1        // Arduino pin 9

// Silnik 2
#define M2_PWM_DDR   DDRD
#define M2_PWM_PORT  PORTD
#define M2_PWM_BIT   PD6        // OC0A → Timer0, kanał A

#define M2_A_DDR     DDRD
#define M2_A_PORT    PORTD
#define M2_A_BIT     PD2        // Arduino pin 2

#define M2_B_DDR     DDRD
#define M2_B_PORT    PORTD
#define M2_B_BIT     PD3        // Arduino pin 3

// LED
#define LED_DDR      DDRB
#define LED_PORT     PORTB
#define LED_BIT      PB5        // Arduino pin 13

// Prędkość testowa
#define V  160

// ============================================================
//  Pomocniki GPIO
// ============================================================
static inline void pin_high(volatile uint8_t *port, uint8_t bit) { *port |=  (1 << bit); }
static inline void pin_low (volatile uint8_t *port, uint8_t bit) { *port &= ~(1 << bit); }

// ============================================================
//  Inicjalizacja Timer0 — Fast PWM, prescaler 64 (~977 Hz)
//  OC0A = PD6, OC0B = PD5
// ============================================================
static void timer0_init(void) {
    // Fast PWM, TOP = 0xFF
    TCCR0A = (1 << COM0A1) | (1 << COM0B1)   // non-inverting na obu kanałach
           | (1 << WGM01)  | (1 << WGM00);   // Fast PWM
    TCCR0B = (1 << CS01) | (1 << CS00);      // prescaler = 64
    OCR0A  = 0;
    OCR0B  = 0;
}

// ============================================================
//  analogWrite dla pinów PWM silników
//  canal: 0 = OC0A (M2_PWM/PD6), 1 = OC0B (M1_PWM/PD5)
// ============================================================
static void pwm_set(uint8_t canal, uint8_t val) {
    if (canal == 0) OCR0A = val;
    else            OCR0B = val;
}

// ============================================================
//  constrain
// ============================================================
static inline int16_t constrain_val(int16_t v, int16_t lo, int16_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ============================================================
//  Sterowanie pojedynczym silnikiem
//  canal: 0 = M2 (OC0A), 1 = M1 (OC0B)
// ============================================================
static void silnik_m1(int16_t moc) {
    moc = constrain_val(moc, -255, 255);
    if (moc > 0) {
        pin_high(&M1_A_PORT, M1_A_BIT);
        pin_low (&M1_B_PORT, M1_B_BIT);
    } else if (moc < 0) {
        pin_low (&M1_A_PORT, M1_A_BIT);
        pin_high(&M1_B_PORT, M1_B_BIT);
        moc = -moc;
    } else {
        pin_low(&M1_A_PORT, M1_A_BIT);
        pin_low(&M1_B_PORT, M1_B_BIT);
    }
    pwm_set(1, (uint8_t)moc);   // OC0B = PD5
}

static void silnik_m2(int16_t moc) {
    moc = constrain_val(moc, -255, 255);
    if (moc > 0) {
        pin_high(&M2_A_PORT, M2_A_BIT);
        pin_low (&M2_B_PORT, M2_B_BIT);
    } else if (moc < 0) {
        pin_low (&M2_A_PORT, M2_A_BIT);
        pin_high(&M2_B_PORT, M2_B_BIT);
        moc = -moc;
    } else {
        pin_low(&M2_A_PORT, M2_A_BIT);
        pin_low(&M2_B_PORT, M2_B_BIT);
    }
    pwm_set(0, (uint8_t)moc);   // OC0A = PD6
}

// ============================================================
//  Manewry
// ============================================================
static void jazda_wprzod(uint8_t v) { silnik_m1( v); silnik_m2( v); }
static void jazda_wtyl  (uint8_t v) { silnik_m1(-v); silnik_m2(-v); }
static void skret_prawy (uint8_t v) { silnik_m1( v); silnik_m2( v / 3); }
static void skret_lewy  (uint8_t v) { silnik_m1( v / 3); silnik_m2( v); }
static void obrot_prawy (uint8_t v) { silnik_m1( v); silnik_m2(-v); }
static void obrot_lewy  (uint8_t v) { silnik_m1(-v); silnik_m2( v); }
static void zatrzymaj   (void)      { silnik_m1( 0); silnik_m2( 0); }

// ============================================================
//  delay w ms  (_delay_ms akceptuje stałą; pętla dla wartości
//  większych niż ~262 ms przy 16 MHz)
// ============================================================
static void delay_ms(uint16_t ms) {
    while (ms--) _delay_ms(1);
}

// ============================================================
//  main
// ============================================================
int main(void) {
    // Kierunki pinów
    M1_PWM_DDR |= (1 << M1_PWM_BIT);
    M1_A_DDR   |= (1 << M1_A_BIT);
    M1_B_DDR   |= (1 << M1_B_BIT);

    M2_PWM_DDR |= (1 << M2_PWM_BIT);
    M2_A_DDR   |= (1 << M2_A_BIT);
    M2_B_DDR   |= (1 << M2_B_BIT);

    LED_DDR    |= (1 << LED_BIT);

    timer0_init();

    // Mrugnięcie LED x3
    for (uint8_t i = 0; i < 3; i++) {
        pin_high(&LED_PORT, LED_BIT); delay_ms(200);
        pin_low (&LED_PORT, LED_BIT); delay_ms(200);
    }
    delay_ms(1500);

    // Pętla główna
    while (1) {
        pin_high(&LED_PORT, LED_BIT); jazda_wprzod(V); delay_ms(1500);
        zatrzymaj();                                    delay_ms(500);

        pin_low (&LED_PORT, LED_BIT); jazda_wtyl(V);   delay_ms(1500);
        zatrzymaj();                                    delay_ms(500);

        pin_high(&LED_PORT, LED_BIT); skret_prawy(V);  delay_ms(800);
        zatrzymaj();                                    delay_ms(500);

        pin_low (&LED_PORT, LED_BIT); skret_lewy(V);   delay_ms(800);
        zatrzymaj();                                    delay_ms(500);

        pin_high(&LED_PORT, LED_BIT); obrot_prawy(V);  delay_ms(600);
        zatrzymaj();                                    delay_ms(500);

        pin_low (&LED_PORT, LED_BIT); obrot_lewy(V);   delay_ms(600);
        zatrzymaj();                                    delay_ms(2000);
    }
}
