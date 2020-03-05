/*!
 * @file neosensory_bluefruit.h
 *
 * @mainpage Neosensory Bluefruit BLE helper.
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Neosensory's NeosensoryBluefruit library,
 * which handles connecting to and communicating with a Neosensory device over BLE. 
 * It heavily relies on Adafruit's Bluefruit library for BLE. 
 * It also opens all developer accessible CLI commands with the Neosensory hardware. 
 * It also offers some higher level motor vibration functions.
 *
 * @section author Author
 *
 * Written by Mike Perrotta for Neosensory, Inc.
 *
 * @section license License
 *
 *
 */

#ifndef NeosensoryBluefruit_h
#define NeosensoryBluefruit_h

#include "Arduino.h"
#include <bluefruit.h>

/** @brief Class that handles connecting to and communicating with a Neosensory device over BLE. 
 *  Relies heavily on Adafruit's Bluefruit library for BLE. Opens all developer accessible
 *  CLI commands with Neosensory hardware. Also offers some higher level motor vibration functions.
 */
class NeosensoryBluefruit
{
    typedef void (*ConnectedCallback)(bool); 
    typedef void (*DisconnectedCallback)(uint16_t, uint8_t); 
    typedef void (*ReadNotifyCallback)(BLEClientCharacteristic*, uint8_t*, uint16_t); 

  public:
    /** @brief Constructor for new NeosensoryBluefruit object
     *  @param[in] device_id The device_id of the hardware to connect to. Leave blank to connect to any Neosensory device.
     *  @param[in] num_motors The number of vibrating motors this device has.
     *  @param[in] initial_min_vibration The mininum vibration intensity, between 0 and 255. Should be less than initial_max_vibration.
     *  @param[in] initial_max_vibration The maximum vibration intensity, between 0 and 255. Should be greater than initial_min_vibration.
     */
    NeosensoryBluefruit(char device_id[]="", uint8_t num_motors=4, 
        uint8_t initial_min_vibration=30, uint8_t initial_max_vibration=255);

    static NeosensoryBluefruit* NeoBluefruit; /**< Static, singleton instance of NeosensoryBluefruit. Used for setting callbacks. */

    /** @brief Returns true if NeosensoryBluefruit has connected to a device.
     *  @return True if NeosensoryBluefruit has connected to a device.
     */
    bool isConnected(void);

    /** @brief Start scanning for desired device
     *  @return True if able to start scan, else False
     *  @note Will automatically connect to device if it is found in scan
     */
    bool startScan(void);

    /** @brief Get address of device to connect to
     *  @return Byte array of address to connect to, or 0 if not set.
     */
    uint8_t* getDeviceAddress(void);
    
    /** @brief Begins Bluetooth components of NeosensoryBluefruit.
    */
    void begin(void);

    /** @brief Sets new device ID for central to search for
     *  @param[in] new_device_id New device id to search for.
     *  If is an empty array, NeosensoryBluefruit will connect to any Neosensory device. 
     *  @note Does not restart scan, just sets device id for
     *  use in next scan.
     */
    void setDeviceId(char new_device_id[]);


    /* Developer Commands */

    /** @brief Returns true if connected device has authorized developer options.
     *  @return True if the connected device has authorized developer options.
     */
    bool isAuthorized(void);

    /** @brief Send a command to the wristband to accept developer terms and conditions.
     */
    void acceptTermsAndConditions(void);

    /** @brief Stars the audio task processing. 
     *  @note This will start microphone audio acquisition and pipe the audio to the current audio sink. 
     */
    void audioStart(void);

    /** @brief Stops the current audio task processing and hence any motor outputs from the algorithm.
     *  @note This stops the actual audio acquisition from the microphone.
     */
    void audioStop(void);

    /** @brief Send a command to the wristband to authorize developer options.
     *  @note Authorization will only happen if this command is followed by 
     *  acceptTermsAndConditions().
     */
    void authorizeDeveloper(void);

    /** @brief Get the amount of charge left on the device battery in percentage.
     */
    void deviceBattery(void);

    /** @brief Get information about the connected Neosensory device.
     *  @note This can be called without authorizing developer options.
     */
    void deviceInfo(void);

    /** @brief Clears the motor command queue.
     */
    void motorsClearQueue(void);

    /** @brief Initialize and start the motors interface. 
     *  @note The device will now respond to motor vibrate commands.
     */
    void motorsStart(void);

    /** @brief Clears the motor queue and stops the motors interface.
     *  @note The device will no longer respond to motor vibrate commands.
     */
    void motorsStop(void);

    /** @brief Send a command to the wristband
     *  @param[in] cmd Command to send
     */
    void sendCommand(char cmd[]);

    /** @brief Stops the sound-to-touch algorithm that runs on the wristband.
     *  @note Stops audio and restarts the motors, which stop when audio is stopped.
     */
    void stopAlgorithm(void);


    /* BLE Callbacks */

    /** @brief Callback when central connects.
     *  @param conn_handle Connection Handle that central connected to
     *  @note Checks that wristband services and characteristics are present,
     *  enables notification callbacks from read characteristic, 
     *  and pairs to connected wristband. If not, disconnects.
     *  Also calls externalConnectedCallback.
     */
    void connectCallback(uint16_t conn_handle);

    /** @brief Callback when central disconnects.
     *  @param[in] conn_handle Connection handle that central is disconnecting from.
     *  @param[in] reason Reason for disconnect.
     */
    void disconnectCallback(uint16_t conn_handle, uint8_t reason);

