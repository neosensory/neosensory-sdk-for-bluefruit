/*
	NeosensoryBluefruit.cpp - Library for connecting to 
	Neosensory hardware via Adafruit's Bluefruit library.
	Created by Mike V. Perrotta, January 23, 2020.
*/

#include "Arduino.h"
#include "neosensory_bluefruit.h"
#include <bluefruit.h>
#include <stdlib.h>
#include <Base64.h>

/** @brief Constructor for new NeosensoryBluefruit object
 *	@param[in] device_id The device_id of the hardware to connect to
 */
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
	setDeviceAddress(device_id);
	num_motors_ = num_motors;
	max_vibration = initial_max_vibration;
	min_vibration = initial_min_vibration;

	// TODO: get this from firmware rather than hardcoding
	firmware_frame_duration_ = 16;
	uint8_t mtu = 247;
	max_frames_per_bt_package_ = (uint8_t)((mtu - 17) / (num_motors_ * (4 / 3.0f)));

	previous_motor_array_ = (uint8_t*)malloc(sizeof(uint8_t) * num_motors_);
	memset(previous_motor_array_, 0, sizeof(uint8_t) * num_motors_);
	is_authenticated_ = false;
}

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

/** @brief Sets new device ID for central to search for
 *	@param[in] new_device_id New device id to search for
 *	@note Does not restart scan, just sets device id for
 *	use in next scan.
 */
void NeosensoryBluefruit::setDeviceId(char new_device_id[]) {
	setDeviceAddress(new_device_id);
}

/** @brief Get address of device to connect to
 *	@return Byte array of address to connect to
 */
uint8_t* NeosensoryBluefruit::getDeviceAddress(void)
{
	return device_address_;
}

/** @brief Start scanning for desired device
 *	@return True if able to start scan, else False
 *	@note Will automatically connect to device if it is found in scan
 */
bool NeosensoryBluefruit::startScan(void)
{
	Bluefruit.Scanner.start(0);
}

bool NeosensoryBluefruit::isConnected(void) {
	return Bluefruit.Central.connected();
}

bool NeosensoryBluefruit::isAuthenticated(void) {
	return is_authenticated_;
}

/** @brief Send a command to the wristband
 *	@param[in] cmd Command to send
 */
void NeosensoryBluefruit::sendCommand(char cmd[]) {
	wb_write_characteristic_.write(cmd, strlen(cmd));
}

/** @brief Send a command to the wristband to authenticate developer options
 */
void NeosensoryBluefruit::authenticateWristband(void) {
	sendCommand("auth as developer\naccept\n");

	// TODO: Wait for successful authentication response
	// rather than assuming success
	delay(500);
	is_authenticated_ = true;
}

/** @brief Send a command to the wristband to turn off algorithm
 *	@note Also sends motors start command since algo stop command
 *	stops motors.
 */
void NeosensoryBluefruit::stopAlgorithm(void) {
	sendCommand("audio stop\nmotors start\n");
}

/** @brief Get number of motors
 */
uint8_t NeosensoryBluefruit::num_motors(void) {
	return num_motors_;
}

/** @brief Get firmware frame duration in milliseconds
 */
uint8_t NeosensoryBluefruit::firmware_frame_duration(void) {
	return firmware_frame_duration_;
}

/** @brief Get firmware frame duration in milliseconds
 */
uint8_t NeosensoryBluefruit::max_frames_per_bt_package(void) {
	return max_frames_per_bt_package_;
}

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

/** @brief Cause the wristband to vibrate at the given intensities
 *	@param[in] intensities An array of float values that denote the linear
 *	intensity values, between 0 and 1. Each index in this array corresponds
 *	to a motor. The value at that index corresponds to the intensity that motor
 *	will play at. A value of 0 is off, a value of 1 is max_vibration, and any
 *	value between is a linearly perceived value between min_vibration and
 *	max_vibration.
 *	@note This will not send a new command if the last sent array is identical
 *	to the new array of intensities.
 */
void NeosensoryBluefruit::vibrateMotors(float intensities[]) {
	uint8_t motor_intensities[num_motors_];
	getMotorIntensitiesFromLinArray(intensities, motor_intensities, num_motors_);

	if (compareArrays(motor_intensities, previous_motor_array_, num_motors_)) {
		return;
	}
	memcpy(previous_motor_array_, motor_intensities, sizeof(uint8_t) * num_motors_);

	sendMotorCommand(motor_intensities);
}

