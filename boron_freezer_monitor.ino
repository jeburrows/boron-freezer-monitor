// This #include statement was automatically added by the Particle IDE.
#include <DS18.h>

DS18 sensor(D5);
String alert1;
String eventName = "Boron2Telegram";
bool onUSB = false;
bool onBattery = false;
bool lowBattery = false;
bool doorState = false;
const int reedPin = 7;
const int tempThreshold = 70; //Set high for testing. Change to freezer temp when installed.
const long powerTimer = 10000;
const long doorTimer = 60000;
const long tempTimer = 300000;
unsigned long powerDelay = 0;
unsigned long doorDelay = 0;
unsigned long tempDelay = 0;

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
        Particle.publish("Power", alert1);
        Particle.publish(eventName, alert1, PRIVATE);
    }else{
        alert1 = "AC powered";
        Particle.publish("Power", alert1);
        Particle.publish(eventName, alert1, PRIVATE);     
    }
}

void loop() {//Status check loop
    //Timer
    unsigned long currentTime = millis();
    
    //Read power state
    if (currentTime - powerDelay >= powerTimer) {
        reportPower();
        powerDelay = currentTime;
    }
    
    //Read the door state
    if (currentTime - doorDelay >= doorTimer) {
        reportDoor();
        doorDelay = currentTime;
    }
    
    //Read the temperature sensor
    if (currentTime - tempDelay >= tempTimer) {
        reportTemp();
        tempDelay = currentTime;
    }
}

void reportPower(){//Send a message via Telegram
    int powerSource = System.powerSource();
    if (powerSource == POWER_SOURCE_BATTERY) {//On battery power
        if(!onBattery && onUSB){//Changed from USB power to battery power
            onBattery = true;
            onUSB = false;
            alert1 = "AC power lost!";
            Particle.publish("Power", alert1);
            Particle.publish(eventName, alert1, PRIVATE);
        }
    }else if(onBattery && !onUSB){//Changed from battery power to USB power
            onBattery = false;
            onUSB = true;
            alert1 = "AC power is back on";
            Particle.publish("Power", alert1);
            Particle.publish(eventName, alert1, PRIVATE);
    }
    //Check battery voltage 
    FuelGauge fuel;
    float batteryVoltage = fuel.getVCell();
    if(batteryVoltage < 3.5){//Send low battery alert
        if(!lowBattery){
            lowBattery=true;
            alert1 = "Low battery!";
            reportPower();
        }
    }else if(batteryVoltage>3.7){//Message spamming prevention
        lowBattery=false;
    }
}

void reportDoor(){//Read from the door sensor and report via Telegram if the door is open
    doorState = digitalRead(reedPin);
    if (doorState){
        Particle.publish("Door", "The door is open!");
        Particle.publish(eventName, "The door is open!", PRIVATE);
    }
    else {
        Particle.publish("Door", "The door is closed");
    }
}

void reportTemp(){//Read from the temperature sensor and report via Telegram if the temperature is too high
    bool success = sensor.read();
    if(!success){//If reading the temperature sensor fails...
        sensor.read(); //try again
    }
    float tempSensor = sensor.fahrenheit();
    if (tempSensor > tempThreshold){//Change the temp to your desired freezer temp (ie. 32)
        Particle.publish("Temp", String(sensor.fahrenheit()));
        Particle.publish(eventName, "Temp: " + String(sensor.fahrenheit()), PRIVATE);
    }
    else {
        Particle.publish("Temp", String(sensor.fahrenheit()));
    }
}