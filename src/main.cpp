#include <Arduino.h>
/* Acquisition data library */
#include <AcquisitionData.h>
/* BLE sender data library */
#include <BLE.h>
/* CAN libraries */
#include <CanFunctions.h>
/* State Machine and Bit analyze librarie */
#include <CircularBufferState.h>
/* Ticker interrupts librarie */
#include <tickerISR.h>
/* WatchDog timer libraries */
#include <wdt.h>
/*ESPNOW libraries*/
#include<ESPNOW.h>

#include <SDcard.h>
#include <Definitions/Globals.h>

#include <Sim800L.h>

BLE_packet_t packet;
TaskHandle_t CANtask = NULL, Modulestask = NULL, BLEtask = NULL, SDcardtask = NULL;

/* State Of Telemetry (SOT) variables */
uint8_t _sot = DISCONNECTED;

bool saveFlag = false; 

SemaphoreHandle_t spiMutex; // Semáforo para controlar o SPI
/* Taks */
void CANprocess_Task(void *arg);
void ModulesProcess_Task(void *arg);
void BLEsenderData(void *arg);
void TaskESPNow(void *pvParameters);
void SDcard_Task(void *arg);
void GPRS_mqtt_Task(void *pvParameters);

void setup()
{
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  Serial.println("\r\nINICIANDO ALIVE 3.0\r\n");
  //spiMutex = xSemaphoreCreateMutex(); // Cria o semáforo

  ESPNOW_Setup();

  memset(&packet, 0, sizeof(BLE_packet_t));
  packet.DTC = "null";

  /* Start the MCP2515 to CAN communication */

  start_CAN_device();
  
  /* Set the new WDT timer */
  //set_wdt_timer();

  /* Init the BLE host connection */
  Init_BLE_Server();

  /* Init the Modules */
  start_module_device();

  /* Create the task responsible to the Acquisition(CAN + Accelerometer + GPS) */
  xTaskCreatePinnedToCore(CANprocess_Task, "CANstatemachine", 10000, NULL, 4, &CANtask, 1);
  //xTaskCreatePinnedToCore(ModulesProcess_Task, "Modulesstatemachine", 2048, NULL, 3, &Modulestask, 1);

  /* Create the task responsible to the Connectivity(BLE) management */
  //xTaskCreatePinnedToCore(BLEsenderData, "BLEstatemachine", 4096, NULL, 1, &BLEtask, 0);

  /* Create the task responsible to the Connectivity(ESPNOW) management */
  //xTaskCreatePinnedToCore(TaskESPNow, "ESPNowTask", 4096, NULL, 1, NULL, 0);

  /* Create the task responsible to the Connectivity(ESPNOW) management */
  //xTaskCreatePinnedToCore(SDcard_Task, "SDcardTask", 8192, NULL, 4, &SDcardtask, 1);

  xTaskCreatePinnedToCore(GPRS_mqtt_Task, "GPRSmqttTask", 4096, NULL, 5, NULL, 1);

  
}

void loop() { reset_rtc_wdt(); }

/* Core 1: Acquisition Threads */
void CANprocess_Task(void *arg)
{
  static int circularbuffer_State = IDLE_ST;
  bool dataWrite = true;
  TestIF_StdExt();
  checkPID();
  init_tickers();
  setup_SD_ticker();
  bool status_sd = sdConfig();

  while (1)
  {
    //if (xSemaphoreTake(spiMutex, portMAX_DELAY)){
    
    circularbuffer_State = CircularBuffer_state();

      if (circularbuffer_State != IDLE_ST)
        send_OBDmsg(circularbuffer_State, &packet);


      if(saveFlag && status_sd){        
        
        if(dataWrite){
            sdSave(true, packet);  
         }
         
        Check_SD_for_storage(packet);
        saveFlag = false;
        dataWrite = false;
      }
    //xSemaphoreGive(spiMutex);

    //}
      
        packet.gps_data.LAT = -8.055810;
        packet.gps_data.LNG = -34.951691;
    vTaskDelay(1);
  }
}

void ModulesProcess_Task(void *arg)
{
  static uint8_t gps_counter_per_seconds = 0;     // Each second will be incremented
  const static uint8_t Time_to_get_gps_data = 30; // Expected time to get/update the gps data (in seconds)

  while (1)
  {
    gps_counter_per_seconds++;

    if (gps_counter_per_seconds == Time_to_get_gps_data)
    {
      gps_acq_function(&packet);
      gps_counter_per_seconds = 0;
    }

    imu_acq_function(&packet);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/* Core 0: Telemetry Threads */
void BLEsenderData(void *arg)
{
  for (;;)
  {
    if (BLE_connected())
      Send_BLE_msg(packet);

    vTaskDelay(MAX_BLE_DELAY + 10);
  }
}

/*Task to receive external data from WIFI ESP-NOW*/
void TaskESPNow(void *pvParameters) {

  for (;;) {
    //Serial.println("TASK ESPNOW");
    //Serial.printf("ESPNOW received?: %d",newDataReceived);
    ESPNOW_printdata();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Pequeno delay para não ocupar toda a CPU
  }
}

/*Task to log data on SDcard*/
/*
void SDcard_Task(void *arg){

  uint8_t _sd = FAIL_RESPONSE;       // flag to check if SD module compile
if (xSemaphoreTake(spiMutex, portMAX_DELAY)) {
  _sd = start_SD_device(packet);
  xSemaphoreGive(spiMutex);
}

  for (;;) {
    if (xSemaphoreTake(spiMutex, portMAX_DELAY)) {
     Check_SD_for_storage(packet);
     xSemaphoreGive(spiMutex);
    }
    vTaskDelay(MAX_BLE_DELAY + 10);
  }

}*/


/* Connectivity Task */
void GPRS_mqtt_Task(void *pvParameters)
{
  _sot = Initialize_GSM();
  while (1)
  {   
    if (!Check_mqtt_client_conection())
    {    
      gsmReconnect(_sot);      
    }    //Serial.printf("mqtt_client_connection_geral --> %d\r\n", bluetooth_packet.mqtt_client_connection);

    Send_msg_MQTT(packet);

    vTaskDelay(500);
  }

  vTaskDelay(1);
}
