/*
 * Copyright 2020 Neosensory, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Please note that while this Neosensory SDK has an Apache 2.0 license, 
 * usage of the Neosensory API to interface with Neosensory products is 
 * still  subject to the Neosensory developer terms of service located at:
 * https://neosensory.com/legal/dev-terms-service/
 * 
 * connect_and_vibrate.ino - Example for using
 * NeosensoryBluefruit library. Requires a Neosensory
 * Buzz and an Adafruit Feather board (or other board
 * that works with Adafruit's Bluefruit library).
 * Created by Mike V. Perrotta, January 23, 2020.
*/

#include <neosensory_bluefruit.h>

NeosensoryBluefruit NeoBluefruit;
// NeosensoryBluefruit NeoBluefruit("F2 AD 50 EA 96 31");

int motor = 0;
float intensity = 0;
float **rumble_frames;

void setup() {
  Serial.begin(9600);
  NeoBluefruit.begin();
  NeoBluefruit.setConnectedCallback(onConnected);
  NeoBluefruit.setDisconnectedCallback(onDisconnected);
  NeoBluefruit.setReadNotifyCallback(onReadNotify);
  NeoBluefruit.startScan();
  set_rumble_frames();
  while (!NeoBluefruit.isConnected() || !NeoBluefruit.isAuthorized()) {}
  NeoBluefruit.deviceInfo();
  NeoBluefruit.deviceBattery();
}

void loop() {
  if (NeoBluefruit.isConnected() && NeoBluefruit.isAuthorized()) {
    NeoBluefruit.vibrateMotor(motor, intensity);
    intensity += 0.1;
    if (intensity > 1) {
      intensity = 0;
      motor++;
      if (motor >= NeoBluefruit.num_motors()) {
        motor = 0;
        rumble();
        rumble();
        rumble();
      }
    }
    delay(50);
  }
}

void set_rumble_frames() {
  rumble_frames = new float*[NeoBluefruit.max_frames_per_bt_package()];
  for (int i = 0; i < NeoBluefruit.max_frames_per_bt_package(); i++) {
    rumble_frames[i] = new float[NeoBluefruit.num_motors()];
    for (int j = 0; j < NeoBluefruit.num_motors(); j++) {
      rumble_frames[i][j] = (i % 2) == (j % 2);
    }
  }
}

void rumble() {
  NeoBluefruit.vibrateMotors(rumble_frames, NeoBluefruit.max_frames_per_bt_package());
  delay((NeoBluefruit.max_frames_per_bt_package()) * NeoBluefruit.firmware_frame_duration());
}

/* Callbacks */

void onConnected(bool success) {
  if (!success) {
    Serial.println("Attempted connection but failed.");
    return;
  }
  Serial.println("Connected!");

  // Once we are successfully connected to the wristband,
  // send developer autherization command and commands
  // to stop sound-to-touch algorithm.
  NeoBluefruit.authorizeDeveloper();
  NeoBluefruit.acceptTermsAndConditions();
  NeoBluefruit.stopAlgorithm();
}

void onDisconnected(uint16_t conn_handle, uint8_t reason) {
  Serial.println("\nDisconnected");
}

void onReadNotify(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  for (int i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
}
