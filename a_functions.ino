void log(const char* message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void log(const __FlashStringHelper* message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void sendJson(const JsonDocument& doc) {
  if (DEBUG) {
    log("[i] sent to bluetooth:");
    serializeJsonPretty(doc, Serial);
    Serial.println();
  }
  
  serializeJson(doc, bluetooth);
  bluetooth.println();
}

void sendSignal(const char* type) {
  StaticJsonDocument<48> doc;
  doc["type"] = type;
  sendJson(doc);
}

void beginCurrentMeasuring() {
  lastCurrent = getCurrent();
}

float measureCurrent() {
  float value = getCurrent() - lastCurrent;
  return value > 0 ? value : 0;
}

float getCurrent() {
  float value = current.mA_DC();
  return value > 0 ? value : 0;
}
