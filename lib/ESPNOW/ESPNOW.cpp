#include<ESPNOW.h>

ESPNOW_Struct espnow_msg;
bool newDataReceived = false;  // Flag para indicar novos dados recebidos

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&espnow_msg, incomingData, sizeof(espnow_msg));
  
    //Serial.print("Bytes received: ");
    //Serial.println(len);
    newDataReceived = true; 
    
}

/* Start the WIFI ESPNOW Setup */
void ESPNOW_Setup(){
// Desconecta de alguma conexão WiFi anterior e define o modo estação (STA)
 WiFi.disconnect();
 WiFi.mode(WIFI_STA);
 Serial.print("Endereço MAC: ");
 Serial.println(WiFi.macAddress()); // retorna o endereço MAC do dispositivo

 // Inicia a biblioteca ESP-NOW e, caso ocorra algum erro, reinicia o dispositivo
 if (esp_now_init() != ESP_OK) {    
   Serial.print("ESP-NOW com Erro");
   //ESP.restart();
 }
 // Registra a função OnDataRecv como a função a ser chamada quando receber dados via ESP-NOW
  esp_now_register_recv_cb(OnDataRecv);
}


void ESPNOW_printdata(){
    if(newDataReceived) {    
     
        Serial.print("Temperature: ");
        Serial.println(espnow_msg.temperature);
        Serial.print("Voltage: ");
        Serial.println(espnow_msg.voltage);
        newDataReceived = false;  // Reseta a flag
        // Processamento adicional dos dados recebidos, se necessário
        
      }
}