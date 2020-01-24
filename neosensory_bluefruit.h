/*
  Neosensory_Bluefruit.h - Library for connecting to 
  Neosensory hardware via Adafruit's Bluefruit library.
  Created by Mike Perrotta, January 23, 2020.
*/
#ifndef NeosensoryBluefruit_h
#define NeosensoryBluefruit_h

#include "Arduino.h"
#include <bluefruit.h>

class NeosensoryBluefruit
{
    typedef void (*ConnectedCallback)(bool); 
    typedef void (*DisconnectedCallback)(uint16_t, uint8_t); 
    typedef void (*ReadNotifyCallback)(BLEClientCharacteristic*, uint8_t*, uint16_t); 

  public:
    NeosensoryBluefruit(char device_id[], uint8_t numMotors=4, 
        uint8_t min_vibration=30, uint8_t max_vibration=255);
    static NeosensoryBluefruit* NeoBluefruit;
    void begin(void);
    bool startScan(void);
    void disconnect(void);
    void setDeviceId(char new_device_id[]);
    uint8_t* getDeviceAddress(void);
    bool isConnected(void);
    bool isAuthenticated(void);
    void sendCommand(char cmd[]);
    void stopAlgorithm(void);
    void authenticateWristband(void);
    void setConnectedCallback(ConnectedCallback);
    void setDisconnectedCallback(DisconnectedCallback);
    void setReadNotifyCallback(ReadNotifyCallback);

    /* BLE Callbacks */
    void scanCallback(ble_gap_evt_adv_report_t* report);
    void readNotifyCallback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
    void connectCallback(uint16_t conn_handle);
    void disconnectCallback(uint16_t conn_handle, uint8_t reason);

    /* Vibration */
    void vibrateAtIntensities(float intensities[]);
    void turnOffAllMotors(void);
    void vibrateMotor(uint8_t motor, float intensity);
    uint8_t min_vibration;
    uint8_t max_vibration;
    uint8_t num_motors();

  private:
    uint8_t device_address_[BLE_GAP_ADDR_LEN];
    void setDeviceAddress(char device_id[]);
    bool checkAddressMatches(uint8_t foundAddress[]);
    bool is_authenticated_;
    char read_message_[1024];

    /* Vibrations */
    uint8_t num_motors_;
    uint8_t *previous_motor_array_;
    void getMotorIntensitiesFromLinArray(
        float lin_array[], uint8_t motor_space_array[], size_t array_size);

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

void scanCallbackWrapper(ble_gap_evt_adv_report_t* report);
void readNotifyCallbackWrapper(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
void connectCallbackWrapper(uint16_t conn_handle);
void disconnectCallbackWrapper(uint16_t conn_handle, uint8_t reason);

#endif