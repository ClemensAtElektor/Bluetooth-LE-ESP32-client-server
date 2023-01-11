#include <Arduino.h>
#include "BLEDevice.h"
#include "Wire.h"
#include "U8g2lib.h"


#define USER_SERIAL_DBG

/* serviceID

Every Service has its own BLE ID. More than one device can have the same serviceID 
e.g. this is like the sign on one of these franchise restaurants
and we are looking for a defined franchise here. 

*/

static BLEUUID serviceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");

/* tempUUID and hum UUID 
Those are the propertys, or "menu items" we are looking for.
Hope they are not out of stock....
*/

static BLEUUID    tempUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");
static BLEUUID    humUUID("ca73b3ba-39f6-4ab3-91ae-186dc9577d99");

/* We have some global variables here:*/
/* Kids don't do this at home, global variabels are bad!*/

static boolean doConnect = false; //Shows that we can connecton to our BLE Server
static boolean connected = false; //Shows that we are connected
static boolean doScan = false; //Shows that we have to search for BLE Server

//This 
static BLEAdvertisedDevice* myDevice; //This holds the address of the server we want to connect to

static BLERemoteCharacteristic* pRemoteTempCharacteristic; //This is the remote characteristic we are interrested in
static BLERemoteCharacteristic* pRemoteHumCharacteristic; //This is the remote characteristic we are interrested in

char ReceivedTemp[8]; //Remote characteristic is a string so we will store it here
char ReceivedHum[8]; //Remote characteristic is a string so we will store it here


/* This is for the SSD1306 based I²C OLED */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);




/* This will be called if a scan is completed and here also means we havent fount a sutable BLE server */
static void scanCompleteCB(BLEScanResults scanResults) {
  #ifdef USER_SERIAL_DBG
	  printf("Scan complete\n");
  #endif
	doScan=true; //We need to scan againMyClientCallback
} 

/* Our BLE characteristic will send notify for new data, this is the callback for it*/
static void NewTempNotify(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    /* our both characteristics just supply stings.... */
    if (true ==pBLERemoteCharacteristic->getUUID().equals(tempUUID)) {
      //We have a new temperatur
      bzero(&ReceivedHum,sizeof(ReceivedTemp));
      if (length<sizeof(ReceivedTemp))
      {
        strncpy(&ReceivedTemp[0],(const char *)pData,length);
        for (int i=0; i<length; i++)
        {
          Serial.print(pData[i],HEX);
          Serial.print(',');
        }
        Serial.println();
        #ifdef USER_SERIAL_DBG
          Serial.print("New Temp Data:");
          Serial.println(ReceivedTemp);
        #endif
      }
    } else {
       //?!
    }
    
}

/* Our BLE characteristic will send notify for new data, this is the callback for it*/
static void NewHumNotify(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
      if( true == pBLERemoteCharacteristic->getUUID().equals(humUUID)){
        //We have a new humidity
        bzero(&ReceivedHum,sizeof(ReceivedHum));
        if(length<sizeof(ReceivedHum)){
          strncpy(&ReceivedHum[0],(const char *)pData,length);
        for (int i=0; i<length; i++)
        {
          Serial.print(pData[i],HEX);
          Serial.print(',');
        }
          #ifdef USER_SERIAL_DBG
            Serial.print("New Hum Data:");
            Serial.println(ReceivedHum);
          #endif
        }
      } else {
        //?!
      }
}

/* Class for BLE Stack callbacks
here onConnect and onDisconnect are implimented 
*/
class BLEClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    #ifdef USER_SERIAL_DBG
      Serial.println("BLE Server Connected");
    #endif
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    #ifdef USER_SERIAL_DBG
      Serial.println("BLE Server Disconnect");
    #endif
  }
};

