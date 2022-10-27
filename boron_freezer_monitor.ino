// This #include statement was automatically added by the Particle IDE.
#include <DS18.h>

DS18 sensor(D5);
String alert1;
bool onUSB = false;
bool onBattery = false;
bool lowBattery = false;
bool doorState = false;
const int reedPin = 7;
unsigned long checkTimer;//Check power every 10sec

void setup() {
    //Reed switch setup
    pinMode(reedPin, INPUT_PULLUP);
    //Initial power check
    int powerSource = System.powerSource();
    if (powerSource == POWER_SOURCE_BATTERY) {//On battery power
        onBattery = true;
        onUSB = false;
    }
    else{//On USB power
        onBattery = false;
        onUSB = true;
    }
    
    if(onBattery){//Bootup power source alert
         alert1 = "Battery powered";
         sendData();
    }else{
         alert1 = "AC powered";
         sendData();        
    }
    
    checkTimer = millis();
}

void loop() {
  //Power check loop
    if(millis()-checkTimer>10000){
    doorState = digitalRead(reedPin); //Read door state
    if (doorState){
          Particle.publish("Door", "The door is open");
    } else {
          Particle.publish("Door", "The door is closed");
    }
    checkTimer = millis();
    int powerSource = System.powerSource();
    if (powerSource == POWER_SOURCE_BATTERY) {//On battery power
        if(!onBattery && onUSB){//Changed from USB power to battery power
         onBattery = true;
         onUSB = false;
         alert1 = "AC power lost!";
         sendData();
        }
    }else if(onBattery && !onUSB){//Changed from battery power to USB power
        onBattery = false;
        onUSB = true;
        alert1 = "AC power is back on";
        sendData();
    }
    if (onBattery){
        reportTemp();
    }
    //Check battery voltage 
    FuelGauge fuel;
    float batteryVoltage = fuel.getVCell();
    if(batteryVoltage < 3.5){//Send low battery alert
        if(!lowBattery){
            lowBattery=true;
            alert1 = "Low battery!";
            sendData();
        }
    }else if(batteryVoltage>3.7){//Message spamming prevention
        lowBattery=false;
    }
  }
}

void sendData(){//Send a message via Telegram
     unsigned long startConnectTime = millis();
     Particle.publish("Boron2Telegram", alert1, PRIVATE);
}

void reportTemp(){//Read from the temperature sensor and report via Telegram
    bool success = sensor.read();
    if(!success){
        sensor.read();
    }
    Particle.publish("Temp", String(sensor.fahrenheit()));
    Particle.publish("Boron2Telegram", "Temp: " + String(sensor.fahrenheit()), PRIVATE);
}