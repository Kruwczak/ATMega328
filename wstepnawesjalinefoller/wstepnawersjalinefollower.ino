// --- POPRAWNE MAPOWANIE DLA ATMEGA328P DIP28 ---

// Silniki (Kierunki)
int m_prawy_A = 8;  // Fizyczna nóżka 14
int m_prawy_B = 9;  // Fizyczna nóżka 15
int m_lewy_A  = 10; // Fizyczna nóżka 16
int m_lewy_B  = 11; // Fizyczna nóżka 17

// PWM (Moc/Próbkowanie) - Te nóżki OBSŁUGUJĄ sprzętowe PWM
int en_prawy  = 5;  // Fizyczna nóżka 11 (PD5)
int en_lewy   = 6;  // Fizyczna nóżka 12 (PD6)

// Czujniki (Wejścia analogowe użyte jako cyfrowe)
int S1 = A0; // Fizyczna nóżka 23
int S2 = A1; // Fizyczna nóżka 24
int S3 = A2; // Fizyczna nóżka 25
int S4 = A3; // Fizyczna nóżka 26
int S5 = A4; // Fizyczna nóżka 27

// --- PRĘDKOŚCI ---
int V_MAX   = 180; // Pełna moc przy jeździe prosto
int V_KOREKTA = 100; // Moc przy lekkim skręcie
int V_STOP  = 0;   // Zatrzymanie koła

void setup() {
  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A, OUTPUT);  pinMode(m_lewy_B, OUTPUT);
  pinMode(en_prawy, OUTPUT);  pinMode(en_lewy, OUTPUT);
  
  pinMode(S1, INPUT); pinMode(S2, INPUT); 
  pinMode(S3, INPUT); pinMode(S4, INPUT); pinMode(S5, INPUT);
  
  delay(5000); // 5s na start
}

void loop() {
  // Odczytujemy stan wszystkich czujników (LOW = widzi białe, HIGH = widzi linię)
  // UWAGA: Jeśli Twój czujnik działa odwrotnie, zmień HIGH na LOW w warunkach.
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // LOGIKA RUCHU:
  
  if(s3 == HIGH) { 
    // Środkowy na linii -> JEDŹ PROSTO
    move(V_MAX, V_MAX);
  }
  else if(s2 == HIGH) {
    // Linia lekko po lewej -> LEKKA KOREKTA W LEWO
    move(V_KOREKTA, V_MAX);
  }
  else if(s4 == HIGH) {
    // Linia lekko po prawej -> LEKKA KOREKTA W PRAWO
    move(V_MAX, V_KOREKTA);
  }
  else if(s1 == HIGH) {
    // Linia ucieka daleko w lewo -> OSTRE CIĘCIE W LEWO
    move(V_STOP, V_MAX);
  }
  else if(s5 == HIGH) {
    // Linia ucieka daleko w prawo -> OSTRE CIĘCIE W PRAWO
    move(V_MAX, V_STOP);
  }
  else {
    // Brak linii pod czujnikami -> STOP
    move(0, 0);
  }
}

// Funkcja pomocnicza do sterowania (oba silniki zawsze do przodu)
void move(int moc_lewy, int moc_prawy) {
  digitalWrite(m_prawy_A, HIGH); digitalWrite(m_prawy_B, LOW);
  digitalWrite(m_lewy_A, HIGH);  digitalWrite(m_lewy_B, LOW);
  analogWrite(en_lewy, moc_lewy);
  analogWrite(en_prawy, moc_prawy);
}