/** @brief Cause the wristband to vibrate at the given intensities, for multiple frames
 *	@param[in] intensities A nested array of float values that denote the linear
 *	intensity values, between 0 and 1. Each index in the inner arrays corresponds
 *	to a motor. The value at that index corresponds to the intensity that motor
 *	will play at. A value of 0 is off, a value of 1 is max_vibration, and any
 *	value between is a linearly perceived value between min_vibration and
 *	max_vibration. The outer indices correspond to individual frames. Each frame
 *	is played by the firmware at firmware_frame_duration intervals.
 *	@param[in] num_frames The number of frames. Cannot be more than max_frames_per_bt_package_.
 *	@note This will send all frames, even if any or all are identical to each other.
 */
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

/** @brief Turn off all the motors
 */
void NeosensoryBluefruit::turnOffAllMotors(void) {
	float motor_intensities[num_motors_];
	memset(motor_intensities, 0, sizeof(float) * num_motors_);
	vibrateMotors(motor_intensities);
}

/** @brief Turn on a single motor at an intensity
 *	@param[in] motor Index of motor to vibrate
 *	@param[in] float Intensity to vibrate motor at, between 0 and 1
 */
void NeosensoryBluefruit::vibrateMotor(uint8_t motor, float intensity) {
	float motor_intensities[num_motors_];
	memset(motor_intensities, 0, sizeof(float) * num_motors_);
	motor_intensities[motor] = intensity;
	vibrateMotors(motor_intensities);
}


/* Callbacks */

/** @brief Checks that a report address, found during a scan,
 * matches the address of the device we are searching for.
 * @param[in] foundAddress The address found during the scan.
 * @note Address will be reversed order from band name array.
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

/** @brief Callback when a device is found during scan
 *	@note This is set to automatically connect to a found
 *	device if its address matches our desired device address.
 *	Otherwise, the scanner resumes scanning.
 */
void NeosensoryBluefruit::scanCallback(ble_gap_evt_adv_report_t* report)
{
	if (checkAddressMatches(report->peer_addr.addr)) {
		Bluefruit.Central.connect(report);
	} else {
		Bluefruit.Scanner.resume();
	}
}

/** @brief Callback when central connects
 *	@param conn_handle Connection Handle that central connected to
 *	@note Checks that wristband services and characteristics are present,
 *	enables notification callbacks from read characteristic, 
 *	and pairs to connected wristband. If not, disconnects.
 *	Also calls externalConnectedCallback.
 */
void NeosensoryBluefruit::connectCallback(uint16_t conn_handle)
{
	bool success = true;
	if (!wb_service_.discover(conn_handle) ||
		!wb_write_characteristic_.discover() ||
		!wb_read_characteristic_.discover() ||
		!wb_read_characteristic_.enableNotify() ||
		!Bluefruit.requestPairing(conn_handle) ||
		!Bluefruit.connPaired(conn_handle))
	{
		Bluefruit.disconnect(conn_handle);
		success = false;
	}

	if (externalConnectedCallback) {
		externalConnectedCallback(success);
	}
}

/** @brief Callback when central disconnects
 */
void NeosensoryBluefruit::disconnectCallback(
	uint16_t conn_handle, uint8_t reason) {
	is_authenticated_ = false;
	externalDisconnectedCallback(conn_handle, reason);
}

/** @brief Callback when read characteristic has data to be read
 */
void NeosensoryBluefruit::readNotifyCallback(
	BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
	externalReadNotifyCallback(chr, data, len);
}

/** @brief Sets a callback that gets called when NeoBluefruit connects to a device
 *	@param[in] connectedCallback The function to call. Takes a bool argument, which
 *	will be true if connection resulted in successfully finding all services and
 *	characteristics, else false.
 */
void NeosensoryBluefruit::setConnectedCallback(
	ConnectedCallback connectedCallback) {
	externalConnectedCallback = connectedCallback;
}

/** @brief Sets a callback that gets called 
 *	when NeoBluefruit disconnects from a device
 *	@param[in] disconnectedCallback The function to call.
 */
void NeosensoryBluefruit::setDisconnectedCallback(
	DisconnectedCallback disconnectedCallback) {
	externalDisconnectedCallback = disconnectedCallback;
}

/** @brief Sets a callback that gets called when read characteristic has data
 *	@param[in] readNotifyCallback The function to call.
 */
void NeosensoryBluefruit::setReadNotifyCallback(
	ReadNotifyCallback readNotifyCallback) {
	externalReadNotifyCallback = readNotifyCallback;
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
