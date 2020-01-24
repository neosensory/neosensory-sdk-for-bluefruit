/*
  NeosensoryBluefruitExample.ino - Example for using
  NeosensoryBluefruit library. Requires a Neosensory
  Buzz and an Adafruit Feather board (or other board
  that works with Adafruit's Bluefruit library).
  
  Created by Mike V. Perrotta, January 23, 2020.
*/

#include <neosensory_bluefruit.h>

NeosensoryBluefruit NeoBluefruit("E1 D7 70 EF CA 3D");

int motor = 0;
float intensity = 0;

void setup() {
  NeoBluefruit.begin();
  NeoBluefruit.setConnectedCallback(onConnected);
  NeoBluefruit.setDisconnectedCallback(onDisconnected);
  NeoBluefruit.setReadNotifyCallback(onReadNotify);
  NeoBluefruit.startScan();
  Serial.begin(9600);
}

void loop() {
  if (NeoBluefruit.isConnected() && NeoBluefruit.isAuthenticated()) {
    NeoBluefruit.vibrateMotor(motor, intensity);
    intensity += 0.1;
    if (intensity > 1) {
      intensity = 0;
      motor++;
      if (motor >= NeoBluefruit.num_motors()) {
        motor = 0;
      }
    }
    delay(50);
  }
}

/* Callbacks */

void onConnected(bool success) {
  if (!success) {
    Serial.println("Attempted connection but failed.");
    return;
  }
  Serial.println("Connected!");

  // Once we are successfully connected to the wristband,
  // send developer authentication command and commands
  // to stop sound-to-touch algorithm.
  NeoBluefruit.authenticateWristband();
  NeoBluefruit.stopAlgorithm();
}

void onDisconnected(uint16_t conn_handle, uint8_t reason) {
  Serial.println("Disconnected");
}

void onReadNotify(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  for (int i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
}