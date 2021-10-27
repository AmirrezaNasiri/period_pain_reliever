#include <NeoSWSerial.h>
#include <ACS712.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "HeatingPad.h"

#define DEBUG false
#define BLUETOOTH_RX_PIN 2
#define BLUETOOTH_TX_PIN 3
#define BLUETOOTH_STATE_PIN 4
#define BLUETOOTH_MAX_RX_PACKET 256
#define CURRENT_SENSOR_PIN A0
#define CURRENT_SENSOR_SENSITIVITY 66
#define PAD_A_GATE_PIN 5
#define PAD_A_TEMP_A_PIN A1
#define PAD_A_TEMP_B_PIN A2
#define PAD_B_GATE_PIN 6
#define PAD_B_TEMP_A_PIN A3
#define PAD_B_TEMP_B_PIN A4
#define PAD_C_GATE_PIN 7
#define PAD_C_TEMP_A_PIN A5
#define PAD_C_TEMP_B_PIN A6

NeoSWSerial bluetooth(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);
ACS712 current(CURRENT_SENSOR_PIN, 5.0, 1023, CURRENT_SENSOR_SENSITIVITY);
HeatingPad padA(PAD_A_GATE_PIN, PAD_A_TEMP_A_PIN, PAD_A_TEMP_B_PIN);
HeatingPad padB(PAD_B_GATE_PIN, PAD_B_TEMP_A_PIN, PAD_B_TEMP_B_PIN);
HeatingPad padC(PAD_C_GATE_PIN, PAD_C_TEMP_A_PIN, PAD_C_TEMP_B_PIN);

float lastCurrent;
volatile bool hasPendingBluetoothData;
volatile char bluetoothData[BLUETOOTH_MAX_RX_PACKET];
volatile int bluetoothDataLen = 0;

static void receiveBluetoothData(uint8_t c) {  
  if (c == '{') {
    for(int i = 0; i < BLUETOOTH_MAX_RX_PACKET; i++) {
      bluetoothData[i] = 0;
    }
    bluetoothDataLen = 0;
    hasPendingBluetoothData = false;
  }

  if (bluetoothDataLen == BLUETOOTH_MAX_RX_PACKET) {
    return;
  }

  if (bluetoothDataLen <= BLUETOOTH_MAX_RX_PACKET - 2) {
    bluetoothData[bluetoothDataLen++] = (char) c;
  }
  
  if (c == '\n' || c == '\r' || c == '}' || bluetoothDataLen == BLUETOOTH_MAX_RX_PACKET - 1) {
    hasPendingBluetoothData = true;
    bluetoothData[bluetoothDataLen++] = 0;
  }
}

void setup() {
  if (DEBUG) {
    Serial.begin(74880);
    while (!Serial) continue;
  }

  current.autoMidPoint();

  log("[*] waiting for bluetooth connection ...");

  pinMode(BLUETOOTH_STATE_PIN, INPUT);
  bluetooth.attachInterrupt(receiveBluetoothData);
  bluetooth.begin(9600);
  while (digitalRead(BLUETOOTH_STATE_PIN) == LOW) continue;

  sendSignal("ping");
  sendInitialReport();
  delay(1000);
}

void loop() {
  processIncomingCommand();
  sendPeriodicReport();

  // Control heating pads
  padA.adjustHeatingEdge();
  padB.adjustHeatingEdge();
  padC.adjustHeatingEdge();

  delay(1000);
}

void sendInitialReport() {
  log("[i] initial report:");

  beginCurrentMeasuring();
  
  StaticJsonDocument<192> json;

  json["type"] = "initial_report";

  // Calculate max current of each pad
  padA.on();
  json["pad_a_max_current"] = measureCurrent();
  padA.off();
  padB.on();
  json["pad_b_max_current"] = measureCurrent();
  padB.off();
  padC.on();
  json["pad_c_max_current"] = measureCurrent();
  padC.off();

  // Calculate max current we can draw
  padA.on();
  padB.on();
  padC.on();
  json["pad_all_max_current"] = measureCurrent();
  padA.off();
  padB.off();
  padC.off();

  sendJson(json);
}

void sendPeriodicReport() {
  log("[i] periodic report:");

  StaticJsonDocument<192> json;

  json["type"] = "periodic_report";
  json["current"] = getCurrent();
  json["pad_a_target_temp"] = padA.getTargetTemp();
  json["pad_a_temp"] = padA.getTemp();
  json["pad_a_temp_a"] = padA.getRawTemp(0);
  json["pad_a_temp_b"] = padA.getRawTemp(1);
  json["pad_b_target_temp"] = padB.getTargetTemp();
  json["pad_b_temp"] = padB.getTemp();
  json["pad_b_temp_a"] = padB.getRawTemp(0);
  json["pad_b_temp_b"] = padB.getRawTemp(1);
  json["pad_c_target_temp"] = padC.getTargetTemp();
  json["pad_c_temp"] = padC.getTemp();
  json["pad_c_temp_a"] = padC.getRawTemp(0);
  json["pad_c_temp_b"] = padC.getRawTemp(1);
  sendJson(json);
}

void processIncomingCommand() {
  if (hasPendingBluetoothData) {
    log("[i] command received: ");

    // Cast volatile char[] type to char[]
    char _bluetoothData[BLUETOOTH_MAX_RX_PACKET];
    for(int i = 0; i < BLUETOOTH_MAX_RX_PACKET; i++) {
      _bluetoothData[i] = bluetoothData[i];
    }

    hasPendingBluetoothData = false;
    
    if (DEBUG) {
      Serial.print(_bluetoothData);
    }

    StaticJsonDocument<BLUETOOTH_MAX_RX_PACKET> json;
    DeserializationError error = deserializeJson(json, _bluetoothData);
    
    if (error) {
      log("[-] error parsing json:");
      log(error.f_str());
      sendSignal("command_error");
      return;
    }
    
    if (json["type"] == "config") {
      padA.setTargetTemp(json["pad_a_target_temp"]);
      //padB.setTargetTemp(json["pad_b_target_temp"]);
      //padC.setTargetTemp(json["pad_c_target_temp"]);
    }

    sendSignal("command_done");
  }
}
