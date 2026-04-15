# Zakres pinów analogowych (A0 = 14, A1 = 15, ..., A5 = 19)
$pinyAnalogowe = 14..19

# Stałe piny wyjściowe dla diod (np. Twoje 5 diod)
$pinyDiod = "2, 3, 4, 5, 6"

$outputRoot = "analogowe_hex"
if (!(Test-Path $outputRoot)) { New-Item -ItemType Directory -Path $outputRoot }

foreach ($pinAn in $pinyAnalogowe) {
    # Nazwa pliku np. test_A0, test_A1 itd.
    $analogIndex = $pinAn - 14
    $nazwa = "test_A$analogIndex"
    $tempFolder = "temp_$nazwa"

    if (!(Test-Path $tempFolder)) { New-Item -ItemType Directory -Path $tempFolder }

    $kod = @"
int diody[] = {$pinyDiod};
int liczbaDiod = 5;

void setup() {
  for (int i = 0; i < liczbaDiod; i++) {
    pinMode(diody[i], OUTPUT);
  }
}

void loop() {
  int odczyt = analogRead($pinAn);
  // Mapujemy odczyt 0-1023 na liczbe zapalonych diod 0-5
  int poziom = map(odczyt, 0, 1023, 0, liczbaDiod);

  for (int i = 0; i < liczbaDiod; i++) {
    if (i < poziom) {
      digitalWrite(diody[i], HIGH);
    } else {
      digitalWrite(diody[i], LOW);
    }
  }
  delay(20);
}
"@
    $kod | Out-File -FilePath "$tempFolder\$tempFolder.ino" -Encoding utf8

    Write-Host "Kompiluje test dla pinu A$analogIndex (Pin $pinAn)..." -ForegroundColor Cyan
    
    .\arduino-cli compile --fqbn arduino:avr:uno --output-dir "$outputRoot/$nazwa" $tempFolder

    if (Test-Path "$outputRoot/$nazwa") {
        Remove-Item -Recurse -Force $tempFolder
    }
}

Write-Host "GOTOWE! Testy wejsc analogowych sa w folderze: $outputRoot" -ForegroundColor Green