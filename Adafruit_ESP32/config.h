/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME  "mol21253"
#define IO_KEY       "aio_VRNq21Cqqx8gxtKBCpuA7YCFbSpN"

/******************************* WIFI **************************************/

#define WIFI_SSID "iPhone57"
#define WIFI_PASS "ojito123"

// comment out the following lines if you are using fona or ethernet
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);