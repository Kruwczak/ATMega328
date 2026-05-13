// --- PINY ---
int m_prawy_A = 8; int m_prawy_B = 9;
int m_lewy_A = 2;  int m_lewy_B = 3;
int en_prawy = 5;  int en_lewy = 6;

int czujniki[] = {A0, A1, A2, A3, A4};

// --- NASTAWY PID ---
float Kp = 0.05;
float Ki = 0.0;
float Kd = 0.5;

// --- ZMIENNE POMOCNICZE ---
int blad = 0;
int poprzedni_blad = 0;
float calka = 0;
int V_BAZA = 120; // 0-255
int ostatni_kierunek = 0;
int blad_przed_zguba = 0;

const int PROG_LINII = 850;

// --- ZMIENNE DO SZUKANIA LINII ---
unsigned long czas_zguby = 0;
bool szukanie_aktywne = false;
const int PREDKOSC_SZUKANIA = 100;
const unsigned long MAX_SZUKANIA = 2000;

void setup() {
  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A, OUTPUT);  pinMode(m_lewy_B, OUTPUT);
  pinMode(en_prawy, OUTPUT);  pinMode(en_lewy, OUTPUT);
  delay(3000);
}

void loop() {
  int pozycja = oblicz_pozycje();

  if (pozycja == 10000) {
    if (!szukanie_aktywne) {
      czas_zguby = millis();
      szukanie_aktywne = true;
    }
    szukaj_linii();
    return;
  }

  // Znaleźliśmy linię po szukaniu
  if (szukanie_aktywne) {
    szukanie_aktywne = false;
    calka = 0;
    poprzedni_blad = 0;

    if (blad_przed_zguba != 0 && pozycja != 0) {
      if ((blad_przed_zguba > 0) != (pozycja > 0)) {
        ostatni_kierunek = -ostatni_kierunek;
      }
    }
  }

  blad_przed_zguba = pozycja;

  blad = pozycja;
  calka = calka + blad;
  calka = constrain(calka, -3000, 3000);

  int rozniczka = blad - poprzedni_blad;
  int korekta = Kp * blad + Ki * calka + Kd * rozniczka;
  poprzedni_blad = blad;

  int moc_lewy  = V_BAZA + korekta;
  int moc_prawy = V_BAZA - korekta;

  move(moc_lewy, moc_prawy);
}

int oblicz_pozycje() {
  long suma_wag = 0;
  int aktywnych = 0;
  int wagi[] = {-2000, -1000, 0, 1000, 2000};

  for (int i = 0; i < 5; i++) {
    int odczyt = analogRead(czujniki[i]);
    if (odczyt < PROG_LINII) {
      suma_wag += wagi[i];
      aktywnych++;
    }
  }

  if (aktywnych == 0) return 10000;

  int pozycja = suma_wag / aktywnych;

  if (pozycja < 0) ostatni_kierunek = -1;
  else if (pozycja > 0) ostatni_kierunek = 1;

  return pozycja;
}

void szukaj_linii() {
  unsigned long czas_szukania = millis() - czas_zguby;

  int predkosc = PREDKOSC_SZUKANIA + (int)(czas_szukania / 20);
  predkosc = constrain(predkosc, PREDKOSC_SZUKANIA, 200);

  if (czas_szukania > MAX_SZUKANIA * 2) {
    move(0, 0);
    return;
  }

  if (czas_szukania > MAX_SZUKANIA) {
    if (ostatni_kierunek == -1) move(predkosc, -predkosc);
    else move(-predkosc, predkosc);
  } else {
    if (ostatni_kierunek == -1) move(-predkosc, predkosc);
    else move(predkosc, -predkosc);
  }

  // Gdy środkowy czujnik trafi na linię — zatrzymaj i wycentruj
  int srodek = analogRead(A2);
  if (srodek < PROG_LINII) {
    move(0, 0);
    delay(50);
    szukanie_aktywne = false;
    calka = 0;
    poprzedni_blad = 0;
  }
}

void move(int ml, int mp) {
  ml = constrain(ml, -255, 255);
  mp = constrain(mp, -255, 255);

  if (ml >= 0) {
    digitalWrite(m_lewy_A, LOW); digitalWrite(m_lewy_B, HIGH);
  } else {
    digitalWrite(m_lewy_A, HIGH); digitalWrite(m_lewy_B, LOW);
    ml = -ml;
  }

  if (mp >= 0) {
    digitalWrite(m_prawy_A, LOW); digitalWrite(m_prawy_B, HIGH);
  } else {
    digitalWrite(m_prawy_A, HIGH); digitalWrite(m_prawy_B, LOW);
    mp = -mp;
  }

  analogWrite(en_lewy, ml);
  analogWrite(en_prawy, mp);
}