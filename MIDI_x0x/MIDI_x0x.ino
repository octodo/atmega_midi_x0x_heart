#include <MIDI.h>
#include "AH_MCP4922.h"

#define GATE_PIN 4
#define GATE_LED A5

#define ENV_MOD_CTRL 7
#define ENV_MOD_PIN 3

#define CUTOFF_CTRL 74
#define CUTOFF_PIN 5

#define SAW_CTRL 70
#define SAW_PIN 6
#define SQR_CTRL 71
#define SQR_PIN 9

#define SLIDE_CTRL 65
#define SLIDE_TIME_CTRL 5
#define SLIDE_IN_PIN 13
#define SLIDE_OUT_PIN 12

#define DECAY_CTRL 72
#define DECAY_PIN 11

#define ACCENT_CTRL 11
#define ACCENT_PIN 10


#define ALL_NOTES_OFF 123
#define CTRL_RESET 121

AH_MCP4922 PitchDac(A1,A2,A3,LOW,LOW);
AH_MCP4922 CutoffDac(A1,A2,A3,HIGH,LOW);

int liveNoteCount = 0;
int pitchbendOffset = 0;
int baseNoteFrequency;

MIDI_CREATE_DEFAULT_INSTANCE();

byte selectedChannel;

void handleNoteOn(byte channel, byte pitch, byte velocity)
{  
  liveNoteCount++;
  
  baseNoteFrequency = (pitch - 12) * 42;
  PitchDac.setValue(baseNoteFrequency + pitchbendOffset);
  CutoffDac.setValue(velocity * 32);

  digitalWrite(GATE_PIN, HIGH);
  digitalWrite(GATE_LED, HIGH);
  analogWrite(VELOCITY_PIN, 2 * velocity);
 }


void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  liveNoteCount--;
  
  if (liveNoteCount == 0) {
    digitalWrite(GATE_PIN, LOW);
    digitalWrite(GATE_LED, LOW);
  }
}




void handleControlChange(byte channel, byte number, byte value)
{
  int scaledValue = value << 3;
  
  switch (number) {
    case CUTOFF_CTRL:
      analogWrite(CUTOFF_PIN, scaledValue);
      break;
  }
}


void handlePitchBend(byte channel, int bend)
{
  pitchbendOffset = bend >> 4;

  PitchDac.setValue(baseNoteFrequency + pitchbendOffset);
}


// -----------------------------------------------------------------------------

void setup()
{
    int channelSpan = 1024 / 16;
    int channelInput = analogRead(0);
    selectedChannel = channelInput / channelSpan;
    
    pinMode(GATE_PIN, OUTPUT);
    digitalWrite(GATE_PIN, LOW);
    pinMode(GATE_LED, OUTPUT);
    digitalWrite(GATE_LED, LOW);

    delay(1000);

    playScale(selectedChannel);

    // calibrate 8V
    baseNoteFrequency = (108 - 12) * 42;
    PitchDac.setValue(baseNoteFrequency);
    // calibrate full cutoff
    CutoffDac.setValue(32 * 127);

    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleControlChange);
    
    MIDI.begin(selectedChannel);
}


void playScale(int channel) {

  int note = 60;

  for (int i=0; i<channel; i++) {

      handleNoteOn(channel, note, 100);
      delay(100);
      handleNoteOff(channel, note, 100);
      delay(100);
      note++;
  }

}


void loop()
{
    MIDI.read();
}

