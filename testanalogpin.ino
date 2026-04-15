const int PIN_POTENCJOMETR = A0; // Fizyczna nóżka 23
const int diody[] = {2, 3, 4, 5, 6}; // Fizyczne nóżki 4, 5, 6, 11, 12

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(diody[i], OUTPUT);
  }
}

void loop() {
  // Odczyt napięcia z potencjometru (0 - 1023)
  int odczyt = analogRead(PIN_POTENCJOMETR);
  
  // Zamiana na zakres 0-5 (bo mamy 5 diod)
  // To upraszcza kod: jeśli wynik to 3, zapalą się 3 diody.
  int poziom = map(odczyt, 0, 1023, 0, 5);

  for (int i = 0; i < 5; i++) {
    if (i < poziom) {
      digitalWrite(diody[i], HIGH);
    } else {
      digitalWrite(diody[i], LOW);
    }
  }
  
  delay(20); // Małe opóźnienie, żeby diody nie mrugały przy wahaniach napięcia
}
