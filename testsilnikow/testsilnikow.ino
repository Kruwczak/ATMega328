// ============================================================
//  TEST SILNIKOW — ATmega328P
// ============================================================

const uint8_t M1_PWM = 5;
const uint8_t M1_A   = 8;
const uint8_t M1_B   = 9;

const uint8_t M2_PWM = 6;
const uint8_t M2_A   = 2;
const uint8_t M2_B   = 3;

const uint8_t LED = 13;

const uint8_t V = 160;

// ============================================================
void setup() {
  pinMode(M1_PWM, OUTPUT); pinMode(M1_A, OUTPUT); pinMode(M1_B, OUTPUT);
  pinMode(M2_PWM, OUTPUT); pinMode(M2_A, OUTPUT); pinMode(M2_B, OUTPUT);
  pinMode(LED, OUTPUT);

  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED, HIGH); delay(200);
    digitalWrite(LED, LOW);  delay(200);
  }
  delay(1500);
}

// ============================================================
void loop() {

  digitalWrite(LED, HIGH); jazda_wprzod(V);   delay(1500);
  zatrzymaj();                                 delay(500);

  digitalWrite(LED, LOW);  jazda_wtyl(V);     delay(1500);
  zatrzymaj();                                 delay(500);

  digitalWrite(LED, HIGH); skret_prawy(V);    delay(800);
  zatrzymaj();                                 delay(500);

  digitalWrite(LED, LOW);  skret_lewy(V);     delay(800);
  zatrzymaj();                                 delay(500);

  digitalWrite(LED, HIGH); obrot_prawy(V);    delay(600);
  zatrzymaj();                                 delay(500);

  digitalWrite(LED, LOW);  obrot_lewy(V);     delay(600);
  zatrzymaj();                                 delay(2000);
}

// ============================================================
void jazda_wprzod(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM,  v);
  silnik(M2_A, M2_B, M2_PWM,  v);
}

void jazda_wtyl(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM, -v);
  silnik(M2_A, M2_B, M2_PWM, -v);
}

void skret_prawy(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM,  v);
  silnik(M2_A, M2_B, M2_PWM,  v / 3);
}

void skret_lewy(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM,  v / 3);
  silnik(M2_A, M2_B, M2_PWM,  v);
}

void obrot_prawy(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM,  v);
  silnik(M2_A, M2_B, M2_PWM, -v);
}

void obrot_lewy(uint8_t v) {
  silnik(M1_A, M1_B, M1_PWM, -v);
  silnik(M2_A, M2_B, M2_PWM,  v);
}

void zatrzymaj() {
  silnik(M1_A, M1_B, M1_PWM, 0);
  silnik(M2_A, M2_B, M2_PWM, 0);
}

// ============================================================
void silnik(uint8_t pinA, uint8_t pinB, uint8_t pinPWM, int moc) {
  moc = constrain(moc, -255, 255);
  if (moc > 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
  } else if (moc < 0) {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
    moc = -moc;
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }
  analogWrite(pinPWM, (uint8_t)moc);
}