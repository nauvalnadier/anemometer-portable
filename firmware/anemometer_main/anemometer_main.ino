#include <DS3232RTC.h>
#include <Streaming.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <SPI.h>
#include <SD.h>

DS3232RTC myRTC;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

int OptoPin = 2;
float velocity = 0;
volatile byte pulses = 0;
unsigned long timeOld = 0;
const float pulsesperturn = 16.0;
const float wheel_diameter = 0.06;
static volatile unsigned long debounce = 0;

const int SAMPLE_INTERVAL = 5000;
const int NUM_SAMPLES = 5;

const float CALIBRATION_SLOPE = 10.51;
const float CALIBRATION_INTERCEPT = 0.43;

const int BUFFER_SIZE = 12;
struct DataPoint {
  time_t timestamp;
  float velocity;
  float azimuth;
};
DataPoint dataBuffer[BUFFER_SIZE];
int bufferIndex = 0;

float velocitySamples[NUM_SAMPLES];
float azimuthSamples[NUM_SAMPLES];
int sampleIndex = 0;
unsigned long lastSampleTime = 0;
unsigned long intervalStart = 0;

const int chipSelect = 10;
bool sdCardAvailable = false;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  myRTC.begin();
  setSyncProvider([]() { return myRTC.get(); });
  Serial.println(F("ANEMOMETER PORTABEL by Nauval Nadier N"));
  Serial.print(F("RTC Sync: "));
  Serial.println(timeStatus() != timeSet ? F("FAIL!") : F("OK"));

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(chipSelect)) {
    Serial.println(F("FAIL!"));
    sdCardAvailable = false;
  } else {
    Serial.println(F("OK"));
    sdCardAvailable = true;
    if (!SD.exists("datalog.txt")) {
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.println(F("========================================"));
        dataFile.println(F("ANEMOMETER PORTABEL by Nauval Nadier N"));
        dataFile.println(F("========================================"));
        dataFile.println(F("Date       Time      Velocity(m/s)  Azimuth(°)"));
        dataFile.println(F("========================================"));
        dataFile.close();
        Serial.println(F("File header created"));
      }
    }
  }

  if (!mag.begin()) {
    Serial.println(F("Compass not detected! Check wiring."));
    while (1);
  }
  mag.setMagGain(HMC5883_MAGGAIN_8_1);
  Serial.println(F("Compass initialized"));

  pinMode(OptoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(OptoPin), counter, RISING);
  Serial.println(F("Wind speed sensor initialized"));

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("Date       Time      Velocity(m/s)  Azimuth(°)"));
  Serial.println(F("========================================"));

  intervalStart = millis();
  lastSampleTime = millis();
  bufferIndex = 0;
}

void loop() {
  unsigned long timeNew = millis();

  if (timeNew - timeOld >= 1000) {
    noInterrupts();
    unsigned int currentPulses = pulses;
    pulses = 0;
    interrupts();

    float rawVelocity = (PI * wheel_diameter * (currentPulses / pulsesperturn))
                        / ((timeNew - timeOld) / 1000.0);
    velocity = (CALIBRATION_SLOPE * rawVelocity) + CALIBRATION_INTERCEPT;
    if (velocity < 0) velocity = 0;

    sensors_event_t event;
    mag.getEvent(&event);
    float a = atan2(event.magnetic.y, event.magnetic.x) * 180.0 / PI;
    if (a < 0) a += 360.0;

    velocitySamples[sampleIndex] = velocity;
    azimuthSamples[sampleIndex]  = a;
    sampleIndex++;

    timeOld = timeNew;
  }

  if (timeNew - intervalStart >= SAMPLE_INTERVAL && sampleIndex >= NUM_SAMPLES) {
    float avgVelocity = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) avgVelocity += velocitySamples[i];
    avgVelocity /= NUM_SAMPLES;

    float sumSin = 0, sumCos = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
      float rad = azimuthSamples[i] * PI / 180.0;
      sumSin += sin(rad);
      sumCos += cos(rad);
    }
    float avgAzimuth = atan2(sumSin / NUM_SAMPLES, sumCos / NUM_SAMPLES) * 180.0 / PI;
    if (avgAzimuth < 0) avgAzimuth += 360.0;

    // Koreksi arah berlawanan jarum jam
    avgAzimuth = fmod((360.0 - avgAzimuth), 360.0);

    time_t t = now();

    printDateTime(t);
    Serial.print(F("  "));
    Serial.print(avgVelocity, 5);
    Serial.print(F("      "));
    Serial.print(avgAzimuth, 1);
    Serial.println();

    dataBuffer[bufferIndex].timestamp = t;
    dataBuffer[bufferIndex].velocity  = avgVelocity;
    dataBuffer[bufferIndex].azimuth   = avgAzimuth;
    bufferIndex++;

    Serial.print(F("Buffer: "));
    Serial.print(bufferIndex);
    Serial.print(F("/"));
    Serial.println(BUFFER_SIZE);

    if (bufferIndex >= BUFFER_SIZE) {
      flushBufferToSD();
      bufferIndex = 0;
    }

    sampleIndex   = 0;
    intervalStart = timeNew;
  }

  if (Serial.available() >= 12) setRTCTime();
}

