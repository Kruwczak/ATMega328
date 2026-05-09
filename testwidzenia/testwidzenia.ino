const int CZUJNIK_TESTOWY = A2; 
const int DIODA = 13;           
int PROG_LINII = 850; // Proponowana wartosc 

void setup() {
  pinMode(DIODA, OUTPUT);
}

void loop() {
  if (analogRead(CZUJNIK_TESTOWY) < PROG_LINII) {
    digitalWrite(DIODA, HIGH); // Świeci, gdy widzi CZARNĄ LINIĘ
  } else {
    digitalWrite(DIODA, LOW);  // Gaśnie, gdy widzi BIAŁE TŁO
  }
}