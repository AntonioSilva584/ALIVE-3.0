#include "Sim800L.h"

/* GPRS credentials */
#ifdef TIM
  const char *apn = "timbrasil.br";    // Your APN
  const char *gprsUser = "tim";        // User
  const char *gprsPass = "tim";        // Password
  const char *simPIN = "1010";         // SIM card PIN code, if any
#elif defined(CLARO)
  const char *apn = "claro.com.br";    // Your APN
  const char *gprsUser = "claro";      // User
  const char *gprsPass = "claro";      // Password
  const char *simPIN = "3636";         // SIM cad PIN code, id any
#elif defined(VIVO)
  const char *apn = "zap.vivo.com.br";  // Your APN
  const char *gprsUser = "vivo";        // User
  const char *gprsPass = "vivo";        // Password
  const char *simPIN = "8486";          // SIM cad PIN code, id any
#else
  const char *apn = "timbrasil.br";    // Your APN
  const char *gprsUser = "tim";        // User
  const char *gprsPass = "tim";        // Password
  const char *simPIN = "1010";         // SIM card PIN code, if any
#endif

//unsigned long timer;

// Flags to ticker function 
bool sendFlag = false;
bool buff = false;

/* Variables to storage the data in array and send the 20 packets of 64 bytes */
uint8_t volatile_bytes[MSG_BUFFER_SIZE];
int volatile_position = 0;

const char *node_server = "157.245.220.92";
char payload_char[MSG_BUFFER_SIZE];
char msg[MSG_BUFFER_SIZE];
/*
// ESP hotspot definitions
const char *host = "esp32";                   // Here's your "host device name"
const char *ESP_ssid = "ALIVE_hotspot";     // Here's your ESP32 WIFI ssid
const char *ESP_password = "Alive"; // Here's your ESP32 WIFI pass
*/

/* GSM definitions */
#include <TinyGSM.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqttClient(client);

uint8_t Initialize_GSM()
{
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  // Or, use modem.init() if you don't need the complete restart

  Serial.print("Modem: "); Serial.println(modem.getModemInfo());

  Serial.print("Status: "); Serial.println(modem.getSimStatus());

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3)
  {
    modem.simUnlock(simPIN);
  }

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork(15000L))
  {
    Serial.println("fail");
    return (uint8_t)ERROR_CONECTION;
  }
  Serial.println("OK");

  if (modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }

  Serial.print(F("Connecting to APN: "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    Serial.println(" fail");
    return (uint8_t)ERROR_CONECTION;
  }
  Serial.println(" OK");
  /*
  // Wi-Fi Config and Debug
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ESP_ssid, ESP_password);
  WiFi.begin(ESP_ssid, ESP_password);
  */

  // if(!MDNS.begin(host)) // Use MDNS to solve DNS
  // {
  //   // http://esp32.local
  //   Serial.println("Error configuring mDNS. Rebooting in 1s...");
  //   return (uint8_t)ERROR_CONECTION;
  // }
  // Serial.println("mDNS configured;");

  mqttClient.setServer(node_server, PORT);
  // mqttClient.setCallback(gsmCallback);
  mqttClient.setBufferSize(MAX_GPRS_BUFFER - 1);


  Serial.println("Ready");
  
  //Serial.print("SoftAP IP address: "); Serial.println(WiFi.softAPIP());

  uint8_t i[2] = {204, 255};
  mqttClient.publish("/logging", i, sizeof(i));
  setup_GSM_tic();

  return (uint8_t)CONNECTED;
}

void gsmCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  memset(payload_char, 0, sizeof(payload_char));

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    payload_char[i] = (char)payload[i];
  }
  Serial.println();
}

boolean Check_mqtt_client_conection()
{
  return mqttClient.connected();
}

