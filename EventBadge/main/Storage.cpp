#include "Storage.h"
#include "utilities.h"
#include <SD.h>
#include <SPI.h>
bool Storage::ready=false;
bool Storage::Begin(){pinMode(BOARD_SD_CS,OUTPUT);digitalWrite(BOARD_SD_CS,HIGH);ready=SD.begin(BOARD_SD_CS);return ready;}
bool Storage::Ready(){return ready;}
String Storage::ReadTextFile(const char*p){if(!ready)return "";File f=SD.open(p,FILE_READ);if(!f)return "";String s;while(f.available())s+=(char)f.read();f.close();return s;}
