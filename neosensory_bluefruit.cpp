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
 */

/*
	NeosensoryBluefruit.cpp - Library for connecting to 
	Neosensory hardware via Adafruit's Bluefruit library.
	Created by Mike V. Perrotta, January 23, 2020.
*/

#include "Arduino.h"
#include "neosensory_bluefruit.h"
#include <Base64.h>
#include <bluefruit.h>

NeosensoryBluefruit::NeosensoryBluefruit(char device_id[], uint8_t num_motors, 
				uint8_t initial_min_vibration, uint8_t initial_max_vibration)
 : wb_service_uuid_ {
			0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
			0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E
		}
 , wb_write_char_uuid_ {
			0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
			0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E
		}
 , wb_read_char_uuid_ {
			0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
			0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E
		}
 , wb_service_(wb_service_uuid_)
 , wb_write_characteristic_(wb_write_char_uuid_)
 , wb_read_characteristic_(wb_read_char_uuid_)
{
	NeoBluefruit = this;
	setDeviceId(device_id);
	num_motors_ = num_motors;
	max_vibration = initial_max_vibration;
	min_vibration = initial_min_vibration;

	// TODO: get this from firmware rather than hardcoding
	firmware_frame_duration_ = 16;
	uint8_t mtu = 247;
	max_frames_per_bt_package_ = (uint8_t)((mtu - 17) / (num_motors_ * (4 / 3.0f)));

	previous_motor_array_ = (uint8_t*)malloc(sizeof(uint8_t) * num_motors_);
	memset(previous_motor_array_, 0, sizeof(uint8_t) * num_motors_);
	is_authorized_ = false;
}


/* Bluetooth */

void NeosensoryBluefruit::begin(void) {
	// Initialize Bluefruit with 1 central connection
	Bluefruit.begin(0, 1);
	Bluefruit.setName("Neosensory Bluefruit Central Device");
	
	// Initialize wristband client service
	wb_service_.begin();

	// Initialize wristband write client characteristic
	wb_write_characteristic_.begin();

	// Initialize wristband read client characteristic
	wb_read_characteristic_.setNotifyCallback(readNotifyCallbackWrapper);
	wb_read_characteristic_.begin();

	// Callbacks for central connect and disconnect
	Bluefruit.Central.setConnectCallback(connectCallbackWrapper);
	Bluefruit.Central.setDisconnectCallback(disconnectCallbackWrapper);

	// Set parameters for central scanner
	Bluefruit.Scanner.setRxCallback(scanCallbackWrapper);
	Bluefruit.Scanner.restartOnDisconnect(true);
	Bluefruit.Scanner.setInterval(160, 80);
	Bluefruit.Scanner.useActiveScan(false);
}

/** @brief Sets private variable device_address_ from given device_id
 *	@param[in] device_id The device_id of the hardware to connect to
 *	@note Converts a character array into an array of bytes
 */
void NeosensoryBluefruit::setDeviceAddress(char device_id[])
{
	for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
		device_address_[i] = (uint8_t)strtol(device_id, &device_id, 16);
	}
}

void NeosensoryBluefruit::setDeviceId(char new_device_id[]) {
	connect_to_any_neo_device_ = strlen(new_device_id) <= 0;
	if (!connect_to_any_neo_device_) {
		setDeviceAddress(new_device_id);
	}
}

uint8_t* NeosensoryBluefruit::getDeviceAddress(void)
{
	if (connect_to_any_neo_device_) return 0;
	return device_address_;
}

bool NeosensoryBluefruit::startScan(void)
{
	return Bluefruit.Scanner.start(0);
}

bool NeosensoryBluefruit::isConnected(void) {
	return Bluefruit.Central.connected();
}

/** @brief Checks that a report address, found during a scan,
 *  matches the address of the device NeosensoryBluefruit is searching for.
 *  @param[in] foundAddress The address found during the scan.
 *  @note Address will be reversed order from band name array.
 */
bool NeosensoryBluefruit::checkAddressMatches(uint8_t foundAddress[]) {
	for (int i = 0; i < BLE_GAP_ADDR_LEN; i++) {
		if (device_address_[i] != 
			foundAddress[BLE_GAP_ADDR_LEN - (i + 1)]) {
			return false;
		}
	}
	return true;
}

/** @brief Checks if the found BLE report belongs to a Neosensory device
 *  @param[in] report The found report
 *  @note For now, just checks that the string "Buzz" is in the 
 *  advertising data.
 */
