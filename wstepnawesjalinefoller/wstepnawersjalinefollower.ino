// --- PINY BEZ ZMIAN ---
int m_prawy_A = 8; int m_prawy_B = 9;
int m_lewy_A = 10; int m_lewy_B = 11;
int en_prawy = 5;  int en_lewy = 6;

int czujniki[] = {A0, A1, A2, A3, A4};

// --- NASTAWY PID (TUTAJ BĘDZIESZ TESTOWAĆ) ---
float Kp = 0.15;  // Odpowiada za szybkość reakcji
float Ki = 0.001; // Usuwa drobne błędy długofalowe
float Kd = 0.8;   // Tłumi oscylacje (zapobiega rybieniu)

// --- ZMIENNE POMOCNICZE ---
int blad = 0;
int poprzedni_blad = 0;
float calka = 0;
int V_BAZA = 150; // Prędkość bazowa na prostej
int ostatni_kierunek = 0; // -1 dla lewo, 1 dla prawo

void setup() {
  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A, OUTPUT); pinMode(m_lewy_B, OUTPUT);
  pinMode(en_prawy, OUTPUT); pinMode(en_lewy, OUTPUT);
  
  delay(3000); 
}

void loop() {
  int pozycja = oblicz_pozycje();

  // Jeśli nie widzi linii (wszystkie czujniki białe)
  if (pozycja == 10000) {
    szukaj_linii();
    return;
  }

  // OBLICZENIA PID
  blad = pozycja; // 0 to idealnie na środku
  calka = calka + blad;
  int rozniczka = blad - poprzedni_blad;
  
  int korekta = Kp * blad + Ki * calka + Kd * rozniczka;
  poprzedni_blad = blad;

  // Sterowanie silnikami
  int moc_lewy = V_BAZA + korekta;
  int moc_prawy = V_BAZA - korekta;

  move(moc_lewy, moc_prawy);
}

int oblicz_pozycje() {
  long suma_wag = 0;
  int aktywnych = 0;
  int wagi[] = {-2000, -1000, 0, 1000, 2000};
  
  for (int i = 0; i < 5; i++) {
    int odczyt = analogRead(czujniki[i]);
    if (odczyt > 500) { // Próg detekcji linii (dostosuj!)
      suma_wag += wagi[i];
      aktywnych++;
    }
  }

  if (aktywnych == 0) return 10000; // Kod błędu: brak linii

  // Zapamiętaj kierunek do funkcji szukaj_linii
  if (suma_wag / aktywnych < 0) ostatni_kierunek = -1;
  else if (suma_wag / aktywnych > 0) ostatni_kierunek = 1;

  return suma_wag / aktywnych;
}

void szukaj_linii() {
  // Obracaj się w miejscu w stronę, gdzie ostatnio była linia
  if (ostatni_kierunek == -1) move(-80, 80); 
  else move(80, -80);
}

void move(int ml, int mp) {
  // Ograniczenie wartości PWM do zakresu -255 do 255
  ml = constrain(ml, -255, 255);
  mp = constrain(mp, -255, 255);

  // Silnik lewy
  if (ml >= 0) {
    digitalWrite(m_lewy_A, HIGH); digitalWrite(m_lewy_B, LOW);
  } else {
    digitalWrite(m_lewy_A, LOW); digitalWrite(m_lewy_B, HIGH);
    ml = -ml;
  }

  // Silnik prawy
  if (mp >= 0) {
    digitalWrite(m_prawy_A, HIGH); digitalWrite(m_prawy_B, LOW);
  } else {
    digitalWrite(m_prawy_A, LOW); digitalWrite(m_prawy_B, HIGH);
    mp = -mp;
  }

  analogWrite(en_lewy, ml);
  analogWrite(en_prawy, mp);
}
