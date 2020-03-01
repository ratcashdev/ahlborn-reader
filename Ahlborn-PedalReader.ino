//**************************************************************//
//  Name    : Ahlbordn 30-ntoe PedalBoard MIDI interface        //
//  Author  : ratcashdev                                        //
//  Date    : 1 Match, 2020                                     //
//  Version : 1.0                                               //
//  Notes   : Based on the code to read using a CD4021B         //
//          : Shift Register                                    //
//          : https://www.arduino.cc/en/tutorial/ShiftIn        //
//****************************************************************

#include "MIDIUSB.h"
//#include "pitchToNote.h"

//  PINOUT on the 5 pin DIN connector
//       3
//    2     4
//  1         5
//
//
//
//  1- +5V (YELLOW)
//  2- LATCH (PINK -> WHITE)
//  3- GND (RED)
//  4- CLOCK (BROWN/GREEN)
//  5- DATA (BLACK)

//define where your pins are
//on UNO
int clockPin = 7;
int latchPin = 8;
int dataPin = 9;
int midiChannel = 9;


void setup() {
  //start serial
  Serial.begin(9600);
  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
}

const byte pitchC1 = 24;
int notesRead[] = {0,0,0};
 
void loop() {
  
  // scans the pedal's state (in parallel) and stores it in the
  // shift registeres for serial read-out and sets them into serial-read-out mode
  scanPedalsToRegisters();

  // read 29 bits (the actual config of the 2x 4006B shift registers)
  // and drop them (these are the preloaded content of the shift registers, or old content)
  for (byte preamble = 0; preamble < 31; preamble++) {
    readSingleBit(dataPin, clockPin);
  }

  
  //collect each shift register into a byte
  //the register attached to the chip comes in first
//  Serial.println("");
//  Serial.println("-------------------");

  int bitsRead = 0;
  // a 30 note pedalboard contains only 6 notes (C-F) in the last octave
  // read that first
  bitsRead = readPartialOctave(dataPin, clockPin, 6);
  sendNotesToMidi(bitsRead, notesRead[2], 36);
  notesRead[2] = bitsRead;
  //Serial.print(bitsRead, BIN);

  bitsRead = readFullOctave(dataPin, clockPin);
  sendNotesToMidi(bitsRead, notesRead[1], 24);
  notesRead[1] = bitsRead;
  //Serial.print(bitsRead, BIN);

  bitsRead = readFullOctave(dataPin, clockPin);
  sendNotesToMidi(bitsRead, notesRead[0], 12);
  notesRead[0] = bitsRead;
  //Serial.print(bitsRead, BIN);
  
  //white space
  //Serial.println("-------------------");
  //delay so all these print satements can keep up.
//  delay(500);
}

//------------------------------------------------end main loop


int readFullOctave(int myDataPin, int myClockPin) {
  return readPartialOctave(myDataPin, myClockPin, 12);
}


int readPartialOctave(int myDataPin, int myClockPin, byte noteCount) {
  return readMultipleBits(myDataPin, myClockPin, noteCount);
}


////// reads data from the SHIFT registers,
////// SHIFT registers return the MOST significant bit first.
////// the first bit read is the leftmost bit in the returned int
int readMultipleBits(int myDataPin, int myClockPin, int bitCount) {

  int i;
  int temp = 0;
  int myDataIn = 0;

  for (i=bitCount-1; i>=0; i--)
  {
    temp = readSingleBit(myDataPin, myClockPin);
    if (temp) {
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }

  }

  return myDataIn;
}

int readSingleBit(int myDataPin, int myClockPin) {
   // low-to-high transition to prepare the next-bit for read
   digitalWrite(myClockPin, 1);
   delayMicroseconds(2);
   
   digitalWrite(myClockPin, 0);
   delayMicroseconds(2);
   return digitalRead(myDataPin);
}

void scanPedalsToRegisters() {
  //Pulse the latch pin:
  //set it to 1 to collect parallel data, wait
  digitalWrite(latchPin,1);
  delayMicroseconds(20);
  //set it to 0 to transmit data serially  
  digitalWrite(latchPin,0);
}

////// ----------------------------------------getBit
boolean getBit(int notes, int noteMask) {
  boolean bitState;
  bitState = notes & noteMask;
  return bitState;
}

void sendNotesToMidi(int currentState, int lastState, byte octaveShift) {
  if (currentState == lastState) {
    return;
  }

  for (byte noteIndx = 0; noteIndx < 12; noteIndx++) {
    int noteIsOn = getBit(currentState, 1 << noteIndx);
    if (noteIsOn != getBit(lastState, 1 << noteIndx )) {
      byte pitch = pitchC1 + octaveShift + noteIndx;
      if (noteIsOn) {
        midiNoteOn(midiChannel, pitch, 127);
      } else {
        midiNoteOff(midiChannel, pitch, 127);
      }
    }
  }
  MidiUSB.flush();
}

void midiNoteOn(byte channel, byte pitch, byte velocity) {
//  Serial.print("Note on event: ");
//  Serial.println(pitch);
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void midiNoteOff(byte channel, byte pitch, byte velocity) {
//  Serial.print("Note off event: ");
//  Serial.println(pitch);
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