bool NeosensoryBluefruit::checkIsNeosensory(ble_gap_evt_adv_report_t* report) {
	String advertising_data = "";
	for (int i = 7; i < report->data.len; i++) {
		advertising_data += (char)report->data.p_data[i];
	}
	bool found = advertising_data.indexOf("Buzz") != -1;
	return found;
}

/** @brief Checks if NeosensoryBluefruit should connect to the found BLE report.
 *  @param[in] report The found report
 */
bool NeosensoryBluefruit::checkDevice(ble_gap_evt_adv_report_t* report) {
	if (connect_to_any_neo_device_) {
		return checkIsNeosensory(report);
	} else {
		return checkAddressMatches(report->peer_addr.addr);
	}
}


/* CLI Commands */

bool NeosensoryBluefruit::isAuthorized(void) {
	return is_authorized_;
}

void NeosensoryBluefruit::sendCommand(char cmd[]) {
	wb_write_characteristic_.write(cmd, strlen(cmd));
}

void NeosensoryBluefruit::authorizeDeveloper(void) {
	sendCommand("auth as developer\n");
}

void NeosensoryBluefruit::acceptTermsAndConditions(void) {
	sendCommand("accept\n");
}

void NeosensoryBluefruit::stopAlgorithm(void) {
	audioStop();
	motorsStart();
}

void NeosensoryBluefruit::deviceInfo(void) {
	sendCommand("device info\n");
}

void NeosensoryBluefruit::motorsStart(void) {
	sendCommand("motors start\n");
}

void NeosensoryBluefruit::motorsStop(void) {
	sendCommand("motors stop\n");
}

void NeosensoryBluefruit::motorsClearQueue(void) {
	sendCommand("motors clear_queue\n");
}

void NeosensoryBluefruit::deviceBattery(void) {
	sendCommand("device battery_soc\n");
}

void NeosensoryBluefruit::audioStart(void) {
	sendCommand("audio start\n");
}

void NeosensoryBluefruit::audioStop(void) {
	sendCommand("audio stop\n");
}

/** @brief Looks for a JSON object in the input data, or in a combination of this data and previous data.
 */
void NeosensoryBluefruit::parseCliData(uint8_t* data, uint16_t len) {
	for (int i = 0; i < len; i++) {
		if (data[i] == '{') {
			jsonStarted_ = true;
			jsonMessage_ = "";
		}
		if (jsonStarted_) {
			jsonMessage_ += (char)data[i];
		}
		if (data[i] == '}') {
			jsonStarted_ = false;
			handleCliJson(jsonMessage_);
		}
	}
}

/** @brief Handles CLI JSON responses, by granting authorization for instance.
 *  @note This method can be adjusted to handle more response messages. For instance,
 *  this method could parse the JSON and see if it handles information about the battery level
 *  and then update a variable that holds the latest read battery level.
 */
void NeosensoryBluefruit::handleCliJson(String jsonMessage) {
	if (jsonMessage.indexOf('Developer API access granted!') != -1) {
		is_authorized_ = true;
	}
}


/* Hardware */

uint8_t NeosensoryBluefruit::num_motors(void) {
	return num_motors_;
}

uint8_t NeosensoryBluefruit::firmware_frame_duration(void) {
	return firmware_frame_duration_;
}

uint8_t NeosensoryBluefruit::max_frames_per_bt_package(void) {
	return max_frames_per_bt_package_;
}


/* Motor Control */

/** @brief Translates a linear intensity value into a 
 *	linearly perceived motor intensity value
 */
uint8_t linearIntensityToMotorSpace(
	float linear_intensity, uint8_t min_intensity, uint8_t max_intensity) {
		if (linear_intensity <= 0) {
	return 0;
	}
	if (linear_intensity >= 1) {
		return max_intensity;
	}
	return uint8_t((exp(linear_intensity) - 1) /
							 (exp(1) - 1) * (max_intensity - min_intensity) + min_intensity);
}

/** @brief Translates an array of intensities from linear space to motor space
 *	@param[in] lin_array Array of intensities from (0, 1)
 *	@param[out] motor_space_array Array of motor intensities 
 *	corresponding to the linear intensities
 *	@param[in] array_size Number of values in the arrays
 *	@note Translates an array of intensities between (0, 1) to 
 *	(THRESHOLD_INTENSITY, MAX_INTENSITY) on an exponential curve,
 *	so that each linear step in the lin_array feels like a linear
 *	change on the skin. This is due to the Weber Curve, which 
 *	shows that larger increases in intensity are needed for larger
 *	intensities than for lesser intensities, if the same 
 *	perceptual change is to be felt.
 */
