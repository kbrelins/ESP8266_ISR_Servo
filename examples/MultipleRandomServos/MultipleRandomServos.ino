/****************************************************************************************************************************
 * examples/MultipleRandomServos.ino
 * For ESP8266 boards
 * Written by Khoi Hoang
 * 
 * Built by Khoi Hoang https://github.com/khoih-prog/ESP8266_ISR_Servo
 * Licensed under MIT license
 * Version: v1.0.0
 * 
 * The ESP8266 timers are badly designed, using only 23-bit counter along with maximum 256 prescaler. They're only better than UNO / Mega.
 * The ESP8266 has two hardware timers, but timer0 has been used for WiFi and it's not advisable to use. Only timer1 is available.
 * The timer1's 23-bit counter terribly can count only up to 8,388,607. So the timer1 maximum interval is very short.
 * Using 256 prescaler, maximum timer1 interval is only 26.843542 seconds !!!
 * 
 * Now these new 16 ISR-based PWM servo contro uses only 1 hardware timer.
 * The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
 * Therefore, their executions are not blocked by bad-behaving functions / tasks.
 * This important feature is absolutely necessary for mission-critical tasks. 
 * 
 * Notes:
 * Special design is necessary to share data between interrupt code and the rest of your program.
 * Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume 
 * variable can not spontaneously change. Because your function may change variables while your program is using them, 
 * the compiler needs this hint. But volatile alone is often not enough.
 * When accessing shared variables, usually interrupts must be disabled. Even with volatile, 
 * if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly. 
 * If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled 
 * or the entire sequence of your code which accesses the data.
 *
 * Version Modified By   Date      Comments
 * ------- -----------  ---------- -----------
 *  1.0.0   K Hoang      04/12/2019 Initial release
*****************************************************************************************************************************/

/****************************************************************************************************************************
 * This example will demonstrate the nearly perfect accuracy compared to software timers by printing the actual elapsed millisecs.
 * Being ISR-based timers, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet
 * and Blynk services. You can also have many (up to 16) timers to use.
 * This non-being-blocked important feature is absolutely necessary for mission-critical tasks. 
 * You'll see blynkTimer is blocked while connecting to WiFi / Internet / Blynk, and elapsed time is very unaccurate
 * In this super simple example, you don't see much different after Blynk is connected, because of no competing task is
 * written
 * 
 * From ESP32 Servo Example Using Arduino ESP32 Servo Library
 * John K. Bennett
 * March, 2017
 * 
 * Different servos require different pulse widths to vary servo angle, but the range is 
 * an approximately 500-2500 microsecond pulse every 20ms (50Hz). In general, hobbyist servos
 * sweep 180 degrees, so the lowest number in the published range for a particular servo
 * represents an angle of 0 degrees, the middle of the range represents 90 degrees, and the top
 * of the range represents 180 degrees. So for example, if the range is 1000us to 2000us,
 * 1000us would equal an angle of 0, 1500us would equal 90 degrees, and 2000us would equal 1800
 * degrees.
 * 
 * Circuit:
 * Servo motors have three wires: power, ground, and signal. The power wire is typically red,
 * the ground wire is typically black or brown, and the signal wire is typically yellow,
 * orange or white. Since the ESP32 can supply limited current at only 3.3V, and servos draw
 * considerable power, we will connect servo power to the VBat pin of the ESP32 (located
 * near the USB connector). THIS IS ONLY APPROPRIATE FOR SMALL SERVOS. 
 * 
 * We could also connect servo power to a separate external
 * power source (as long as we connect all of the grounds (ESP32, servo, and external power).
 * In this example, we just connect ESP32 ground to servo ground. The servo signal pins
 * connect to any available GPIO pins on the ESP32 (in this example, we use pins
 * 22, 19, 23, & 18).
 * 
 * In this example, we assume four Tower Pro SG90 small servos.
 * The published min and max for this servo are 500 and 2400, respectively.
 * These values actually drive the servos a little past 0 and 180, so
 * if you are particular, adjust the min and max values to match your needs.
 * Experimentally, 550 and 2350 are pretty close to 0 and 180.* 
*****************************************************************************************************************************/

#define TIMER_INTERRUPT_DEBUG       1
#define ISR_SERVO_DEBUG             0

#include "ESP8266_ISR_Servo.h"

