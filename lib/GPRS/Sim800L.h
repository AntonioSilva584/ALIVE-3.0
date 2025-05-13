#ifndef CON_STATE_MACHINE_H
#define CON_STATE_MACHINE_H

#include <Arduino.h>
//#include <WiFi.h>
#include <ESPmDNS.h>
//#include <ArduinoJson.h>
#include <Ticker.h>
//#include "CAN.h"
#include "gprs_defs.h"
//#include "hardware_defs.h"
#include "CanFunctions.h" 
#include "CircularBufferState.h"
#include "driver/gpio.h"

#include <ArduinoJson.h>

/* GPRS */
#define MODEM_RST
#define MODEM_TX      GPIO_NUM_17
#define MODEM_RX      GPIO_NUM_16

/* Credentials Variables */
//#define TIM     // Uncomment this line and comment the others if this is your chip
#define CLARO   // Uncomment this line and comment the others if this is your chip
//#define VIVO    // Uncomment this line and comment the others if this is your chip

/* State Machines */
typedef enum {
    DISCONNECTED    = (0 << 1), 
    CONNECTED       = (1 << 0), 
    RESERVED_ID_4x4 = (1 << 1), // this ID is set in front ECU
    ERROR_CONECTION = (1 << 2)
} connectivity_states;

uint8_t Initialize_GSM(void);
void gsmCallback(char *topic, byte *payload, unsigned int length);
boolean Check_mqtt_client_conection(void);
void gsmReconnect(uint8_t &_try_reconect);
void Send_msg_MQTT(BLE_packet_t msg_packet);
void publishPacket(BLE_packet_t msg_packet);

/* Ticker functions */
void setup_GSM_tic(void);

/* Interrupts routine */
void ticker1HzISR(void);
void ticker20HzISR(void);

#endif