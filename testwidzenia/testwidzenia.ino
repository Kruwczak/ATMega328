const int czujniki[] = {A0, A1, A2, A3, A4};
const int DIODA = 13;  // <-- zmieniona nazwa
int PROG_LINII = 500;

void setup() {
  pinMode(DIODA, OUTPUT);
}

void loop() {
  int widzi = 0;
  for (int i = 0; i < 5; i++) {
    if (analogRead(czujniki[i]) > PROG_LINII) widzi++;
  }

  for (int i = 0; i < widzi; i++) {
    digitalWrite(DIODA, HIGH); delay(200);
    digitalWrite(DIODA, LOW);  delay(200);
  }
  delay(500);
}