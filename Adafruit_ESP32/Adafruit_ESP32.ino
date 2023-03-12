// Universidad del Valle de Guatemala
// Electrónica Digital 2
// Oscar Donis y Carlos Molina
// 08/03/2023

/**************************** Configuration ************************************/

// WI-FI
#include "config.h"

// Servo
#include <ESP32Servo.h>

// UART
#include <Arduino.h>
#include <HardwareSerial.h>

/****************************** Variables ************************************/
Servo servo1;
ESP32PWM pwm;

int minUs = 500;
int maxUs = 2600;

int count = 0;
int temp = 0;
int temporal = 1;
int servo1Pin = 15;
int pos = 0;        

/************************ Prototipo de Funcion *******************************/

/******************************* Main ****************************************/
// set up the feeds
AdafruitIO_Feed *counter = io.feed("Lots");
AdafruitIO_Feed *temperaturefeed = io.feed("Temperature");
AdafruitIO_Feed *proximityfeed = io.feed("Proximity");
AdafruitIO_Feed *rotfeed = io.feed("Rotation");

void setup() {
  pinMode(3, INPUT);
  
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // start the serial connection 
  Serial.begin(9600);             // 9600 

  servo1.setPeriodHertz(50);      // Standard 50hz servo

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");
  
  // connect to io.adafruit.com
  io.connect();
  
  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  rotfeed->onMessage(handleMessage);

  
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(1);
  }

  // Because Adafruit IO doesn't support the MQTT retain flag, we can use the
  // get() function to ask IO to resend the last value for this feed to just
  // this MQTT client after the io client is connected.
  //rotfeed->get();

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
}

void loop() {
  servo1.attach(servo1Pin, minUs, maxUs);
  
  #if defined(ARDUINO_ESP32S2_DEV)
  pwm.attachPin(37, 10000);//10khz
  #else
  pwm.attachPin(27, 10000);//10khz
  #endif

  // Guardamos y almacenamos los datos entrantes del UART 
  if (Serial.available()>0) {
    int data = Serial.read();
    int temporal = data;
    
    if (temporal <= 3){
      count = data;
    }
    else{
      temp = data;
    }
    }
  
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  // Obtenemos datos de la comunicación UART
  

  //Enviamos numerode parqueos
  Serial.print("sending -> ");
  Serial.println(count);
  counter->save(count);
  
  //Enviamos temperatura
  Serial.print("sending -> ");
  Serial.println(temp);
  temperaturefeed->save(temp);

  // Adafruit IO is rate limited for publishing, so a delay is required in
  // between feed->save events. In this example, we will wait three seconds
  // (1000 milliseconds == 1 second) during each loop.
  servo1.detach();
  pwm.detachPin(27);
  
  // Esperamos 6 segundos para no congestionar Adafruit con la cantidad de datos enviados como recibidos.
  delay(6000);
}

// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.

void handleMessage(AdafruitIO_Data *data) {
  
  // Recibimos instrucción del botón de Adafruit
  Serial.print("received <- ");
  Serial.println(data->value());

  if(data->toPinLevel() == HIGH){
    for (pos = 0; pos <= 180; pos += 1) { // Mover de 0 grados a 180 grados
    servo1.write(pos);
    delay(1);             // Esperar 20ms para que el servo alcance la posición
    }
  }
  if(data->toPinLevel() == LOW){
    for (pos = 180; pos >= 0; pos -= 1) { // Mover de 180 grados a 0 grados
      servo1.write(pos);
      delay(1);           // Esperar 20ms para que el servo alcance la posición
    }
  }
}