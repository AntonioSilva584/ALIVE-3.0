#include"SDcard.h"

// Define timeout time in milliseconds,0 (example: 2000ms = 2s)
const long timeoutTime = 3000;
bool mounted = false;
char file_name[20];
File dataFile;

/* Debug Variables */
boolean savingBlink = false;
boolean saveFlag = false;

uint8_t start_SD_device(BLE_packet_t msg_packet)
{
  do
  { Serial.println("Mount SD..."); } while (!sdConfig() && millis() < timeoutTime);

  if (!mounted)
  {
    Serial.println("SD mounted error!!");
    return FAIL_RESPONSE;
  }
    
  else{
    Serial.println("SD mounted!!");

  }
  sdSave(true, msg_packet);
  setup_SD_ticker();

  return SUCESS_RESPONSE;
}

bool sdConfig()
{
  if (!SD.begin(SD_CS))
    return false;

  File root;
  root = SD.open("/");
  int num_files = countFiles(root);
  sprintf(file_name, "/%s%d.csv", "OBDdata", num_files + 1);
  mounted = true;

  Serial.println("NOVO ARQUIVO!!");


  return true;
}

int countFiles(File dir)
{
  int fileCountOnSD = 0; // for counting files
  for (;;)
  {
    File entry = dir.openNextFile();

    // no more files
    if (!entry)
      break;

    // for each file count it
    fileCountOnSD++;
    entry.close();
  }

  return fileCountOnSD - 1;
}

uint8_t sdSave(bool set, BLE_packet_t msg_packet)
{
  uint8_t check_sd = FAIL_RESPONSE;

  dataFile = SD.open(file_name, FILE_APPEND);

  if (dataFile)
  {
    dataFile.println(packetToString(set,msg_packet));
    dataFile.close();
    savingBlink = !savingBlink;
    //digitalWrite(DEBUG_LED, savingBlink);
    check_sd = SUCESS_RESPONSE;
  }
  
  else
  {
    //digitalWrite(DEBUG_LED, LOW);
    Serial.println(F("falha no save"));
    check_sd = FAIL_RESPONSE;
  }
  
  //Serial.println("Into the function");
  //Serial.print("check_sd --> ");
  //Serial.println(check_sd);

  return check_sd;
}

String packetToString(bool err, BLE_packet_t msg_packet)
{  

  String dataString = "";
  if (err)
  {
    dataString += "Engine_Load";
    dataString += ",";
    dataString += "Engine_Coolant";
    dataString += ",";
    dataString += "Fuel_Pressure";
    dataString += ",";
    dataString += "MAP_SENSOR";
    dataString += ",";
    dataString += "Engine_RPM";
    dataString += ",";
    dataString += "Speed";
    dataString += ",";
    dataString += "Throttle_Position";
    dataString += ",";
    dataString += "Run_Time";
    dataString += ",";
    dataString += "Fuel_Level";
    dataString += ",";
    dataString += "Distance_traveled";
    dataString += ",";
    dataString += "Ambient_Temperature";
    dataString += ",";
    dataString += "Engine_Oil_Temperature";
    dataString += ",";
    dataString += "Engine_fuel_rate";
    dataString += ",";
    dataString += "Odometer";
    dataString += ",";
    dataString += "Acc_X";
    dataString += ",";
    dataString += "Acc_Y";
    dataString += ",";
    dataString += "Acc_Z";
    dataString += ",";

    /*
    dataString += "Ang_X";
    dataString += ",";
    dataString += "Ang_Y";
    dataString += ",";
    dataString += "Ang_Z";
    dataString += ",";    
    */
   dataString += "Latitude";
   dataString += ",";
   dataString += "Longitude";
   dataString += ",";
   /*
   dataString += "Temp_Intern";
   dataString += ",";
    */
   dataString += "DTC";


  }

  else
  {
    dataString += String((msg_packet.Calculated_Engine_Load));
    dataString += ",";
    dataString += String((msg_packet.Engine_Coolant_Temperature));
    dataString += ",";
    dataString += String((msg_packet.Fuel_Pressure));
    dataString += ",";
    dataString += String((msg_packet.Engine_RPM));
    dataString += ",";
    dataString += String((msg_packet.Speed));
    dataString += ",";
    dataString += String((msg_packet.Throttle_Position));
    dataString += ",";
    dataString += String((msg_packet.Run_Time));
    dataString += ",";
    dataString += String((msg_packet.Distance_traveled_with_MIL_on));
    dataString += ",";
    dataString += String((msg_packet.Fuel_Level_input));
    dataString += ",";
    dataString += String((msg_packet.Distance_traveled));
    dataString += ",";
    dataString += String((msg_packet.Ambient_Air_Temperature));
    dataString += ",";
    dataString += String((msg_packet.Engine_Oil_Temperature));
    dataString += ",";
    dataString += String((msg_packet.Engine_fuel_rate));
    dataString += ",";
    dataString += String((msg_packet.Odometer));
    dataString += ",";
    dataString += String((msg_packet.imu_acc.acc_x));
    dataString += ",";
    dataString += String((msg_packet.imu_acc.acc_y));
    dataString += ",";
    dataString += String((msg_packet.imu_acc.acc_z));

    /*
    dataString += ",";
    dataString += String((msg_packet.imu_acc.ang_x));
    dataString += ",";
    dataString += String((msg_packet.imu_acc.ang_x));
    dataString += ",";
    dataString += String((msg_packet.imu_acc.ang_x));
    */

    dataString += ",";
    dataString += String((msg_packet.gps_data.LAT));
    dataString += ",";    
    dataString += String((msg_packet.gps_data.LNG));
    /*
    dataString += ",";
    dataString += String((msg_packet.imu_acc.acctemp));
    */
    
    dataString += ",";
    dataString += String((msg_packet.DTC));
   
  }

  return dataString;
}

uint8_t Check_SD_for_storage(BLE_packet_t msg_packet)
{
  static uint8_t sd_status = FAIL_RESPONSE;

  if (saveFlag && mounted)
  {
    sd_status = sdSave(false, msg_packet);
    saveFlag = false;
  }

  return sd_status;
}

/* Ticker routine */
Ticker ticker40Hz;

void setup_SD_ticker()
{
  ticker40Hz.attach(1.0f, ticker40HzISR);
}

void ticker40HzISR()
{
  saveFlag = true; // set to true when the ECU store the CAN datas
}
