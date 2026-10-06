#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// =========================
// PINS
// =========================
#define ANALOG_PIN   34
#define DIGITAL_PIN  27

// Digital microphone output
#define BEAT_STATE LOW

// =========================
// HEART SOUND SETTINGS
// =========================
#define MIN_SOUND_GAP   150
#define MIN_S1_S2_GAP   100
#define MAX_S1_S2_GAP    500
#define BPM_WINDOW      5000

// =========================
// AUDIO SETTINGS
// =========================
#define SAMPLE_RATE 4000

const unsigned long SAMPLE_INTERVAL = 1000000UL / SAMPLE_RATE;
unsigned long nextSampleTime = 0;

// =========================
// DETECTION VARIABLES
// =========================
int previousDigitalState = HIGH;
unsigned long lastSoundTime = 0;
unsigned long s1Time = 0;
unsigned long s2Time = 0;
bool waitingForS2 = false;

unsigned long totalSoundNumber = 0;
unsigned long totalHeartBeatNumber = 0;

int soundCount5sec = 0;
int heartBeatCount5sec = 0;
unsigned long windowStartTime;

// =====================================================
// SEND TEXT TO BLUETOOTH
// =====================================================
void BT(String message) {
  SerialBT.println(message);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-HEART");

  pinMode(ANALOG_PIN, INPUT);
  pinMode(DIGITAL_PIN, INPUT);

  windowStartTime = millis();
  nextSampleTime = micros();

  BT("================================");
  BT("ESP32 HEART SOUND MONITOR");
  BT("================================");
  BT("Bluetooth connected");
  BT("S1 + S2 = ONE HEARTBEAT");
  BT("BPM calculated every 5 seconds");
  BT("================================");
}

// =====================================================
// SEND AUDIO PACKET
// =====================================================
#define AUDIO_BLOCK 128
uint8_t audioBuffer[AUDIO_BLOCK];
int audioIndex = 0;

void sendAudioBlock() {
  uint8_t checksum = 0;
  Serial.write(0xA5);
  Serial.write(0x5A);
  Serial.write((uint8_t)AUDIO_BLOCK);

  for (int i = 0; i < AUDIO_BLOCK; i++) {
    Serial.write(audioBuffer[i]);
    checksum += audioBuffer[i];
  }
  Serial.write(checksum);
}

void loop() {
  unsigned long now = millis();

  // DIGITAL HEART SOUND DETECTION
  int digitalState = digitalRead(DIGITAL_PIN);

  if (previousDigitalState == HIGH && digitalState == BEAT_STATE) {
    unsigned long soundTime = millis();

    if (lastSoundTime == 0 || soundTime - lastSoundTime >= MIN_SOUND_GAP) {
      totalSoundNumber++;
      soundCount5sec++;

      if (!waitingForS2) {
        s1Time = soundTime;
        waitingForS2 = true;
        BT("S1 detected");
        BT("Sound #" + String(totalSoundNumber));
      } else {
        s2Time = soundTime;
        unsigned long gap = s2Time - s1Time;

        if (gap >= MIN_S1_S2_GAP && gap <= MAX_S1_S2_GAP) {
          totalHeartBeatNumber++;
          heartBeatCount5sec++;
          BT("S2 detected");
          BT("Sound #" + String(totalSoundNumber));
          BT("S1-S2 gap = " + String(gap) + " ms");
          BT("Heartbeat #" + String(totalHeartBeatNumber));
          BT("--------------------------------");
          waitingForS2 = false;
        } else {
          s1Time = soundTime;
          waitingForS2 = true;
          BT("New S1 detected");
        }
      }
      lastSoundTime = soundTime;
    }
  }

  previousDigitalState = digitalState;

  // 5 SECOND BPM CALCULATION
  if (now - windowStartTime >= BPM_WINDOW) {
    float bpm = heartBeatCount5sec * 12.0;
    BT("");
    BT("========== 5 SECOND RESULT ==========");
    BT("Total sounds = " + String(soundCount5sec));
    BT("S1-S2 pairs = " + String(heartBeatCount5sec));
    BT("BPM = " + String(bpm, 1));
    BT("Reference BPM = 72");
    BT("Difference = " + String(bpm - 72.0, 1));
    BT("=====================================");
    BT("");
    soundCount5sec = 0;
    heartBeatCount5sec = 0;
    windowStartTime = now;
  }

  // ANALOG AUDIO SAMPLING
  unsigned long currentMicros = micros();
  if ((long)(currentMicros - nextSampleTime) >= 0) {
    nextSampleTime += SAMPLE_INTERVAL;
    int value = analogRead(ANALOG_PIN);
    uint8_t sample = value >> 4;
    audioBuffer[audioIndex] = sample;
    audioIndex++;

    if (audioIndex >= AUDIO_BLOCK) {
      sendAudioBlock();
      audioIndex = 0;
    }
  }
}
