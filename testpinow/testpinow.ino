// ZMIEŃ TE DWIE LICZBY ABY PRZETESTOWAĆ INNE PINY:
const int PIN_WEJSCIOWY = 2;  // Pin, do którego dotykasz kabelkiem z GND
const int PIN_WYJSCIOWY = 13; // Pin, do którego podłączasz diodę LED

void setup() {
  // Ustawiamy pin wejściowy z wbudowanym rezystorem podciągającym (bezpieczniejsze)
  pinMode(PIN_WEJSCIOWY, INPUT_PULLUP);
  
  // Ustawiamy pin wyjściowy dla diody
  pinMode(PIN_WYJSCIOWY, OUTPUT);
}

void loop() {
  // Odczytujemy stan pinu wejściowego
  int stanWejscia = digitalRead(PIN_WEJSCIOWY);

  // Ponieważ używamy INPUT_PULLUP, pin ma stan WYSOKI (HIGH), gdy nic nie jest podłączone.
  // Gdy dotkniesz nim do GND, stan zmieni się na NISKI (LOW).
  // Odwracamy ten sygnał (!stanWejscia), aby dioda zapaliła się po dotknięciu GND.
  
  if (stanWejscia == LOW) {
    digitalWrite(PIN_WYJSCIOWY, HIGH); // Włącz diodę
  } else {
    digitalWrite(PIN_WYJSCIOWY, LOW);  // Wyłącz diodę
  }
}
