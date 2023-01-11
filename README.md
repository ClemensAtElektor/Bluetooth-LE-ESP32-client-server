# Bluetooth-LE-ESP32-client-server
Simple Bluetooth client-server setup on two ESP32 modules for trying out the nRF52840 BLE sniffer.

Arduino sketches based on Elektor Magazine article [Bluetooth Low Energy with ESP32-C3 and ESP32](https://www.elektormagazine.com/magazine/elektor-272/60930) (Nr. 516, September & October 2022)

The sensor module has a DHT11 sensor connected to pin 8 of an ESP32-C3 NodeMCU module.
The display module consists of an ESP32 Pico Kit v4 module with an 0.96" I²C OLED display connected to pin 21 (SDA) and pin 22 (SCL).

Required libraries:
- DHT sensor library
- Adafruit Unified Sensor
- U8g2

For explanations how all this is supposed to be used, see this [video](https://www.youtube.com/@ElektorTV) (placeholder, search this channel)