// Published values for SG90 servos; adjust if needed
#define MIN_MICROS      800  //544
#define MAX_MICROS      2450

#define NUM_SERVOS    6

typedef struct
{
  int     servoIndex;
  uint8_t servoPin;
} ISR_servo_t;

ISR_servo_t ISR_servo[NUM_SERVOS] =
{
  { -1, D0 }, { -1, D1 }, { -1, D2 }, { -1, D5 }, { -1, D6 }, { -1, D7 } 
};

void setup() 
{
  Serial.begin(115200);
  Serial.println("\nStarting");

  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_servo[index].servoIndex = ISR_Servo.setupServo(ISR_servo[index].servoPin, MIN_MICROS, MAX_MICROS);
    
    if (ISR_servo[index].servoIndex != -1)
      Serial.println("Setup OK Servo index = " + String(ISR_servo[index].servoIndex));
    else
      Serial.println("Setup Failed Servo index = " + String(ISR_servo[index].servoIndex));
  }
}

void loop() 
{
  int position;      // position in degrees

  position = 0;
  Serial.println("Servos @ 0 degree");
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_Servo.setPosition(ISR_servo[index].servoIndex, position );
    Serial.print("Servos idx = " + String(index) + ", act. pos. (deg) = " + String(ISR_Servo.getPosition(ISR_servo[index].servoIndex)) );
    Serial.println(", pulseWidth (us) = " + String(ISR_Servo.getPulseWidth(ISR_servo[index].servoIndex)) );
 }
  // waits 5s between test
  delay(5000);

  position = 90;
  Serial.println("Servos @ 90 degree");
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_Servo.setPosition(ISR_servo[index].servoIndex, position );
    Serial.print("Servos idx = " + String(index) + ", act. pos. (deg) = " + String(ISR_Servo.getPosition(ISR_servo[index].servoIndex)) );
    Serial.println(", pulseWidth (us) = " + String(ISR_Servo.getPulseWidth(ISR_servo[index].servoIndex)) );
  }
  // waits 5s between test
  delay(5000);

  position = 180;
  Serial.println("Servos @ 180 degree");
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_Servo.setPosition(ISR_servo[index].servoIndex, position );
    Serial.print("Servos idx = " + String(index) + ", act. pos. (deg) = " + String(ISR_Servo.getPosition(ISR_servo[index].servoIndex)) );
    Serial.println(", pulseWidth (us) = " + String(ISR_Servo.getPulseWidth(ISR_servo[index].servoIndex)) );
  }
  // waits 5s between test
  delay(5000);
  
  Serial.println("Servos sweeps from 0-180 degress");
  for (position = 0; position <= 180; position += 1) 
  { 
    // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    for (int index = 0; index < NUM_SERVOS; index++)
    {
      ISR_Servo.setPosition(ISR_servo[index].servoIndex, position );
    }
    // waits 50ms for the servo to reach the position
    delay(50);
  }
  // waits 5s between test
  delay(5000);

  Serial.println("Servos sweeps from 180-0 degress");  
  for (position = 180; position >= 0; position -= 1) 
  { 
    // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    for (int index = 0; index < NUM_SERVOS; index++)
    {
      ISR_Servo.setPosition(ISR_servo[index].servoIndex, position );
    }
    // waits 50ms for the servo to reach the position
    delay(50);
  }
  // waits 5s between test
  delay(5000);

  Serial.println("Servos, index depending, be somewhere from 0-180 degress");
  for (position = 0; position <= 180; position += 1) 
  { 
    // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    for (int index = 0; index < NUM_SERVOS; index++)
    {
      ISR_Servo.setPosition(ISR_servo[index].servoIndex, (position + index * (180 / NUM_SERVOS)) % 180 );
    }
    // waits 50ms for the servo to reach the position
    delay(50);
  }
  delay(5000);

  Serial.println("Servos, index depending, be somewhere from 180-0 degress");
  for (position = 180; position >= 0; position -= 1) 
  { 
    // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    for (int index = 0; index < NUM_SERVOS; index++)
    {
      ISR_Servo.setPosition(ISR_servo[index].servoIndex, (position + index * (180 / NUM_SERVOS)) % 180 );
    }
    // waits 50ms for the servo to reach the position
    delay(50);
  }
  // waits 5s between test
  delay(5000);
  
}
