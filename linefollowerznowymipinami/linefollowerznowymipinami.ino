// ============================================================
//  LINE FOLLOWER — ATmega328P
//  Mapowanie pinów zgodne ze specyfikacją sprzętową
// ============================================================

// --- SILNIKI ---
// Silnik 1 (LEWY)
const uint8_t M1_PWM = 5;   // PD5 - OC0B - prędkość
const uint8_t M1_A   = 8;   // PB0 - kierunek A
const uint8_t M1_B   = 9;   // PB1 - kierunek B

// Silnik 2 (PRAWY)
const uint8_t M2_PWM = 6;   // PD6 - OC0A - prędkość
const uint8_t M2_A   = 2;   // PD2 - kierunek A
const uint8_t M2_B   = 3;   // PD3 - kierunek B

// --- CZUJNIKI LINII (analogowe) ---
const uint8_t CZUJNIKI[5] = {A0, A1, A2, A3, A4}; // PC0–PC4

// --- DIAGNOSTYKA ---
const uint8_t LED_STATUS  = 13; // PB5 - LED statusu
const uint8_t BATERIA_PIN = A5; // PC5 - odczyt napięcia baterii
const uint8_t DIAG_0      = 0;  // PD0 - dodatkowy pin diagnostyczny
const uint8_t DIAG_1      = 1;  // PD1 - dodatkowy pin diagnostyczny

// --- PROGI I KALIBRACJA ---
const int  PROG_CZUJNIKA = 500;  // Próg ADC: >500 = linia (czarna na białym)
const int  NAPIECIE_MIN  = 680;  // ~3.3 V na A5 (przy dzielniku 5:1 → ~16.5 V min)

// --- NASTAWY PID ---
float Kp = 0.15;
float Ki = 0.001;
float Kd = 0.8;

// --- ZMIENNE STEROWANIA ---
int   blad            = 0;
int   poprzedni_blad  = 0;
float calka           = 0;
int   V_BAZA          = 150;
int   ostatni_kierunek = 0; // -1 lewo, +1 prawo

// --- ZMIENNE DIAGNOSTYCZNE ---
unsigned long ostatni_czas_led = 0;
bool          led_stan         = false;

// ============================================================
void setup() {
  // Silnik 1
  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_A,   OUTPUT);
  pinMode(M1_B,   OUTPUT);

  // Silnik 2
  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_A,   OUTPUT);
  pinMode(M2_B,   OUTPUT);

  // Diagnostyka
  pinMode(LED_STATUS, OUTPUT);
  pinMode(DIAG_0, OUTPUT);
  pinMode(DIAG_1, OUTPUT);

  zatrzymaj();

  // Sygnalizacja gotowości: 3 mignięcia LED
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED_STATUS, HIGH); delay(200);
    digitalWrite(LED_STATUS, LOW);  delay(200);
  }

  delay(2000); // Czas na ustawienie robota
}

// ============================================================
void loop() {
  sprawdz_baterie();   // Ostrzeżenie LED przy niskim napięciu

  int pozycja = oblicz_pozycje();

  if (pozycja == 10000) {
    // Brak linii — szukaj w ostatnio widzianym kierunku
    digitalWrite(DIAG_0, HIGH); // Diagnostyka: tryb szukania
    szukaj_linii();
    return;
  }

  digitalWrite(DIAG_0, LOW);

  // --- PID ---
  blad         = pozycja;
  calka       += blad;
  calka        = constrain(calka, -3000, 3000); // Anti-windup całki
  int pochodna = blad - poprzedni_blad;

  int korekta = (int)(Kp * blad + Ki * calka + Kd * pochodna);
  poprzedni_blad = blad;

  // --- SILNIKI ---
  int moc_lewy  = V_BAZA + korekta;
  int moc_prawy = V_BAZA - korekta;
  move(moc_lewy, moc_prawy);
}

// ============================================================
//  OBLICZANIE POZYCJI (ważona średnia czujników)
// ============================================================
int oblicz_pozycje() {
  const int WAGI[5] = {-2000, -1000, 0, 1000, 2000};
  long suma_wag  = 0;
  int  aktywnych = 0;

  for (uint8_t i = 0; i < 5; i++) {
    if (analogRead(CZUJNIKI[i]) > PROG_CZUJNIKA) {
      suma_wag += WAGI[i];
      aktywnych++;
    }
  }

  if (aktywnych == 0) return 10000; // Kod błędu: linia zaginęła

  int pozycja = (int)(suma_wag / aktywnych);

  // Zapamiętaj kierunek na wypadek utraty linii
  if      (pozycja < 0) ostatni_kierunek = -1;
  else if (pozycja > 0) ostatni_kierunek =  1;

  return pozycja;
}

// ============================================================
//  SZUKANIE LINII
// ============================================================
void szukaj_linii() {
  if (ostatni_kierunek == -1) move(-80,  80);
  else                        move( 80, -80);
}

// ============================================================
//  STEROWANIE SILNIKAMI
//  move(ml, mp): dodatnia = do przodu, ujemna = do tyłu
// ============================================================
void move(int ml, int mp) {
  ml = constrain(ml, -255, 255);
  mp = constrain(mp, -255, 255);
  ustaw_silnik(M1_A, M1_B, M1_PWM, ml);
  ustaw_silnik(M2_A, M2_B, M2_PWM, mp);
}

void ustaw_silnik(uint8_t pinA, uint8_t pinB, uint8_t pinPWM, int moc) {
  if (moc >= 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
    moc = -moc;
  }
  analogWrite(pinPWM, (uint8_t)moc);
}

void zatrzymaj() {
  move(0, 0);
}

// ============================================================
//  MONITORING BATERII (PC5 / A5)
//  Szybkie mruganie LED gdy napięcie za niskie
// ============================================================
void sprawdz_baterie() {
  int odczyt = analogRead(BATERIA_PIN);

  if (odczyt < NAPIECIE_MIN) {
    // Szybkie mruganie co 100 ms
    unsigned long teraz = millis();
    if (teraz - ostatni_czas_led >= 100) {
      ostatni_czas_led = teraz;
      led_stan = !led_stan;
      digitalWrite(LED_STATUS, led_stan);
      digitalWrite(DIAG_1, led_stan); // Dodatkowy sygnał na PD1
    }
  } else {
    // Normalna praca: LED stale ON
    digitalWrite(LED_STATUS, HIGH);
    digitalWrite(DIAG_1, LOW);
  }
}