# senseBoxMCU V2

# Erste Schritte

## Vorbereitung der Arduino IDE
Für den ESP32 werden von der Arduino-DIE zusätzliche Repositories benötigt:
Unter Datei -> Voreinstellungen muss unter Zusätzliche Boardverwalter-URLs 
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
```
eingetragen werden. Anschließend muss unter Werkzeuge  Board: …  Boardverwalter… das Tool esp32 by Espressif Systems installiert werden.
Nachdem alles geladen wurde, muss man unter Werkzeuge das Board ESP32S2 Dev Module auswählen und USB CDC On Boot auf Enable stellen.
Zur Nutzung des UF2-Bootloaders müssen die UF2-Dateien von der Arduino-DIE erzeugt werden. Eine ausführliche Anleitung, wie die Arduino-IDE hierfür angepasst werden muss, findet sich unter
https://blog.adafruit.com/2021/09/17/automated-generation-of-bin-hex-uf2-files-for-arduino-continuous-integration/

### Installieren des UF2-Bootloaders
Eine Anleitung zur Installation des UF2-Bootloaders findet sich unter:
https://learn.adafruit.com/adafruit-metro-esp32-s2/install-uf2-bootloader

### Demo Codes


**Abstandssensor**
Es wird aus der Arduino-Bibliothek das Bifrost library for HC-SR04 Modul benötigt.
Es handelt sich hier um das angepasste Beispiel aus dem Modul, hier wurden lediglich die Pins für den IO2/IO3 Stecker angepasst und das Aktivieren der IO-Ports hinzugefügt.

**GPIO** 
Es werden keine besonderen Module benötigt.
Im Code werden die IO-Ports aktiviert und die beiden GPIOs 6 und 7 im Sekundentakt ein- und ausgeschaltet.

**IMU-Sensor ICM-20948**
Es wird zusätzlich das ICM20948_WE Modul benötigt
Dies ist das angepasste Modul aus der Beispielsammlung. Es wurden die I2C-IOs angepasst.

**Lichtsensor / Photo-Diode**
In diesem Beispiel wird die fest verbaute IN-S63DTLS analog gemessen. Besonders hier ist hier, dass PD_ENABLE Active-High ist, anstatt Active-Low wie in den anderen Beispielen.
Die Photodiode wird einmal pro Sekunde gemessen und der Bytewert ist über den seriellen Monitor von Arduino ausgebbar.
Die gemessene Spannung ist abhängig vom Photostrom der Photodiode:
V_Out=I_Photo/R
mit 
R=33kΩ
Bei normalem Umgebungslicht sollte der Bytewert etwa 200-500 betragen, bei direkter Sonneneinstrahlung liegt der Wert typisch bei 1000-2000. Bei direkter Bestrahlung mit einer weißen Hochleistungs-LED kann der Wert bis zum maximalen Wert von 4095 steigen. Die folgende Abbildung aus dem Datenblatt der Photodiode zeigt den Zusammenhang zwischen Lichtintensität und Photostrom:
 

**RGB-LED**
Es wird zusätzlich das Freenove_WS2812_Lib_for_ESP32 Modul benötigt.
Dieses Beispiel lässt die verbaute WS2812-LED im Sekundentakt alternativ rot, grün oder blau leuchten.

**SD-Card**
Das SD-Modul ist automatisch installiert.
Dieses ist das angepasste Beispiel aus dem Modul, es wurde die Aktivierung der Stromversorgung der SD-Karte hinzugefügt. Zusätzlich mussten die GPIOs für SPI angepasst werden.

**WLAN Client**
Das genutzte Wifi Modul ist automatisch installiert. 
Dieses Beispiel versucht eine WLAN-Verbindung aufzubauen und dann einen Webserver zu erreichen.

## Reset Button 
Das Board kann zurück in den Bootloader Modus gebracht werden, wenn der `Reset` Knopf gedrückt wird und der `Boot-Switch`-Button gleichzeitig gehalten wird.
