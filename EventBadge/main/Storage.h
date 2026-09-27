#pragma once
#include <Arduino.h>
class Storage {public: static bool Begin(); static bool Ready(); static String ReadTextFile(const char*); private: static bool ready;};