bool connectToServer() {
    #ifdef USER_SERIAL_DBG
      Serial.print("Forming a connection to ");
      Serial.println(myDevice->getAddress().toString().c_str());
    #endif
    BLEClient*  pClient  = BLEDevice::createClient();
    #ifdef USER_SERIAL_DBGNewHumNotify
      Serial.println(" - Created client");
    #endif

    pClient->setClientCallbacks(new BLEClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    #ifdef USER_SERIAL_DBG
      Serial.println(" - Connected to server");
    #endif

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      #ifdef USER_SERIAL_DBG
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
      #endif
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteTempCharacteristic = pRemoteService->getCharacteristic(tempUUID);
    if (pRemoteTempCharacteristic == nullptr) {
      #ifdef USER_SERIAL_DBG
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(tempUUID.toString().c_str());
      #endif
      pClient->disconnect();
      return false;
    } else {
      #ifdef USER_SERIAL_DBG
        Serial.print("Found our characteristic our characteristic UUID: ");
        Serial.println(tempUUID.toString().c_str());
      #endif
      if(true==pRemoteTempCharacteristic->canNotify()){
        pRemoteTempCharacteristic->registerForNotify(NewTempNotify);
      }
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteHumCharacteristic = pRemoteService->getCharacteristic(humUUID);
    if (pRemoteHumCharacteristic == nullptr) {
      #ifdef USER_SERIAL_DBG
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(tempUUID.toString().c_str());
      #endif
      pClient->disconnect();
      return false;
    } else {
      #ifdef USER_SERIAL_DBG
        Serial.print("Found our characteristic our characteristic UUID: ");
        Serial.println(humUUID.toString().c_str());
      #endif
      if(true==pRemoteHumCharacteristic->canNotify()){
        pRemoteHumCharacteristic->registerForNotify(NewHumNotify);
      }
     
    
    } 
    

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class Configured_AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    #ifdef USER_SERIAL_DBG
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
    #endif

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void oled_setup(){
  Wire.begin(21,22,100000);
  oled.setI2CAddress(0x78);
  oled.begin();
  oled.clear();
  oled.sendBuffer();
}

void oled_draw(uint8_t scr){
  oled.clear();
  switch(scr){
  case 0:{
    oled.setFont(u8g2_font_open_iconic_all_8x_t);
    oled.drawGlyph(0,64,247);
    oled.setFont(u8g2_font_inb16_mf); 
    oled.drawStr(58,16,"Setup");
    oled.drawStr(70,48,"BLE");
    oled.sendBuffer();
  }break;

  case 1:{
    oled.setFont(u8g2_font_open_iconic_all_8x_t);
    oled.drawGlyph(0,64,247);
    oled.setFont(u8g2_font_inb16_mf); 
    oled.drawStr(58,16,"Con");
    oled.drawStr(70,48,"BLE");
    oled.sendBuffer();
  }break;

   case 2:{
    oled.setFont(u8g2_font_inb16_mf); 
    uint8_t cursor = oled.drawStr(4,24,ReceivedTemp);
    oled.drawUTF8(cursor+8,24,"°C");
    cursor=oled.drawStr(4,54,ReceivedHum);
    oled.drawStr(cursor+8,54,"%");
    oled.sendBuffer();
  }break;

  default:{
    oled.sendBuffer();
  } break;
  }



}

void setup_BLE(){
  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new Configured_AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, scanCompleteCB, false);
}

void setup() {
  Serial.begin(115200);
  oled_setup();
  oled_draw(0);
  #ifdef USER_SERIAL_DBG
    Serial.println("Starting Arduino BLE Client application...");
  #endif
  setup_BLE();
  
} // End of setup.


// This is the Arduino main loop function.
void loop() {
static uint32_t last_oled_update=0;
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    oled_draw(1);
    if (connectToServer()) {
      #ifdef USER_SERIAL_DBG
        Serial.println("We are now connected to the BLE Server.");
      #endif
    } else {
      #ifdef USER_SERIAL_DBG
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      #endif
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    
    if(millis()>last_oled_update+10000){
      last_oled_update = millis();
      //Update OLED every 10 seconds....
      oled_draw(2);
    }
    
  }else if(doScan){
    doScan=false;
    oled_draw(0);
    #ifdef USER_SERIAL_DBG
      Serial.print(millis());
      Serial.println(": Start BLE Scan");
    #endif
    BLEDevice::getScan()->stop();
    BLEDevice::getScan()->start(5,scanCompleteCB,false);  

    
  }
  
  delay(1000); // Delay a second between loops.
  #ifdef USER_SERIAL_DBG
    Serial.print(".");
  #endif
} // End of loop
