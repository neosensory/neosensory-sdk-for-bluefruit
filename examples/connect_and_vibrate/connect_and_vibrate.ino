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
 * 
 * If you have trouble connecting, you can try putting 
 * your Buzz into pairing mode by holding the (+) and (-) 
 * buttons simultaneously until the blue LEDs flash.
 *
 * Created by Mike V. Perrotta, January 23, 2020.
 *
*/

#include <neosensory_bluefruit.h>

/* 
 * If there are multiple Buzz devices in proximity, 
 * you can supply a Device ID for a specific Buzz.
 * You can obtain this by connecting over USB (9600 Baud) 
 * and running the command "device info"
 */
NeosensoryBluefruit NeoBluefruit;
// NeosensoryBluefruit NeoBluefruit("F2 AD 50 EA 96 31");

int motor = 0;
float intensity = 0;
float **rumble_frames;
int led_intestity = 20;
char * colors[] = {"0xFF0000","0xFF0000", "0xFFFFFF","0x00FF00","0x00FFFF","0xFFFF00","0x0000FF"};
int num_colors = 7;
char * current_color = "0xFFFFFF";
int minusButton = 3;
int powerButton =2;
int plusButton = 1;

void setup() {
  Serial.begin(9600);
  NeoBluefruit.begin();
  NeoBluefruit.setConnectedCallback(onConnected);
  NeoBluefruit.setDisconnectedCallback(onDisconnected);
  NeoBluefruit.setReadNotifyCallback(onReadNotify);
  NeoBluefruit.setButtonPressCallback(onButtonPress);
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
  
  char * colors[] = {current_color,current_color,current_color};
  int intensities [] = {50, 50, 50, };
  // set the LEDS to white ( our starting color ) and at full glow.
  NeoBluefruit.setLeds(colors, intensities);
  // Turn the button response on and make the microphone insenstive to changes
  NeoBluefruit.setButtonResponse(1,0);
  // Set default response ( which stills includes errors ) and set to max threshold.
  NeoBluefruit.setMotorThreshold( 0, 64);
 // Set the LRA mode to closed. in closed loop this should feel like sharper vibrations.
  NeoBluefruit.setLRAMode( 1 );
  
}

void onDisconnected(uint16_t conn_handle, uint8_t reason) {
  Serial.println("\nDisconnected");
}

void onReadNotify(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  for (int i = 0; i < len; i++) {
    Serial.write(data[i]);
  }

  
  
}
/* the button values are 1 for the plus button 3 for the - button and 2 for the powerbutton 
 *  in this example we will change the LEDS by the button press power button to change colour to a random new color
 *  press + or - to change intestity of the LEDS */
void onButtonPress( int buttonID ){
  

  if( buttonID == plusButton)
  {
    Serial.println("PLUS BUTTON");
    led_intestity =led_intestity +10;
    if( led_intestity > 50 ) 
    {
      led_intestity = 50;
    }
    char * colors[] = {current_color,current_color,current_color};
    int intensities [] = {led_intestity, led_intestity, led_intestity, };
    NeoBluefruit.setLeds(colors, intensities);
  }
    if( buttonID == minusButton)
  {
      Serial.println("\n MINUS BUTTON PRESS");
    led_intestity =led_intestity - 10;
    if( led_intestity <0 ) 
    {
      led_intestity = 0;
    }
     char * colors[] = {current_color,current_color,current_color};
    int intensities [] = {led_intestity, led_intestity, led_intestity, };
    NeoBluefruit.setLeds(colors, intensities);
  }
  if(buttonID == powerButton)
  {
    
    Serial.println("\n PLUS BUTTON PRESS");
    int index = random(0,num_colors);
    current_color = colors[index];
    char * colors[] = {current_color,current_color,current_color};
    int intensities [] = {led_intestity, led_intestity, led_intestity, };
    NeoBluefruit.setLeds(colors, intensities);
  }
  
  }
  