    /** @brief Callback when read characteristic has data to be read.
     *  @param[in] chr Characteristic that read data.
     *  @param[in] data Data read.
     *  @param[in] len Length of data array.
     */
    void readNotifyCallback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);

    /** @brief Callback when a device is found during scan.
     *  @param[in] report Report of device that scan found.
     *  @note This is set to automatically connect to a found
     *  device if its address matches our desired device address.
     *  Otherwise, the scanner resumes scanning.
     */
    void scanCallback(ble_gap_evt_adv_report_t* report);

    /** @brief Sets a callback that gets called when NeoBluefruit connects to a device.
     *  @param[in] connectedCallback The function to call. Takes a bool argument, which
     *  will be true if connection resulted in successfully finding all services and
     *  characteristics, else false.
     */
    void setConnectedCallback(ConnectedCallback);

    /** @brief Sets a callback that gets called.
     *  when NeoBluefruit disconnects from a device
     *  @param[in] disconnectedCallback The function to call.
     */
    void setDisconnectedCallback(DisconnectedCallback);

    /** @brief Sets a callback that gets called when read characteristic has data
     *  @param[in] readNotifyCallback The function to call.
     */
    void setReadNotifyCallback(ReadNotifyCallback);


    /* Vibration */

    /** @brief Get firmware frame duration in milliseconds.
     *  @return Frame duration of the firmware in milliseconds.
     *  @note When multiple motor frames are sent to the wristband, they
     *  will each play for this duration (or longer, if no subsequent motor
     *  frame has been sent).
     */
    uint8_t firmware_frame_duration(void);

    /** @brief Get maximum number of frames allowed in a Bluetooth packet.
     *  @return Max frames allowed in a Bluetooth packet.
     */
    uint8_t max_frames_per_bt_package(void);

    uint8_t max_vibration; /**< Maximum vibration intensity, between 0 and 255. */

    uint8_t min_vibration; /**< Minimum vibration intensity, between 0 and 255. */

    /** @brief Get number of motors
     *  @return The number of motors this instance of NeosensoryBluetooth 
     *  expects in the target device.
     */
    uint8_t num_motors(void);

    /** @brief Turn off all the motors.
     */
    void turnOffAllMotors(void);

    /** @brief Turn on a single motor at an intensity
     *  @param[in] motor Index of motor to vibrate
     *  @param[in] intensity Intensity to vibrate motor at, between 0 and 1
     */
    void vibrateMotor(uint8_t motor, float intensity);

    /** @brief Cause the wristband to vibrate at the given intensities, for multiple frames
     *  @param[in] intensities A nested array of float values that denote the linear
     *  intensity values, between 0 and 1. Each index in the inner arrays corresponds
     *  to a motor. The value at that index corresponds to the intensity that motor
     *  will play at. A value of 0 is off, a value of 1 is max_vibration, and any
     *  value between is a linearly perceived value between min_vibration and
     *  max_vibration. The outer indices correspond to individual frames. Each frame
     *  is played by the firmware at firmware_frame_duration intervals.
     *  @param[in] num_frames The number of frames. Cannot be more than max_frames_per_bt_package_.
     *  @note This will send all frames, even if any or all are identical to each other.
     */
    void vibrateMotors(float *intensities[], int num_frames);

    /** @brief Cause the wristband to vibrate at the given intensities
     *  @param[in] intensities An array of float values that denote the linear
     *  intensity values, between 0 and 1. Each index in this array corresponds
     *  to a motor. The value at that index corresponds to the intensity that motor
     *  will play at. A value of 0 is off, a value of 1 is max_vibration, and any
     *  value between is a linearly perceived value between min_vibration and
     *  max_vibration.
     *  @note This will not send a new command if the last sent array is identical
     *  to the new array of intensities.
     */
    void vibrateMotors(float intensities[]);

  private:
    bool checkAddressMatches(uint8_t foundAddress[]);
    bool checkDevice(ble_gap_evt_adv_report_t* report);
    bool checkIsNeosensory(ble_gap_evt_adv_report_t* report);
    bool connect_to_any_neo_device_;
    bool is_authorized_;
    uint8_t device_address_[BLE_GAP_ADDR_LEN];
    void setDeviceAddress(char device_id[]);

    /* Vibrations */
    uint8_t *previous_motor_array_;
    uint8_t firmware_frame_duration_;
    uint8_t max_frames_per_bt_package_;
    uint8_t num_motors_;
    void getMotorIntensitiesFromLinArray(
        float lin_array[], uint8_t motor_space_array[], size_t array_size);
    void sendMotorCommand(uint8_t motor_intensities[], size_t num_frames=1);

    /* CLI Parsing */
    bool jsonStarted_;
    String jsonMessage_;
    void handleCliJson(String jsonMessage);
    void parseCliData(uint8_t* data, uint16_t len);

    /* External Callbacks */
    ConnectedCallback externalConnectedCallback;
    DisconnectedCallback externalDisconnectedCallback;
    ReadNotifyCallback externalReadNotifyCallback;

    /* Services & Characteristic UUIDs */
    uint8_t wb_service_uuid_[16];
    uint8_t wb_write_char_uuid_[16];
    uint8_t wb_read_char_uuid_[16];

    /* Services & Characteristic */
    BLEClientService wb_service_;
    BLEClientCharacteristic wb_write_characteristic_;
    BLEClientCharacteristic wb_read_characteristic_;
};

void connectCallbackWrapper(uint16_t conn_handle);
void disconnectCallbackWrapper(uint16_t conn_handle, uint8_t reason);
void readNotifyCallbackWrapper(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
void scanCallbackWrapper(ble_gap_evt_adv_report_t* report);

#endif