void NeosensoryBluefruit::getMotorIntensitiesFromLinArray(
	float lin_array[], uint8_t motor_space_array[], size_t array_size) {
	for (int i = 0; i < array_size; i++) {
		float input = lin_array[i];
		motor_space_array[i] = linearIntensityToMotorSpace(
			input, min_vibration, max_vibration);
	}
}

/** @brief Checks if two arrays are equal
 *	@param[in] arr1 First array
 *	@param[in] arr2 Second array
 *	@param[in] arr_len Length of both arrays
 *	@return True if arrays have equal values at all indices, else False
 */
bool compareArrays(uint8_t arr1[], uint8_t arr2[], size_t arr_len) {
	for (int i = 0; i < arr_len; ++i)
	{
		if (arr1[i] != arr2[i]) {
			return false;
		}
	}
	return true;
}

/** @brief Encode an array of motor intensity byte values 
 *	into a Base64 encoded string
 *	@param[in] motor_intensities The array of motor intensities to encode
 *	@param[in] arr_len Length of the array
 *	@pararm[out] encoded_motor_intensities Char array to fill 
 *	with encoded string
 *	@return Array of characters which is the Base64 encoded string
 */
void encodeMotorIntensities(
	uint8_t* motor_intensities, size_t arr_len, char encoded_motor_intensities[]) {
	size_t input_arr_size = sizeof(uint8_t) * arr_len;
	base64_encode(
		encoded_motor_intensities, (char*)motor_intensities, input_arr_size); 
}

/** @brief Converts motor intensities to base64 encoded array and sends appropriate command
 *	@param[in] motor_intensities The motor intensities to send. If multiple frames, this
 *	is a flattened array. 
 *	@param[in] num_frames The number of frames in motor_intensities. Cannot be more 
 *	than max_frames_per_bt_package_.
 */
void NeosensoryBluefruit::sendMotorCommand(uint8_t motor_intensities[], size_t num_frames) {
	num_frames = min(max_frames_per_bt_package_, num_frames);
	char encoded_motor_intensities[base64_enc_len(sizeof(uint8_t) * num_motors_ * num_frames)];
	encodeMotorIntensities(
		motor_intensities, num_motors_ * num_frames, encoded_motor_intensities);
	sendCommand("motors vibrate ");
	sendCommand(encoded_motor_intensities);
	sendCommand("\n");
}

void NeosensoryBluefruit::vibrateMotors(float intensities[]) {
	uint8_t motor_intensities[num_motors_];
	getMotorIntensitiesFromLinArray(intensities, motor_intensities, num_motors_);

	if (compareArrays(motor_intensities, previous_motor_array_, num_motors_)) {
		return;
	}
	memcpy(previous_motor_array_, motor_intensities, sizeof(uint8_t) * num_motors_);

	sendMotorCommand(motor_intensities);
}

void NeosensoryBluefruit::vibrateMotors(float *intensities[], int num_frames) {
	num_frames = min(max_frames_per_bt_package_, num_frames);
	float flat_intensities[num_motors_ * num_frames];
	for (int i = 0; i < num_frames; ++i)
	{
		for (int j = 0; j < num_motors_; ++j)
		{
			flat_intensities[i * num_motors_ + j] = intensities[i][j];
		}
	}
	uint8_t motor_intensities[num_motors_ * num_frames];
	getMotorIntensitiesFromLinArray(flat_intensities, motor_intensities, num_motors_ * num_frames);

	sendMotorCommand(motor_intensities, num_frames);
}

void NeosensoryBluefruit::turnOffAllMotors(void) {
	float motor_intensities[num_motors_];
	memset(motor_intensities, 0, sizeof(float) * num_motors_);
	vibrateMotors(motor_intensities);
}

void NeosensoryBluefruit::vibrateMotor(uint8_t motor, float intensity) {
	float motor_intensities[num_motors_];
	memset(motor_intensities, 0, sizeof(float) * num_motors_);
	motor_intensities[motor] = intensity;
	vibrateMotors(motor_intensities);
}

/* LEDS */
void NeosensoryBluefruit::setLeds(char *colorVals[],int intensities[])
{

    static char color_vals[64];
    sprintf(color_vals, "%s %s %s", colorVals[0],colorVals[1],colorVals[2]);
    char intensity_vals[64];
    sprintf(intensity_vals,"%d %d %d", intensities[0],intensities[1],intensities[2]);
    sendCommand("leds set ");
    sendCommand(color_vals);
    sendCommand(" ");
    sendCommand(intensity_vals);
    sendCommand("\n");
}

