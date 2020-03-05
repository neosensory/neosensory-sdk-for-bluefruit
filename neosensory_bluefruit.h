/*
  Neosensory_Bluefruit.h - Library for connecting to 
  Neosensory hardware via Adafruit's Bluefruit library.
  Created by Mike Perrotta, January 23, 2020.
*/
#ifndef NeosensoryBluefruit_h
#define NeosensoryBluefruit_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <bluefruit.h>

class NeosensoryBluefruit
{
    typedef void (*ConnectedCallback)(bool); 
    typedef void (*DisconnectedCallback)(uint16_t, uint8_t); 
    typedef void (*ReadNotifyCallback)(BLEClientCharacteristic*, uint8_t*, uint16_t); 

  public:
    NeosensoryBluefruit(char device_id[]="", uint8_t numMotors=4, 
        uint8_t initial_min_vibration=30, uint8_t initial_max_vibration=255);
    static NeosensoryBluefruit* NeoBluefruit;

    bool isAuthenticated(void);
    bool isConnected(void);
    bool startScan(void);
    uint8_t* getDeviceAddress(void);
    void begin(void);
    void setDeviceId(char new_device_id[]);

    /* Developer Commands */
    void acceptTermsAndConditions(void);
    void audioStart(void);
    void audioStop(void);
    void authorizeDeveloper(void);
    void deviceBattery(void);
    void deviceInfo(void);
    void motorsClearQueue(void);
    void motorsStart(void);
    void motorsStop(void);
    void sendCommand(char cmd[]);
    void stopAlgorithm(void);

    /* BLE Callbacks */
    void connectCallback(uint16_t conn_handle);
    void disconnectCallback(uint16_t conn_handle, uint8_t reason);
    void readNotifyCallback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
    void scanCallback(ble_gap_evt_adv_report_t* report);
    void setConnectedCallback(ConnectedCallback);
    void setDisconnectedCallback(DisconnectedCallback);
    void setReadNotifyCallback(ReadNotifyCallback);

    /* Vibration */
    uint8_t firmware_frame_duration(void);
    uint8_t max_frames_per_bt_package(void);
    uint8_t max_vibration;
    uint8_t min_vibration;
    uint8_t num_motors(void);
    void turnOffAllMotors(void);
    void vibrateMotor(uint8_t motor, float intensity);
    void vibrateMotors(float *intensities[], int num_frames);
    void vibrateMotors(float intensities[]);

  private:
    bool checkAddressMatches(uint8_t foundAddress[]);
    bool checkDevice(ble_gap_evt_adv_report_t* report);
    bool checkIsNeosensory(ble_gap_evt_adv_report_t* report);
    bool connect_to_any_neo_device_;
    bool is_authenticated_;
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