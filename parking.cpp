#include <Servo.h>
#include <LiquidCrystal.h>

Servo barrera;
LiquidCrystal lcd(3, 13, 9, 10, 11, 12);
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // LCD pin configuration

const int GREEN_LED = 2;
//const int RED_LED = 3;
const int OUTSIDE_SENSOR_ECHO = 4;
const int OUTSIDE_SENSOR_TRIGGER = 5;
const int INSIDE_SENSOR_ECHO = 7;
const int INSIDE_SENSOR_TRIGGER = 6;
const int SERVO_MOTOR = 8;

const long wait_time = 3000;
const int dist_trigger = 7;
const int open_angle = 90;

// Car count variable
int totalSpaces = 10;
int carCount = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(OUTSIDE_SENSOR_TRIGGER, OUTPUT);
  pinMode(OUTSIDE_SENSOR_ECHO, INPUT);
  pinMode(INSIDE_SENSOR_TRIGGER, OUTPUT);
  pinMode(INSIDE_SENSOR_ECHO, INPUT);

  barrera.attach(SERVO_MOTOR);
  lcd.begin(16, 2);

  // Display initial welcome message
  lcd.setCursor(0, 0);
  lcd.print("BIENVENIDO     ");
  lcd.setCursor(0, 1);
  lcd.print("OCUPADOS: 0     ");
}

// Function to calculate distance from a sensor
long calcDistfromSensor(int triggerPin, int echoPin) {
  long t;  // Time for echo
  long d;  // Distance in centimeters

  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);  // Send a 10us pulse
  digitalWrite(triggerPin, LOW);

  t = pulseIn(echoPin, HIGH);  // Measure the echo pulse
  d = t / 59;                  // Convert to distance in cm

  return d;
}

bool handleEntry() {
  // Measure the distance from the outside sensor
  long outsideDist = calcDistfromSensor(OUTSIDE_SENSOR_TRIGGER, OUTSIDE_SENSOR_ECHO);

  if (outsideDist < dist_trigger) {
    if (carCount == totalSpaces) {
      Serial.println("Parking lot is full.");
      lcd.setCursor(0, 0);
      lcd.print("ESTACIONAMIENTO");
      lcd.setCursor(0, 1);
      lcd.print("LLENO   ");
      return false;
    }
    barrera.write(open_angle);  // Open the barrier
    digitalWrite(GREEN_LED, HIGH);

    unsigned long startTime = millis();
    bool carEntered = false;

    Serial.println("Car in entrance.");

    // Wait 10 seconds for the car to pass the inside sensor
    while (millis() - startTime < wait_time) {
      //delay(100);
      long insideDist = calcDistfromSensor(INSIDE_SENSOR_TRIGGER, INSIDE_SENSOR_ECHO);
      Serial.print("INSIDE DIST: ");
      Serial.println(insideDist);

      if (insideDist < dist_trigger) {
        Serial.print("Detected car entry at ");
        Serial.println(insideDist);
        carEntered = true;
        break;
      }
    }

    barrera.write(0);  // Close the barrier
    digitalWrite(GREEN_LED, LOW);

    if (carEntered) {
      carCount++;
      lcd.setCursor(0, 0);
      lcd.print("BIENVENIDO     ");
      lcd.setCursor(0, 1);
      lcd.print("OCUPADOS: ");
      lcd.print(carCount);
      lcd.print("      ");  // Clear extra characters
      Serial.print("Car Entered. Total Cars in Parking: ");
      Serial.println(carCount);
      return true;
    } else {
      Serial.println("Car did not enter the parking lot.");
      return false;
    }
  }
}

bool handleExit() {
  // Measure the distance from the inside sensor
  long insideDist = calcDistfromSensor(INSIDE_SENSOR_TRIGGER, INSIDE_SENSOR_ECHO);

  if (insideDist < dist_trigger) {
    if (carCount == 0) {
      Serial.println("No cars in parking lot.");
      return false;
    }
    Serial.println("Car in exit.");
    barrera.write(open_angle);  // Open the barrier
    digitalWrite(GREEN_LED, HIGH);

    unsigned long startTime = millis();
    bool carExited = false;

    // Wait 10 seconds for the car to pass the outside sensor
    while (millis() - startTime < wait_time) {
      //delay(100);
      long outsideDist = calcDistfromSensor(OUTSIDE_SENSOR_TRIGGER, OUTSIDE_SENSOR_ECHO);
      Serial.print("OUSTIDE DIST: ");
      Serial.println(outsideDist);
      if (outsideDist < dist_trigger) {
        Serial.print("Detected car exit at ");
        Serial.println(insideDist);
        carExited = true;
        break;
      }
    }

    barrera.write(0);  // Close the barrier
    digitalWrite(GREEN_LED, LOW);

    if (carExited) {
        lcd.setCursor(0, 0);
        lcd.print("GRACIAS       ");
        lcd.setCursor(0, 1);
        lcd.print("OCUPADOS: ");
        lcd.print(carCount);
        lcd.print("      ");  // Clear extra characters
        Serial.print("Car Exited. Total Cars in Parking: ");
        Serial.println(carCount);
        return true;
    } else {
      Serial.println("Car did not exit the parking lot.");
      return false;
    }
  }
}

void loop() {
  bool barrier_activated;

  barrier_activated = false;
  barrier_activated = handleEntry();
  if (barrier_activated) delay(wait_time);

  barrier_activated = false;
  barrier_activated = handleExit();
  if (barrier_activated) delay(wait_time);
  Serial1.println(carCount);
}