void NeosensoryBluefruit::getLeds()
{
    sendCommand("leds get");
    sendCommand("\n");
}

/* Buttons */
void NeosensoryBluefruit::setButtonResponse(int enable, int allowSensitivity){


    static char args[5*sizeof(char)];
    sprintf(args, " %d %d ", enable, allowSensitivity);
    sendCommand("config set_buttons_response ");
     sendCommand(args);
    sendCommand("\n");
}
/* LRA Mode */
void NeosensoryBluefruit::setLRAMode( int mode ){
    static char args[2*sizeof(char)];
    sprintf(args, " %d ", mode);
    sendCommand("motors config_lra_mode");
    sendCommand(args);
    sendCommand("\n");
}
void NeosensoryBluefruit::getLRAMode(){

    sendCommand("motors get_lra_mode");
    sendCommand("\n");
}

/* Motor thresholds */
void NeosensoryBluefruit::getMotorThreshold(){
    sendCommand("motors get_threshold");
    sendCommand("\n");
}
void NeosensoryBluefruit::setMotorThreshold( int feedbackType, int threshold){


        static char args[5*sizeof(char)];
        sprintf(args, " %d %d ", feedbackType, threshold);
        sendCommand("motors config_threshold  ");
         sendCommand(args);
        sendCommand("\n");

}

String NeosensoryBluefruit::getJson()
{
    return jsonMessage_;
}
/* Callbacks */

void NeosensoryBluefruit::scanCallback(ble_gap_evt_adv_report_t* report)
{
	if (checkDevice(report)) {
		Bluefruit.Central.connect(report);
	} else {
		Bluefruit.Scanner.resume();
	}
}

void NeosensoryBluefruit::connectCallback(uint16_t conn_handle)
{
if ( !conn->bonded() )
  {
    conn->requestPairing();
		  
  }
	bool success = true;
	if (!wb_service_.discover(conn_handle) ||
		!wb_write_characteristic_.discover() ||
		!wb_read_characteristic_.discover() ||
		!wb_read_characteristic_.enableNotify()||
		//!Bluefruit.requestPairing(conn_handle) ||
		//!Bluefruit.connPaired(conn_handle)
		!conn->bonded()
	)
		Bluefruit.disconnect(conn_handle);
		success = false;
	}

	if (externalConnectedCallback) {
		externalConnectedCallback(success);
	}
}

void NeosensoryBluefruit::disconnectCallback(
	uint16_t conn_handle, uint8_t reason) {
	is_authorized_ = false;
	externalDisconnectedCallback(conn_handle, reason);
}

void NeosensoryBluefruit::readNotifyCallback(
	BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
	parseCliData(data, len);
	externalReadNotifyCallback(chr, data, len);
    if(jsonMessage_.indexOf("button")!=-1)
    {

        if(externalButtonPressCallback)
        {
            String buttonVal = "button_val";
            int position = jsonMessage_.indexOf(buttonVal);

            char b = jsonMessage_[position +buttonVal.length() + 3 ];
             int buttonID = b-'0';
            externalButtonPressCallback(buttonID);
        }
        jsonMessage_ = "";
}

void NeosensoryBluefruit::setConnectedCallback(
	ConnectedCallback connectedCallback) {
	externalConnectedCallback = connectedCallback;
}

void NeosensoryBluefruit::setDisconnectedCallback(
	DisconnectedCallback disconnectedCallback) {
	externalDisconnectedCallback = disconnectedCallback;
}

void NeosensoryBluefruit::setReadNotifyCallback(
	ReadNotifyCallback readNotifyCallback) {
	externalReadNotifyCallback = readNotifyCallback;
}
void NeosensoryBluefruit::setButtonPressCallback(ButtonPressCallback buttonPressCallback)
{
    externalButtonPressCallback = buttonPressCallback;
}
/* Callback Wrappers */
NeosensoryBluefruit* NeosensoryBluefruit::NeoBluefruit = 0;

void scanCallbackWrapper(ble_gap_evt_adv_report_t* report) {
	NeosensoryBluefruit::NeoBluefruit->scanCallback(report);
}

void readNotifyCallbackWrapper(
	BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
	NeosensoryBluefruit::NeoBluefruit->readNotifyCallback(chr, data, len);
}

void connectCallbackWrapper(uint16_t conn_handle) {
	NeosensoryBluefruit::NeoBluefruit->connectCallback(conn_handle);
}

void disconnectCallbackWrapper(uint16_t conn_handle, uint8_t reason) {
	NeosensoryBluefruit::NeoBluefruit->disconnectCallback(conn_handle, reason);
}