void flushBufferToSD() {
  if (!sdCardAvailable) {
    Serial.println(F("SD card not available, data lost!"));
    return;
  }
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    for (int i = 0; i < bufferIndex; i++) {
      time_t t = dataBuffer[i].timestamp;
      if (day(t)    < 10) dataFile.print('0'); dataFile.print(day(t));    dataFile.print('/');
      if (month(t)  < 10) dataFile.print('0'); dataFile.print(month(t));  dataFile.print('/');
      dataFile.print(year(t));                 dataFile.print(' ');
      if (hour(t)   < 10) dataFile.print('0'); dataFile.print(hour(t));   dataFile.print(':');
      if (minute(t) < 10) dataFile.print('0'); dataFile.print(minute(t)); dataFile.print(':');
      if (second(t) < 10) dataFile.print('0'); dataFile.print(second(t)); dataFile.print("  ");
      dataFile.print(dataBuffer[i].velocity, 5);
      dataFile.print("      ");
      dataFile.print(dataBuffer[i].azimuth, 1);
      dataFile.println();
    }
    dataFile.close();
    Serial.print(F("Flushed "));
    Serial.print(bufferIndex);
    Serial.println(F(" records to SD card"));
  } else {
    Serial.println(F("Error opening datalog.txt"));
  }
}

void counter() {
  if (digitalRead(OptoPin) && (micros() - debounce > 500) && digitalRead(OptoPin)) {
    debounce = micros();
    pulses++;
  }
}

void printDateTime(time_t t) { printDate(t); Serial.print(F(" ")); printTime(t); }
void printTime(time_t t)     { printI00(hour(t), ':'); printI00(minute(t), ':'); printI00(second(t), ' '); }
void printDate(time_t t)     { printI00(day(t), '/'); printI00(month(t), '/'); Serial.print(year(t)); }
void printI00(int val, char delim) {
  if (val < 10) Serial.print('0');
  Serial.print(val);
  if (delim > 0) Serial.print(delim);
}

void setRTCTime() {
  if (bufferIndex > 0) { flushBufferToSD(); bufferIndex = 0; }
  int y = Serial.parseInt();
  if (y >= 100 && y < 1000) {
    Serial.println(F("Error: Year must be two digits or four digits!"));
  } else {
    tmElements_t tm;
    tm.Year   = (y >= 1000) ? CalendarYrToTm(y) : y2kYearToTm(y);
    tm.Month  = Serial.parseInt();
    tm.Day    = Serial.parseInt();
    tm.Hour   = Serial.parseInt();
    tm.Minute = Serial.parseInt();
    tm.Second = Serial.parseInt();
    time_t t = makeTime(tm);
    myRTC.set(t); setTime(t);
    Serial.print(F("RTC set to: ")); printDateTime(t); Serial.println();
    while (Serial.available() > 0) Serial.read();
  }
}
