#ifndef ESPNOW_H
#define ESPNOW_H

#include <esp_now.h>            // Biblioteca para utilizar o protocolo de comunicação ESP-NOW
#include <WiFi.h>               // Biblioteca para conectar em redes Wi-Fi

typedef struct ESPNOW_Struct {  // Define a estrutura ESPNOW_Struct para troca de informações
    float temperature;   
    float voltage;   
    float current;
  } ESPNOW_Struct;
  

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

void ESPNOW_Setup();

void ESPNOW_printdata();
#endif