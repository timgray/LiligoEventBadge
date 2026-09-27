#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "utilities.h"
#include "Display.h"
#include "Touch.h"
#include "RTC.h"
#include "Storage.h"
#include "Menu.h"
#include "Schedule.h"
void Home(){Menu::Show();}
void setup(){Serial.begin(115200);delay(250);Wire.begin(BOARD_SDA,BOARD_SCL);SPI.begin(BOARD_SPI_SCLK,BOARD_SPI_MISO,BOARD_SPI_MOSI);if(!Display::Begin()){Serial.println("Display init failed");while(true)delay(1000);}BadgeRTC::Begin();Storage::Begin();Touch::SetHomeCallback(Home);Touch::Begin();Menu::Show();}
void loop(){Display::Loop();Schedule::Update();delay(5);}
