String encryptDecryptString(String toEncrypt) {
    char key = 'K'; // Wähle einen Schlüssel für die XOR-Operation
    String output = toEncrypt;
    
    for (int i = 0; i < toEncrypt.length(); i++) {
        output[i] = toEncrypt[i] ^ key; // XOR-Operation für jedes Zeichen
    }
    
    return output;
}

// Da die Operation symmetrisch ist, kann die Funktion sowohl zum Verschlüsseln als auch zum Entschlüsseln verwendet werden
String encryptString(String toEncrypt) {
    return encryptDecryptString(toEncrypt);
}

String decryptString(String toDecrypt) {
    return encryptDecryptString(toDecrypt);
}

void splitStringByComma(String input, String &partBeforeComma, String &partAfterComma) {
    int commaIndex = input.indexOf(','); // Finde den Index des ersten Kommas
    if (commaIndex != -1) { // Wenn ein Komma gefunden wurde
        partBeforeComma = input.substring(0, commaIndex); // Alles vor dem Komma
        partAfterComma = input.substring(commaIndex + 1); // Alles nach dem Komma
    } else {
        // Falls kein Komma vorhanden ist, ist der gesamte String vor dem "Komma"
        partBeforeComma = input;
        partAfterComma = ""; // Kein Text nach dem Komma
    }
}

String buildHTMLString(){
    String html = "<html><head>";
    html += "<style>";
    html += "body { margin: 0; padding: 0; font-family: Arial, sans-serif; background-color: #f0f0f0; }";
    html += ".container { text-align: center; margin-top: 50px; }";
    html += "h1 { color: #333; }";
    html += "input[type='text'], input[type='password'] { width: 250px; padding: 10px; margin: 10px; border: 1px solid #ccc; border-radius: 5px; }";
    html += "input[type='submit'] { padding: 10px 20px; background-color: #007bff; color: #fff; border: none; border-radius: 5px; cursor: pointer; }";
    html += "input[type='submit']:hover { background-color: #0056b3; }";
    html += ".values { margin-top: 20px; }";  // Stil für die Messwert-Anzeige
    html += "</style>";
    html += "<meta charset='UTF-8'>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>senseBox</h1>";
    html += "<form action='/save' method='post'>";
    html += "<div>Neue SSID: <input type='text' name='ssid'></div>";
    html += "<div>Neues Passwort: <input type='password' name='password'></div>";
    html += "<input type='submit' value='Verbinden'>";
    html += "</form>";

    // Messwerte anzeigen
    html += "<div class='values'>";
    html += "<h2>Aktuelle Messwerte</h2>";
    html += "<p>Temperatur: " + String(temperature) + "°C</p>";
    html += "<p>Luftfeuchtigkeit: " + String(humidity) + " %</p>";
    html += "<p>Luftdruck: " + String(pressure_event.pressure) + " hPa</p>";
    html += "<p>Lichtintensität: " + String(lux) + " Lux</p>";
    html += "<p>UV-Intensität: " + String(uv) + " W/cm²</p>";
    html += "<p>Feinstaub PM1.0: " + String(m.mc_1p0) + " µg/cm³</p>";
    html += "<p>Feinstaub PM2.5: " + String(m.mc_2p5) + " µg/cm³</p>";
    html += "<p>Feinstaub PM4.0: " + String(m.mc_4p0) + " µg/cm³</p>";
    html += "<p>Feinstaub PM10: " + String(m.mc_10p0) + " µg/cm³</p>";
    html += "<p>Niederschlag (letztes Event): " + String(rg_15.getEventAccumulation()) + " mm</p>";
    html += "<p>Niederschlag (letztes Event Akkumuliert): " + String(rg_15.getRainfallIntensity()) + " mm</p>";
    html += "<p>Niederschlag (total): " + String(rg_15.getTotalAccumulation()) + " mm</p>";

    // Zusätzliche Informationen zum Status der Box
    html += "<h2>Status der Box</h2>";
    html += "<p>SD-Karte eingelegt: " + String(sdMounted) + "</p>";
    html += "<p>WiFi verbunden: " + String(wifiConnected) + "</p>";

    html += "</div>";

    html += "</div></body></html>";

    return html;
}

String buildSaveString(){
  String save_html = "<html><head>";
  save_html += "<style>";
  save_html += "body { margin: 0; padding: 0; font-family: Arial, sans-serif; background-color: #f0f0f0; }";
  save_html += ".container { text-align: center; margin-top: 50px; }";
  save_html += "h1 { color: #333; }";
  save_html += "p { margin-top: 20px; font-size: 18px; }";
  save_html += "a { text-decoration: none; color: #007bff; }";
  save_html += "</style>";
  save_html += "<meta charset='UTF-8'>";
  save_html += "</head><body>";
  save_html += "<div class='container'>";
  save_html += "<h1>Einstellungen gespeichert</h1>";
  save_html += "<p>Die neuen Einstellungen wurden erfolgreich gespeichert.</p>";
  save_html += "<p>Die senseBox startet in 5 Sekunden neu! Bitte ändere wieder dein akutelles WiFi und schaue dir die Box auf der <a href='https://opensensemap.org/explore/" + String(SENSEBOX_ID) + "'>openSenseMap</a> an.</p>";
  save_html += "<p><a href='/'>Zurück zur Startseite</a></p>";
  save_html += "<p></p>"; // SenseBox-ID wird hier dynamisch eingefügtsave_html += "</div></body></html>";
    return save_html;

}

