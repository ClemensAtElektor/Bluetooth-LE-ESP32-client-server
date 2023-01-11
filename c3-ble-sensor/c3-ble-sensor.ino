#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* If we comment this define serial output will be disabled */
//#define Serial_Debug

#define DHTPIN 8 /* cpv, was 6 */    
#define DHTTYPE    DHT11     // DHT 11

/* Name for this BLE Device */
#define bleServerName "DHT11_ESP32-C3"

/* Enum for our LED */
typedef enum {
  RED=0,
  BLUE,
  GREEN
} LedColor_t;

/* New DHT11 Sensor Object */
DHT_Unified dht(DHTPIN, DHTTYPE);

bool deviceConnected = false;
bool oldDeviceConnected = false;

BLEServer* pServer = NULL;

const char service_uuid[] = "91bad492-b950-4226-aa2b-4ede9fa42f59";
BLECharacteristic dht11TemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dht11TemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic dht11HumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dht11HumidityDescriptor(BLEUUID((uint16_t)0x2902));


/*
    Class that contains callbacks for 
    onConnect and onDisconnect
*/
class BleServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

/**************************************************************************************************
 *    Function      : BLE_Setup
 *    Description   : Setup the BLE Server
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void BLE_Setup()
{
    // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BleServerCallbacks());

  // Create the BLE Service
  BLEService *dht11Service = pServer->createService(&service_uuid[0]);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Temperature
  dht11Service->addCharacteristic(&dht11TemperatureCelsiusCharacteristics);
    dht11TemperatureCelsiusDescriptor.setValue("Temp °C");
    dht11TemperatureCelsiusCharacteristics.addDescriptor(&dht11TemperatureCelsiusDescriptor);
  // Humidity
  dht11Service->addCharacteristic(&dht11HumidityCharacteristics);
    dht11HumidityDescriptor.setValue("Humidity");
    dht11HumidityCharacteristics.addDescriptor(&dht11HumidityDescriptor);
    
  // Start the service
  dht11Service->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(&service_uuid[0]);
  pServer->getAdvertising()->start();
  
}

/**************************************************************************************************
 *    Function      : setLed
 *    Description   : will set a RBG Led Color to a given brightness
 *    Input         : LedColor_t, uint8_t
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void setLed( LedColor_t color, uint8_t value){
  switch(color){
    case RED:{
        ledcWrite(1,value);
    } break;

    case GREEN:{
       ledcWrite(2,value);
    } break;

    case BLUE:{
      ledcWrite(0,value);
    } break;

    default:{
      
    } break;
    
  }
}

/**************************************************************************************************
 *    Function      : SetLedRGB
 *    Description   : Will set the RGB value of the onboard led
 *    Input         : uint8_t R, uint8_t G, uint8_t B
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void SetLedRGB(uint8_t R, uint8_t G, uint8_t B){
  setLed(RED,R);
  setLed(GREEN,G);
  setLed(BLUE,B);
}

/**************************************************************************************************
 *    Function      : SetLedOn
 *    Description   : Turns on the Led (white color)
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void SetLedOn( void ){
  setLed(RED,16);
  setLed(BLUE,16);
  setLed(GREEN,16);
}

/**************************************************************************************************
 *    Function      : SetLedOff
 *    Description   : Turns LED off
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void SetLedOff(void){
  setLed(RED,0);
  setLed(BLUE,0);
  setLed(GREEN,0);
}

/**************************************************************************************************
 *    Function      : ToggleLed
 *    Description   : Toggles the LED state
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void ToggleLed( void ){
  if(0==ledcRead(0)){
    SetLedOn();
  } else {
    SetLedOff();
  }
}

/**************************************************************************************************
 *    Function      : setup_led
 *    Description   : prepares the onboard led and PWM units
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void setup_led(void)
{
  pinMode(5,OUTPUT); 
  pinMode(3,OUTPUT); 
  pinMode(4,OUTPUT);
  //As the led will be bright we use a PWM to control color and brigthness
  ledcSetup(0,10000,8);
  ledcSetup(1,10000,8);
  ledcSetup(2,10000,8);
  ledcAttachPin(5, 0);
  ledcAttachPin(3, 1);
  ledcAttachPin(4, 2);
  SetLedOn();
  delay(500);
  SetLedOff();
}

/**************************************************************************************************
 *    Function      : setup()
 *    Description   : Setup function called by the Arduino-Framwork
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  setup_led();
  dht.begin();
 
  #ifdef Serial_Debug
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
  #endif
 
  BLE_Setup();
}


/**************************************************************************************************
 *    Function      : loop
 *    Description   : Superloop, called as fast as possible
 *    Input         : none
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void loop(void)
{
    static uint32_t last_dht_read=0;

    if (millis()>(last_dht_read+2000))
    {
        ToggleLed();
        #ifdef Serial_Debug
            Serial.printf("Start @ %i :\n\r",millis()); 
        #endif
        last_dht_read =millis();
        // Get temperature event and print its value.
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature))
        {
            #ifdef Serial_Debug
                Serial.println(F("Error reading temperature!"));
            #endif
        }
        else
        {
            #ifdef Serial_Debug
                Serial.print(F("Temperature: "));
                Serial.print(event.temperature);
                Serial.println(F("°C"));
            #endif
            //Set temperature Characteristic value and notify connected client
            if (pServer != NULL)
            {
                dht11TemperatureCelsiusCharacteristics.setValue(String(event.temperature).c_str());
                dht11TemperatureCelsiusCharacteristics.notify();
                delay(5); // One remark often found with the ESP32: bluetooth stack will go into congestion, if too many packets are sent.
            }
        }
        // Get humidity event and print its value.
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity))
        {
            #ifdef Serial_Debug
                Serial.println(F("Error reading humidity!"));
            #endif
        }
        else
        {
            #ifdef Serial_Debug
                Serial.print(F("Humidity: "));
                Serial.print(event.relative_humidity);
                Serial.println(F("%"));
            #endif
            //Set humidity Characteristic value and notify connected client
            if (pServer != NULL)
            {
                dht11HumidityCharacteristics.setValue(String(event.relative_humidity).c_str());
                dht11HumidityCharacteristics.notify();  
                delay(5); // One remark often found with the ESP32: bluetooth stack will go into congestion, if too many packets are sent.
               
            }
        }
    }
    
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) 
    {
        delay(500); // give the bluetooth stack the chance to get things ready
        if (pServer != NULL)
        { 
            pServer->startAdvertising(); // restart advertising
            #ifdef Serial_Debug
                Serial.println("start advertising");
            #endif
        }
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) 
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
