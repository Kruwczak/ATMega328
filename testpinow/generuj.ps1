$piny = 2..13 # Zakres pinów cyfrowych (bez RX/TX)

foreach ($pinWe in $piny) {
    foreach ($pinWy in $piny) {
        # Skipujemy, jeśli wejście i wyjście to ten sam pin
        if ($pinWe -eq $pinWy) { continue }

        $nazwa = "we$pinWe" + "wy$pinWy"
        $folderName = "temp_$nazwa"
        
        # 1. Tworzymy folder projektu
        if (!(Test-Path $folderName)) { New-Item -ItemType Directory -Path $folderName }

        # 2. Tworzymy kod programu
        $kod = @"
void setup() {
  pinMode($pinWe, INPUT_PULLUP); // Wejście z podciąganiem do VCC
  pinMode($pinWy, OUTPUT);       // Wyjście na diodę
}
void loop() {
  // Jeśli pinWe zostanie zwarty do GND (stan LOW), zapal pinWy
  if (digitalRead($pinWe) == LOW) {
    digitalWrite($pinWy, HIGH);
  } else {
    digitalWrite($pinWy, LOW);
  }
}
"@
        # Zapisujemy plik .ino
        $kod | Out-File -FilePath "$folderName\$folderName.ino" -Encoding utf8

        Write-Host "Kompiluję: $nazwa" -ForegroundColor Cyan

        # 3. Kompilacja do folderu 'kombinacje_hex'
        .\arduino-cli compile --fqbn arduino:avr:uno --output-dir "./kombinacje_hex/$nazwa" $folderName
        
        # 4. Sprzątanie folderu tymczasowego
        Remove-Item -Recurse -Force $folderName
    }
}

Write-Host "ZAKOŃCZONO! Wszystkie kombinacje są w folderze kombinacje_hex" -ForegroundColor Green