void gsmReconnect(uint8_t &_try_reconect)
{
  int count = 0;
  Serial.println("Conecting to MQTT Broker...");
  while (!mqttClient.connected() && count < 3)
  {
    count++;
    Serial.println("Reconecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), "preconecta", "liveAPI", "/esp-connected", 2, true, "Offline", true))
    {
      sprintf(msg, "%s", "Online");
      mqttClient.publish("/esp-connected", msg);
      memset(msg, 0, sizeof(msg));
      Serial.println("Connected.");

      _try_reconect = CONNECTED; // enable online flag

      /* Subscribe to topics */
      mqttClient.subscribe("/esp-test");
      //digitalWrite(LED_BUILTIN, HIGH);

    } else {
      Serial.print("Failed with state");
      Serial.println(mqttClient.state());
      
      delay(2000); 
      _try_reconect = DISCONNECTED; // disable online flag 
    }
  }
}

void Send_msg_MQTT(BLE_packet_t msg_packet)
{
  //mqtt_packet_t recv = update_packet();
  publishPacket(msg_packet);
  mqttClient.loop();
}

void publishPacket(BLE_packet_t msg_packet)
{
  /*
    Send the message using JSON example:
      * 1 - StaticJsonDocument<305> doc;
      * 2 - doc["data"] = data;
      * 3 - memset(msg, 0, sizeof(msg));
      * 4 - serializeJson(doc, msg);
      * 5 - mqttClient.publish("/logging", msg)
  */

   StaticJsonDocument<400> doc;
     
       doc["Engine_Load"]            = verify_message_is_null(EngineLoad, msg_packet.Calculated_Engine_Load);
    doc["Engine_Coolant"]         = verify_message_is_null(EngineCollantTemp, msg_packet.Engine_Coolant_Temperature);
    doc["Fuel_Pressure"]          = verify_message_is_null(FuelPressure, msg_packet.Fuel_Pressure);
    doc["MAP_SENSOR"]             = verify_message_is_null(IntakeManifoldAbsolutePressure, msg_packet.Intake_Manifold__MAP);
    doc["Engine_RPM"]             = verify_message_is_null(EngineRPM, msg_packet.Engine_RPM);
    doc["Speed"]                  = verify_message_is_null(VehicleSpeed, msg_packet.Speed);
    doc["Throttle_Position"]      = verify_message_is_null(ThrottlePosition, msg_packet.Throttle_Position);
    doc["Run_Time"]               = verify_message_is_null(RunTimeSinceEngineStart, msg_packet.Run_Time);
    doc["Distance_traveled_MIL"]  = verify_message_is_null(DistanceTraveledMIL, msg_packet.Distance_traveled_with_MIL_on);
    doc["Fuel_Level"]             = verify_message_is_null(FuelLevelInput, msg_packet.Fuel_Level_input);
    doc["Distance_traveled"]      = verify_message_is_null(DistanceTraveledSinceCodeCleared, msg_packet.Distance_traveled);
    doc["Ambient_Temperature"]    = verify_message_is_null(AmbientAirTemperature, msg_packet.Ambient_Air_Temperature);
    doc["Engine_Oil_Temperature"] = verify_message_is_null(EngineOilTemperature, msg_packet.Engine_Oil_Temperature);
    doc["Engine_fuel_rate"]       = verify_message_is_null(EngineFuelRate, msg_packet.Engine_fuel_rate);
    //doc["Odometer"]               = verify_message_is_null(Odometer_PID, msg_packet.Odometer);
    //doc["Acc_X"]               = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_x);
    //doc["Acc_Y"]                  = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_y);
    //doc["Acc_Z"]                  = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_z);
    //doc["Ang_X"]            = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_x);
    //doc["Ang_Y"]               = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_y);
    //doc["Ang_Z"]              = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_z);
    //doc["Latitude"] = verify_message_is_null(GPS_ST, msg_packet.gps_data.LAT);
    //doc["Longitude"] = verify_message_is_null(GPS_ST, msg_packet.gps_data.LNG);
    //doc["Temp_Intern"]              = verify_message_is_null(Accelerometer_ST, msg_packet.acctemp);    
    //doc["DTC"]                    = msg_packet.DTC;

    // doc["x04"]  = verify_message_is_null(EngineLoad, msg_packet.Calculated_Engine_Load);
    // doc["x05"]  = verify_message_is_null(EngineCollantTemp, msg_packet.Engine_Coolant_Temperature);
    // doc["x0A"]  = verify_message_is_null(FuelPressure, msg_packet.Fuel_Pressure);
    // doc["x0B"]  = verify_message_is_null(IntakeManifoldAbsolutePressure, msg_packet.Intake_Manifold__MAP);
    // doc["x0C"]  = verify_message_is_null(EngineRPM, msg_packet.Engine_RPM);
    // doc["x0D"]  = verify_message_is_null(VehicleSpeed, msg_packet.Speed);
    // doc["x11"]  = verify_message_is_null(ThrottlePosition, msg_packet.Throttle_Position);
    // doc["x1F"]  = verify_message_is_null(RunTimeSinceEngineStart, msg_packet.Run_Time);
    // doc["x21"]  = verify_message_is_null(DistanceTraveledMIL, msg_packet.Distance_traveled_with_MIL_on);
    // doc["x2F"]  = verify_message_is_null(FuelLevelInput, msg_packet.Fuel_Level_input);
    // doc["x31"]  = verify_message_is_null(DistanceTraveledSinceCodeCleared, msg_packet.Distance_traveled);
    // doc["x46"]  = verify_message_is_null(AmbientAirTemperature, msg_packet.Ambient_Air_Temperature);
    // doc["x5C"]  = verify_message_is_null(EngineOilTemperature, msg_packet.Engine_Oil_Temperature);
    // doc["x5E"]  = verify_message_is_null(EngineFuelRate, msg_packet.Engine_fuel_rate);
    // doc["xA6"]  = verify_message_is_null(Odometer_PID, msg_packet.Odometer);
    // doc["AcX"] = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_x);
    // doc["AcY"] = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_y);
    // doc["AcZ"] = verify_message_is_null(Accelerometer_ST, msg_packet.imu_acc.acc_z);
    // //doc["AgX"]  = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_x);
    // //doc["AgY"]  = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_y);
    // //doc["AgZ"]  = verify_message_is_null(Accelerometer_ST, msg_packet.imu_ang.ang_z);
    // doc["Lat"] = verify_message_is_null(GPS_ST, msg_packet.gps_data.LAT);
    // doc["Lon"] = verify_message_is_null(GPS_ST, msg_packet.gps_data.LNG);
    // //doc["Temp_Intern"]              = verify_message_is_null(Accelerometer_ST, msg_packet.acctemp);    
    // doc["DTC"]                    = msg_packet.DTC;


  memset(msg, 0, sizeof(msg));
  serializeJson(doc, msg);
  mqttClient.publish("/logging", msg);
/*
  if (volatile_position + len > MSG_BUFFER_SIZE)
  {
    // Handle the case when the array is full, for example by resetting the current position to the beginning.
    volatile_position = 0;
  }

  if (buff)
  {
    memcpy(&volatile_bytes[volatile_position], (uint8_t *)T, len);

    volatile_position += len;
    buff = false;
  }

  if (sendFlag)
  {
    mqttClient.publish("/logging", volatile_bytes, MSG_BUFFER_SIZE);
    sendFlag = false;
  }
*/
}

/* Ticker functions */
Ticker ticker1Hz, ticker20Hz;

void setup_GSM_tic()
{
  ticker1Hz.attach(1.0f, ticker1HzISR);
  ticker20Hz.attach(0.05f, ticker20HzISR);
}

void ticker1HzISR()
{
  sendFlag = true;
}

void ticker20HzISR()
{
  buff = true;
}
