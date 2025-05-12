#ifndef SDCARD_H
#define SDCARD_H


#include <SD.h>
#include <Ticker.h>
#include "CanFunctions.h"

#define SUCESS_RESPONSE 2
#define FAIL_RESPONSE   1

/* SD */ 
#define SD_CS         GPIO_NUM_15

/* SD definitions */
uint8_t start_SD_device(BLE_packet_t msg_packet);
bool sdConfig(void);
int countFiles(File dir);
uint8_t sdSave(bool set, BLE_packet_t packet); 
String packetToString(bool err, BLE_packet_t packet);
uint8_t Check_SD_for_storage(BLE_packet_t packet);

/* Ticker definitions */
void setup_SD_ticker(void);

/* Ticker interrupts */
void ticker40HzISR(void);

#endif


