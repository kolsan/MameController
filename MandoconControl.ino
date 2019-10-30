#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Keyboard.h>
#include <movingAvg.h>

//MPU
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

//filter
// Original values were 200 and then 600
const int PressedMaxThreshold = 360;
const int ReleasedMinThreshold = 450;
const byte PinCount = 1;
const byte InputPins[PinCount] = {A0};

struct TouchInput
{
  byte analogPin;
  char keycode;
  movingAvg filter = movingAvg(20);
  boolean wasPressed = false;
};


TouchInput Pins[PinCount];

#define MPU6050_ACCEL_OFFSET_X -4680
#define MPU6050_ACCEL_OFFSET_Y 849
#define MPU6050_ACCEL_OFFSET_Z 2709
#define MPU6050_GYRO_OFFSET_X  61
#define MPU6050_GYRO_OFFSET_Y  -34
#define MPU6050_GYRO_OFFSET_Z  -33

int Horizontal;
int Vertical;

const int buttonPin = 8;
const int fanpin = 10;
const int lightpin = 13;
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
boolean isplaying = false;
boolean KeyHorizontalPressed = false;
boolean KeyVerticalPressed = false;
boolean KeyPedalPressed = false;
boolean FanIsRunning = false;
boolean SafeExitExecuted = false;
unsigned long time = 0;
unsigned long fantime = 10000;
unsigned long pedaltime = 10000;

void setup()
{



  Wire.begin();
  Serial.begin(9600);
  while (!Serial); // wait for Leonardo enumeration, others continue immediately

  Serial.println("Initialize MPU");
  mpu.initialize();
  Serial.println(mpu.testConnection() ? "Connected" : "Connection failed");
  mpu.setXAccelOffset(MPU6050_ACCEL_OFFSET_X);
  mpu.setYAccelOffset(MPU6050_ACCEL_OFFSET_Y);
  mpu.setZAccelOffset(MPU6050_ACCEL_OFFSET_Z);
  mpu.setXGyroOffset(MPU6050_GYRO_OFFSET_X);
  mpu.setYGyroOffset(MPU6050_GYRO_OFFSET_Y);
  mpu.setZGyroOffset(MPU6050_GYRO_OFFSET_Z);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Horizontal = ay;
  Vertical = az;

  Serial.print(" Start values x: ");
  Serial.print(Vertical);
  Serial.print(" y: ");
  Serial.println(Horizontal);
  Keyboard.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, OUTPUT);
  pinMode(lightpin, OUTPUT);
  pinMode(fanpin, OUTPUT);
  for (int i = 0; i < PinCount; i++)
  {
    Pins[i].analogPin = InputPins[i];
    Pins[i].filter.begin();
  }
  Serial.println("Starting...10 secs to go");
  delay(10000);
}

void loop()
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    isplaying = !isplaying;
    if (isplaying) {
      digitalWrite(lightpin, HIGH);
    } else {
      digitalWrite(lightpin, LOW);
    }

    digitalWrite(fanpin, LOW);
    Keyboard.releaseAll();
    delay(3000);
  }
  if (isplaying == true)
  {
    //Serial.println ("Partida comenzada!!");
    //digitalWrite(lightpin, HIGH);
    //Si el ventilador lleva funcionando 10sgs lo apago
    if (fantime + 6000 < millis ( ) && FanIsRunning)
    {
      digitalWrite(fanpin, LOW);
      Serial.println ("Apagado ventilador");
      FanIsRunning = false;
    }

    for (int i = 0; i < PinCount; i++)
    {
      float currentAverage = Pins[i].filter.reading(analogRead(Pins[i].analogPin));
      //Serial.println (currentAverage);

      if (time + 1000 < millis () && KeyPedalPressed && !SafeExitExecuted)
      { Keyboard.release(120) ;
        Serial.println ("Libero de emergencia");
        SafeExitExecuted = true;
      }
      if (currentAverage < PressedMaxThreshold && KeyPedalPressed)
      {
        //Los dos pies en apoyados
        KeyPedalPressed = false;
        Keyboard.release(120) ;
        Serial.println ("Detectados 2 pies - no pedaleo");
        //Serial.print (currentAverage);rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
        //Serial.println (" dentro");
        //currentState = true;      // Pressed
        //
        //              Keyboard.press(122);
        //              delay(50);
        //              Keyboard.releaseAll();
        //              Keyboard.press(120);
        //              delay(50);
        //              Keyboard.releaseAll();

      }
      else if  (currentAverage > ReleasedMinThreshold && !KeyPedalPressed)
      {
        //levantado un pie

        //        if (time + 10 < millis ())
        //        {
        time = millis();
        Serial.println ("Detectado 1 pie - pedaleo enciendo ventilador");
        fantime = millis();
        digitalWrite(fanpin, HIGH);
        Keyboard.press(120);
        KeyPedalPressed = true;
        FanIsRunning = true;
        SafeExitExecuted = false;
        //        }
      }
    }




    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    Horizontal = map(ay, -17000, 17000, 0, 179);
    //Vertical =  map(ax, -17000, 17000, 0, 179);
    Vertical =  map(az, -17000, 17000, 0, 179);
//          Serial.print(" Vertical: ");
//          Serial.print(Vertical);
//          Serial.print(" Horizontal: ");
//          Serial.println(Horizontal);


    if (Horizontal > 57 && Horizontal < 120 && KeyHorizontalPressed)
    {
      Serial.print ("Liberamos Mov horizontal");
      Keyboard.release(114);
      Keyboard.release(108);
      KeyHorizontalPressed = false;
      //      delay(3000);
      Serial.print(" Vertical: ");
      Serial.print(Vertical);
      Serial.print(" Horizontal: ");
      Serial.println(Horizontal);
    }

    if (Horizontal > 120 && !KeyHorizontalPressed)
    { Serial.println (" Derecha");
      Keyboard.press(114);
      KeyHorizontalPressed = true;
    }
    //
    if (Horizontal <= 57 && !KeyHorizontalPressed)
    { Serial.println (" Izquierda");
      Keyboard.press(108);
      KeyHorizontalPressed = true;
    }

    if (Vertical > 110 && Vertical < 145 && KeyVerticalPressed)
    {
      Serial.print ("Liberamos Mov Vertical");
      Keyboard.release(100);
      Keyboard.release(117);
      KeyVerticalPressed = false;
      //delay (3000);
      Serial.print(" Vertical: ");
      Serial.print(Vertical);
      Serial.print(" Horizontal: ");
      Serial.println(Horizontal);
    }
    if (Vertical <= 110 && !KeyVerticalPressed)
    { Serial.println (" Arriba");
      Keyboard.press(100);
      KeyVerticalPressed = true;
    }
    if (Vertical > 145 && !KeyVerticalPressed)
    { Serial.println (" Abajo");
      Keyboard.press(117);
      KeyVerticalPressed = true;
    }

  }
}
