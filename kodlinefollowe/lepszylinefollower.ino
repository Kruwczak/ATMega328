// ==========================================
//   LINE FOLLOWER PID – ATmega328P
//   Arduino IDE
// ==========================================

// --- PINY ---
const int m_prawy_A = 8,  m_prawy_B = 9;
const int m_lewy_A  = 10, m_lewy_B  = 11;
const int en_prawy  = 5,  en_lewy   = 6;
const int czujniki[] = {A0, A1, A2, A3, A4};

// ==========================================
//   NASTAWY – tutaj grzeb przy strojeniu
// ==========================================
float Kp = 0.15;
float Ki = 0.001;
float Kd = 0.8;

const int V_BAZA     = 150;   // prędkość na prostej (0-255)
const int V_MIN      = 60;    // minimum na ostrym zakręcie
const int PROG_LINII = 500;   // próg ADC: >500 = czarna linia zmieniać przy testach od 0-1023

// Anti-windup: ograniczenie całki
const float CALKA_MAX = 3000.0;

// Wygładzanie: ile ostatnich pozycji uśredniamy
const int SMOOTH_N = 3;

// ==========================================
//   ZMIENNE GLOBALNE
// ==========================================
float blad           = 0;
float poprzedni_blad = 0;
float calka          = 0;

int ostatni_kierunek = 1;  // 1=prawo, -1=lewo

// Bufor wygładzania
float historia[SMOOTH_N] = {0};
int   hist_idx = 0;

// Stan szukania linii
enum Stan { JAZDA, SZUKANIE_FAZA1, SZUKANIE_FAZA2 };
Stan stan = JAZDA;
unsigned long czas_fazy = 0;
int szukanie_krok = 0;

// ==========================================
void setup() {
  Serial.begin(9600);   // do debugowania – możesz wyłączyć

  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A,  OUTPUT); pinMode(m_lewy_B,  OUTPUT);
  pinMode(en_prawy,  OUTPUT); pinMode(en_lewy,   OUTPUT);

  move(0, 0);
  delay(3000);  // czas na ustawienie robota
}

// ==========================================
void loop() {
  float pozycja = oblicz_pozycje();

  if (pozycja == 10000) {
    // Zgubiliśmy linię
    if (stan == JAZDA) {
      stan = SZUKANIE_FAZA1;
      czas_fazy = millis();
      szukanie_krok = 0;
      calka = 0;  // resetuj całkę przy zgubieniu
    }
    szukaj_linii();
    return;
  }

  // Wróciliśmy na linię
  stan = JAZDA;

  // --- Wygładzanie pozycji (średnia krocząca) ---
  historia[hist_idx] = pozycja;
  hist_idx = (hist_idx + 1) % SMOOTH_N;
  float poz_wygl = 0;
  for (int i = 0; i < SMOOTH_N; i++) poz_wygl += historia[i];
  poz_wygl /= SMOOTH_N;

  // --- PID ---
  blad = poz_wygl;
  calka += blad;
  // Anti-windup
  calka = constrain(calka, -CALKA_MAX, CALKA_MAX);

  float rozniczka = blad - poprzedni_blad;
  float korekta   = Kp * blad + Ki * calka + Kd * rozniczka;
  poprzedni_blad  = blad;

  // --- Dynamiczna prędkość: wolniej przy dużej korekcie ---
  int predkosc = V_BAZA - (int)(0.04 * abs(korekta));
  predkosc = max(predkosc, V_MIN);

  int moc_lewy  = predkosc + (int)korekta;
  int moc_prawy = predkosc - (int)korekta;

  move(moc_lewy, moc_prawy);
}

// ==========================================
//   OBLICZ POZYCJĘ (ważone czujniki)
// ==========================================
float oblicz_pozycje() {
  const int wagi[] = {-2000, -1000, 0, 1000, 2000};
  long  suma_wag   = 0;
  int   aktywnych  = 0;

  for (int i = 0; i < 5; i++) {
    if (analogRead(czujniki[i]) > PROG_LINII) {
      suma_wag += wagi[i];
      aktywnych++;
    }
  }

  if (aktywnych == 0) return 10000;

  float wynik = (float)suma_wag / aktywnych;

  // Zapamiętaj kierunek do szukania
  if      (wynik < -200) ostatni_kierunek = -1;
  else if (wynik >  200) ostatni_kierunek =  1;

  return wynik;
}

// ==========================================
//   SZUKANIE LINII – trójfazowe
//
//   Faza 1: cofnij chwilę (robot mógł przejechać)
//   Faza 2: obracaj się w ostatni znany kierunek,
//           stopniowo zwiększając czas (coraz szerszy łuk)
// ==========================================
void szukaj_linii() {
  unsigned long teraz = millis();

  if (stan == SZUKANIE_FAZA1) {
    // Cofnij przez 200 ms
    move(-80, -80);
    if (teraz - czas_fazy > 200) {
      stan = SZUKANIE_FAZA2;
      czas_fazy = teraz;
      szukanie_krok = 0;
    }
    return;
  }

  if (stan == SZUKANIE_FAZA2) {
    // Obracaj w stronę ostatniej linii
    // Co 300 ms zwiększamy "energię" obrotu (szerszy łuk)
    int obrot_czas[] = {300, 400, 500, 600, 800, 1000};
    int obrot_moc    = 90 + szukanie_krok * 15;
    obrot_moc = min(obrot_moc, 180);

    if (ostatni_kierunek == -1)
      move(-obrot_moc, obrot_moc);
    else
      move(obrot_moc, -obrot_moc);

    int limit = (szukanie_krok < 6) ? obrot_czas[szukanie_krok] : 1000;

    if (teraz - czas_fazy > (unsigned long)limit) {
      czas_fazy = teraz;
      szukanie_krok++;

      // Po 6 krokach bez linii – stój (zabezpieczenie)
      if (szukanie_krok > 8) {
        move(0, 0);
        delay(2000);
        szukanie_krok = 0;
        ostatni_kierunek = -ostatni_kierunek; // spróbuj w drugą stronę
      }
    }
  }
}

// ==========================================
//   STEROWANIE SILNIKAMI
// ==========================================
void move(int ml, int mp) {
  ml = constrain(ml, -255, 255);
  mp = constrain(mp, -255, 255);

  // Silnik lewy
  if (ml >= 0) { digitalWrite(m_lewy_A, HIGH); digitalWrite(m_lewy_B, LOW);  }
  else         { digitalWrite(m_lewy_A, LOW);  digitalWrite(m_lewy_B, HIGH); ml = -ml; }

  // Silnik prawy
  if (mp >= 0) { digitalWrite(m_prawy_A, HIGH); digitalWrite(m_prawy_B, LOW);  }
  else         { digitalWrite(m_prawy_A, LOW);  digitalWrite(m_prawy_B, HIGH); mp = -mp; }

  analogWrite(en_lewy,  ml);
  analogWrite(en_prawy, mp);
}
