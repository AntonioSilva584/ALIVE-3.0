#ifndef SDCARD_H
#define SDCARD_H


#include <SD.h>
#include <Ticker.h>

#define SUCESS_RESPONSE 2
#define FAIL_RESPONSE   1

/* SD */ 
#define SD_CS         GPIO_NUM_5

/* SD definitions */
uint8_t start_SD_device(void);
bool sdConfig(void);
int countFiles(File dir);
uint8_t sdSave(bool set); 
String packetToString(bool err);
uint8_t Check_SD_for_storage(void);

/* Ticker definitions */
void setup_SD_ticker(void);

/* Ticker interrupts */
void ticker40HzISR(void);

#endif


