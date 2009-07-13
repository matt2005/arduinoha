// ToDo's:
// * Make the prescaler smaller to try to get more accurate timings
// * Protocols: http://bertrik.sikken.nl/433mhz/index.html (Haehnel remote shutter trigger HW 433 N80, Cactus v2 remote flash)
// * Protocols: http://jon.oxer.com.au/blog/id/339
// * Protocols: http://www.practicalarduino.com/projects/easy/appliance-remote-control
// * Protocols: http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf 
// * Protocols: http://www.tinaja.com/glib/muse151.pdf
// * Protocols: http://www.navitron.org.uk/forum/index.php?topic=1830.15

// Base and utility includes
#include <ProtocolBase.h>
#include <ProtocolCommand.h>
#include <DynamicArrayHelper.h>

// Protocol includes
#include <KlikAanKlikUitProtocol.h>
#include <RanexProtocol.h>
#include <ElroProtocol.h>
#include <FranElecProtocol.h>
#include <LaCrosseProtocol.h>
#include <LaCrosseProtocol2.h>
#include <SkytronicProtocol.h>
#include <Siemens5WK4Protocol.h>
#include <McVoiceProtocol.h>
#include <HouseLinkProtocol.h>
#include <X10Protocol.h>
#include <ELVProtocol.h>

#define ShowErrors
#define ShowReceivedCommands
//#define ShowReceivedPulses // Beware! Enabling this debug-output may distort the timings!
#define ShowReceivedBitstream
//#define ShowSendBitstream
//#define Automate
//#define ShowSendPulses // Beware! Enabling this debug-output may distort the timings!

// If the receiver-module has a RSSI-output-pin, this pin can be wired-up to only process strongly received signals.
// If the receiver-module does not have a RSSI-output-pin, the data-output-pin can be used as a trigger on every signal (even noise). 
unsigned int RSSIPIN    = 10; // The pin number of the RSSI signal (Sanguino pins: 10, 11, 2)
unsigned int RSSIIRQNR  = 0;  // The irq number of the RSSI pin (Saguino pin=irqnr: 10=0 , 11=1, 2=2)

// Struct which is used to store received pulses in a buffer
struct pulse 
{
  byte state;
  unsigned int duration;
};

unsigned long int currenttime = 0;
unsigned long int syncedtime = 0;
unsigned long int syncedcurrenttime = 0;

enum SendReceiveStateEnum
{
  uninitialized,
  waitingforrssitrigger,
  listeningforpulses,
  stopped,
  sendingheader,
  sendingbits,
  sendingterminator,
  sendingfinished
};


// The datapin defines the pin which is wired-up to the Data-output-pin of the receiver-module.
unsigned int DATAPIN = 14;      // The pin number of the data signal

#define TXPIN 3       // The pin number of the transmission signal

unsigned int STATUSLEDPIN = 0; // The pin number of the status-led (Sanguino: 0)

#define UPDATEFREQUENCY_SCHEDULEDCOMMANDS 100l
#define UPDATEFREQUENCY_QUEUEDCOMMANDS   100l

// The number of milliseconds of silence after which sending of a queued command can commence. 
// This duration should be longer than the silence in pulse-bursts (for example the duration terminator-silence of received commands).
#define SEND_SILENCEDURATION 400l

#define PRESCALER256

// This defines the number of received pulses which can be stored in the Circular buffer. 
// This buffer is used to allow pulses to be received while previous pulses are still being processed.
// The Arduino must catch up or an overflow will occur and pulses will be lost.
#define ReceivedPulsesCircularBufferSize 200

#define SerialBaudrate 115200

// ---------------------------------------------------------------------------------------------------------------------------------------

// The timings of the pulses received from the receiver-module will be stored in this Circul/Ring-buffer. The reason for this is that these pulses can be "offline" processed.
pulse receivedpulsesCircularBuffer[ReceivedPulsesCircularBufferSize];

// The index number of the first pulse which will be read from the buffer.
volatile unsigned short receivedpulsesCircularBuffer_readpos = 0; 

// The index number of the position which will be used to store the next received pulse.
volatile unsigned short receivedpulsesCircularBuffer_writepos = 0; 


volatile unsigned int prevTime; // ICR1 (timetick) value  of the start of the previous Pulse

ProtocolBase *send_protocol = 0; // The instance of the protocol which is being sent
byte * send_bitbuffer = 0; // The buffer for the bitstream which is being sent
volatile byte send_bitbufferlength = 0; // The length of the bitbuffer in bytes
volatile byte send_bitpos = 0; // The position of the bit which will be sent next
unsigned int * send_pulsebuffer = 0;
byte send_pulsebufferlength = 0;
volatile int send_pulsepos = 0;
volatile byte send_repeats = 0; // The number of times the bitstream still needs to be repeated
volatile byte sendpulsestate = LOW;

byte statusledstate = LOW;

unsigned long last_check_scheduledcommands = 0;
unsigned long last_check_queuedcommands = 0;

protocolcommand* ScheduledCommands = 0; // An array of scheduled commands
byte ScheduledCommandsCount = 0; // The number of scheduled commands in the array

protocolcommand* QueuedCommands = 0; // An array of queued commands
byte QueuedCommandsCount = 0; // The number of queued commands in the array

volatile SendReceiveStateEnum sendreceivestate = uninitialized;

short showidx = 0;

unsigned long lastrssitrigger = 0;

DynamicArrayHelper dynamicarrayhelper;

// --------------------------------------------------------------------------------------------------------------------------------------------------
// Debug Output functions
// --------------------------------------------------------------------------------------------------------------------------------------------------

// This method will print the duration of a pulse to the Serial
// It will print data formatted to be easy readable for debug purposes
void ShowPulse(bool state, unsigned int duration)
{
    if (state) Serial.print("["); else Serial.print(" ");
    
    if (duration < 10) 
      Serial.print("   "); 
    else 
      if (duration < 100) Serial.print("  "); 
    else 
      if (duration < 1000) Serial.print(" ");
      
    Serial.print(duration,DEC); 
    
    if (state) 
      Serial.print("]"); 
    else
      Serial.print(" ");
    
    if (showidx++>=21) 
    {
      PrintNewLine();
      showidx = 0;
    }
}

void PrintNewLine()
{
    Serial.println("");
}

void PrintProtocolId(ProtocolBase * protocol)
{
    Serial.print(protocol->GetId()); 
    Serial.print(": ");
}

void PrintTime(unsigned long pt)
{
    unsigned long time = pt - syncedcurrenttime;
    unsigned long t = syncedtime + time;
    unsigned long days = t / 86400000; t = t % 86400000;
    unsigned long hours = t / 3600000; t = t % 3600000;
    unsigned long mins = t / 60000 ; t = t % 60000;
    unsigned long secs = t / 1000;
    Serial.print("["); 
    if (days<10) Serial.print("0");    
    Serial.print(days, DEC);
    Serial.print("-");
    if (hours<10) Serial.print("0");    
    Serial.print(hours,DEC);
    Serial.print(":");
    if (mins<10) Serial.print("0");
    Serial.print(mins,DEC);
    Serial.print(":");
    if (secs<10) Serial.print("0");
    Serial.print(secs,DEC);
    Serial.print("]");
}

void PrintBit(bool value)
{
  Serial.print(value?"1":"0");
}

void PrintFloat(float value)
{
  int deg=(int)fabs(value); Serial.print(deg,DEC);Serial.print(".");int frac=(int)(fabs(value*10.0f)) % 10; Serial.print(frac,DEC);
}

void ShowBitstream(ProtocolBase * protocol, byte * bitbuffer, byte length)
{
  #ifdef ShowReceivedBitstream
    PrintTime(currenttime);
    PrintProtocolId(protocol);
    for (int bitpos=0;bitpos<length;bitpos++)
    {
      int bytepos = bitpos >> 3;
      PrintBit( (bitbuffer[bytepos] & (128 >> (bitpos % 8))) != 0 ) ;
    }
    PrintNewLine();
  #endif
}


// --------------------------------------------------------------------------------------------------------------------------------------------------
// ISR
// --------------------------------------------------------------------------------------------------------------------------------------------------

// This function will store a received pulse in the Circular buffer
void StoreReceivedPulseInBuffer(unsigned int duration, byte state)
{
    // Store pulse in receivedpulsesCircularBuffer
    pulse * p = &(receivedpulsesCircularBuffer[receivedpulsesCircularBuffer_writepos]);
    p->state = state;
    p->duration = duration;
   
    // When at the end of the buffer start from beginning of buffer
    if (++receivedpulsesCircularBuffer_writepos >= ReceivedPulsesCircularBufferSize) 
    {
      receivedpulsesCircularBuffer_writepos = 0;
    }

    #ifdef ShowErrors    
      // Check overflow
      if (receivedpulsesCircularBuffer_writepos==receivedpulsesCircularBuffer_readpos)
      { // Overflow circular buffer
        Serial.print("Overflow");
      }
    #endif
}

// This Interrupt Service Routine will be called upon each change of data on DATAPIN when listeningforpulses.
ISR(TIMER1_CAPT_vect)
{ 
  // Store the value of the timer
  unsigned int newTime = TCNT1 ; //ICR1; 

  // We are only interested in pulses when "listeningforpulses"
  if (sendreceivestate==listeningforpulses) 
  {
    byte curstate ;

    // Were we waiting for a "falling edge interrupt trigger?
    if ( (TCCR1B & (1<<ICES1)) == 0)
    { // ISR was triggerd by falling edge
        // set the ICES bit, so next INT is on rising edge
        TCCR1B |= (1<<ICES1);
        curstate = LOW;
    }
    else
    { // ISR was triggered by rising edge
        //clear the ICES1 bit so next INT is on falling edge
        TCCR1B &= (~(1<<ICES1));      
        curstate = HIGH;
    } 

    // Calculate the duration of the pulse
    unsigned int time = (newTime - prevTime);
    
    // Did the timer overflow?
    if (newTime<prevTime)
    {
      // Calculate the duration of the pulse taking into account the overflowing of the timer
      time =  ((!prevTime) + 1) + newTime;    
    }

    // Store the duration of the received pulse    
    StoreReceivedPulseInBuffer(time, !curstate);
  } ;
  
  prevTime = newTime;  
}

// This Interrupt Service Routine will be called upon an timer-overflow. This will happen every few seconds when receiving. It will also happen for each sent pulse after the duration.
ISR(TIMER1_OVF_vect) 
{
  switch (sendreceivestate)
  {
    case listeningforpulses :
      // Disable timer-overflow interrupt
      TIMSK1 = 0 ;
      
      // Add some pulses to flush decoders
      StoreReceivedPulseInBuffer(65535-prevTime,HIGH);
      StoreReceivedPulseInBuffer(65535-prevTime,LOW);

      // Timer has overflowed, start waiting again
      sendreceivestate=waitingforrssitrigger;
      break;
      
    case sendingheader :
    case sendingbits :
    case sendingterminator :
      // Flip output-signal bit (So: High->Low Low->High)
      sendpulsestate = !sendpulsestate;
      digitalWrite(TXPIN,sendpulsestate);
  
      // Shift to the next pulse that will be send
      send_pulsepos++;
      
      // Is there still a pulse left to be send in this buffer?
      if ( send_pulsepos >= send_pulsebufferlength)
      {
        // Get next pulses in the buffer
        GetNextSendPulses();
        
        // There is nothing more to be sent, we are finished
        if (sendreceivestate==sendingfinished)
        {
          // Make sure the sended burst is ended with a low signal
          digitalWrite(TXPIN,LOW);

          //Disable timer interrupt; Stop edge timer, stop overflow interrupt
          TIMSK1=0; 
          
          sendreceivestate = waitingforrssitrigger;
          
          attachInterrupt(RSSIIRQNR, rssiPinTriggered , RISING);
          
          return; // Leave this function
        }
      }

      // Fetch the time this processing has taken.
      unsigned int lag = TCNT1;

      // Calculate the duration of the next pulse taking into account the lag
      long duration = (65536 - send_pulsebuffer[ send_pulsepos ] + lag );
      
      // Does the calculated destination timer value overflow? (Lab>=pulseduration)
      if (duration>65535) 
      { // Yes
        // Hmmm. We can't travell through time, but we can shorten our delay as much as possible.
        duration = 65535;
        #ifdef ShowErrors      
          Serial.print("Lagged!");
        #endif      
      }
      // Set this register so the next Timer-overflow will happen at desired duration
      TCNT1 = duration ;    

      #ifdef ShowSendPulses
        ShowSendPulse(send_pulsebuffer[ send_pulsepos ]);
      #endif
      
      lastrssitrigger = currenttime;
      break;
  }
}



// --------------------------------------------------------------------------------------------------------------------------------------------------

// This function will be triggered by a high RSSI signal
void rssiPinTriggered(void)
{
  if (sendreceivestate==waitingforrssitrigger)
  {
    sendreceivestate = listeningforpulses;
    prevTime = 0;//ICR1;
     // reset the counter as close as possible when the rssi pin is triggered (to acheive accuracy)
    TCNT1 = 0;
    TIMSK1=1<<ICIE1 /* enable capture interrupt */ | 1<<TOIE1 /* enable timer overflow interrupt */; 
    TIFR1 |= 1 << TOV1; //clear the "pending overflow interupt" flag
    
    //clear the ICES1 bit so next INT is on falling edge
    TCCR1B&=(~(1<<ICES1));      

    #ifdef ShowReceivedPulses
      showidx=0;Serial.println("RSSI");
    #endif
  }
  lastrssitrigger = currenttime;  
}


char * Ranex433ID = "Ranex433";
char * Elro433ID = "Elro433";
char * Kaku433ID = "Kaku433";
char * FranElec433ID = "FE433";
char * Skytronic433ID = "ST433";
char * LaCrosse433ID = "LC433";
char * LaCrosse2433ID = "LC2433";
char * Siemens5WK4433ID = "OCK433";
char * McVoice433ID = "MCV433";
char * HouseLink433ID = "HL433";
char * X10433ID = "X10433";
char * Unknown1433ID = "UK1433";
char * ELV868ID = "ELV868" ;

// Calculate the duration of one timertick (CPUSpeed / Prescalerfactor)
#ifdef PRESCALER256
   double TIMERFREQUENCY =  (F_CPU / 256.0f);
#else
   double TIMERFREQUENCY =  (F_CPU / 64.0f);
#endif

// Initialize the various protocol decoders
RanexProtocol ranex = RanexProtocol( Ranex433ID, TIMERFREQUENCY , ShowBitstream, CommandReceived);
ElroProtocol elro = ElroProtocol( Elro433ID, TIMERFREQUENCY , ShowBitstream, ElroCommandReceived);
KlikAanKlikUitProtocol kaku = KlikAanKlikUitProtocol( Kaku433ID, TIMERFREQUENCY , ShowBitstream, CommandReceived);
FranElecProtocol franelec = FranElecProtocol( FranElec433ID, TIMERFREQUENCY , ShowBitstream, FranElecTrigger, FranElecTrigger);
SkytronicProtocol skytronic = SkytronicProtocol( Skytronic433ID, TIMERFREQUENCY , ShowBitstream, CommandReceived, AllDevicesCommandReceived);
LaCrosseProtocol lacrosse = LaCrosseProtocol( LaCrosse433ID, TIMERFREQUENCY , ShowBitstream, TemperatureReceived, HygroReceived, RainReceived);
LaCrosseProtocol2 lacrosse2 = LaCrosseProtocol2( LaCrosse2433ID, TIMERFREQUENCY , ShowBitstream, RainReceived, WindReceived);
Siemens5WK4Protocol siemens5wk4 = Siemens5WK4Protocol( Siemens5WK4433ID , TIMERFREQUENCY , ShowBitstream, LockCommandReceived , TwoButtonsPressedReceived);
McVoiceProtocol mcvoice = McVoiceProtocol( McVoice433ID, TIMERFREQUENCY , ShowBitstream , DeviceTrippedReceived , BatteryEmptyReceived );
HouseLinkProtocol houselink = HouseLinkProtocol( HouseLink433ID, TIMERFREQUENCY , ShowBitstream , HomeLinkDeviceTrippedReceived  );
X10Protocol x10 = X10Protocol( X10433ID, TIMERFREQUENCY , ShowBitstream ,  X10CommandReceived );
ELVProtocol elv = ELVProtocol( ELV868ID, TIMERFREQUENCY , ShowBitstream , TemperatureReceived, HygroReceived );

    
void TemperatureReceived(ProtocolBase * protocol, byte device, float temperature)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);
    PrintProtocolId(protocol);
    Serial.print("T ");
    Serial.print(device,DEC);
    Serial.print(" ");
    PrintFloat(temperature);
    PrintNewLine();
  #endif
}

void HygroReceived(ProtocolBase * protocol, byte device, float hygro)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print("H ");
    Serial.print(device,DEC);
    Serial.print(" ");
    PrintFloat(hygro);
    PrintNewLine();
  #endif
}

void RainReceived(ProtocolBase * protocol, byte device, int rain)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print("R ");
    Serial.print(device,DEC);
    Serial.print(" ");
    Serial.print(rain, DEC);
    PrintNewLine();
  #endif
}

void WindReceived(ProtocolBase * protocol, byte device, int wind)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print("W ");
    Serial.print(device,DEC);
    Serial.print(" ");
    Serial.print(wind, DEC);
    PrintNewLine();
  #endif
}



void CommandReceived(ProtocolBase * protocol, byte device, bool lightoncommand)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(device,DEC);
    Serial.print(" ");
    if (lightoncommand) Serial.println("On"); else Serial.println("Off");
  #endif
}

void ElroCommandReceived(ProtocolBase * protocol, byte group, byte device, bool lightoncommand)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(group,DEC);Serial.print(" ");
    Serial.print(device,DEC);
    Serial.print(" ");
    if (lightoncommand) Serial.println("On"); else Serial.println("Off");
  #endif
}


void LockCommandReceived(ProtocolBase * protocol, bool lockcommand)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    if (lockcommand) Serial.println("Lock"); else Serial.println("Unlock");
  #endif

  #ifdef Automate  
  // If a button from a car key is pressed, turn on the front-porch-lighting for a minute
  byte * bitbuffer = 0;
  byte bitbufferlength = 0;
  skytronic.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
  int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &skytronic , bitbuffer,  bitbufferlength);
  if (ii==-1)
  {       
    byte * bitbuffer2 = 0;
    byte bitbufferlength2 = 0;
    skytronic.EncodeDeviceCommand( 0 , true , bitbuffer2 , bitbufferlength2 );
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer2 , bitbufferlength2 , true, true);
    ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
  } else
  {
    ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
  }
  #endif
}

void DeviceTrippedReceived(ProtocolBase * protocol,byte device)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(device,DEC);
    Serial.println("Tripped"); 
  #endif
}

void HomeLinkDeviceTrippedReceived(ProtocolBase * protocol,byte group, byte device, bool state)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(group,DEC);Serial.print(" ");
    Serial.print(device,DEC);
    if (state) Serial.println("Tripped"); else Serial.println("Idle");
  #endif
}

void X10CommandReceived(ProtocolBase * protocol, byte group, byte device, bool state)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(group,DEC);Serial.print(" ");
    Serial.print(device,DEC);Serial.print(" ");
    if (state) Serial.println("On"); else Serial.println("Off");
  #endif
  
  #ifdef Automate
  if (device==9)
  {
    // turn all lights in the living room on
    for (int dev=0;dev<5;dev++)
    {    
      byte * bitbuffer;
      byte bitbufferlength ;
      ranex.EncodeDeviceCommand( dev , true , bitbuffer , bitbufferlength );
      if (bitbuffer==0) return;
    
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &ranex , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {
         ScheduleCommand( 0.0f  , &ranex , bitbuffer , bitbufferlength , true, true);
      } 
    }  
  }
  
  if (device==0)
  {
    // turn all lights in the living room off
    for (int dev=0;dev<5;dev++)
    {    
      byte * bitbuffer;
      byte bitbufferlength ;
      ranex.EncodeDeviceCommand( dev , false , bitbuffer , bitbufferlength );
      if (bitbuffer==0) return;
    
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &ranex , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {
         ScheduleCommand( 0.0f  , &ranex , bitbuffer , bitbufferlength , true, true);
      } 
    }  
  }
  
  if (device==1)
  {
    // turn light in the hallway on
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 1 , true , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

  if (device==2)
  {
    // turn light in the hallway off
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 1 , false , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

  if (device==3)
  {
    // turn light front-porch on
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 0 , true , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

  if (device==4)
  {
    // turn light front-porch off
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

if (device==5)
  {
    // turn light hall 1st floor on
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 2 , true , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

  if (device==6)
  {
    // turn light hall 1st floor off
    byte * bitbuffer;
    byte bitbufferlength ;
    skytronic.EncodeDeviceCommand( 2 , false , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer , bitbufferlength , true, true);
  }

if (device==7)
  {
    // turn light hall 2st floor on
    byte * bitbuffer;
    byte bitbufferlength ;
    kaku.EncodeDeviceCommand( 0 , true , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &kaku , bitbuffer , bitbufferlength , true, true);
  }

  if (device==8)
  {
    // turn light hall 2st floor off
    byte * bitbuffer;
    byte bitbufferlength ;
    kaku.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    ScheduleCommand( 0.0f  , &kaku , bitbuffer , bitbufferlength , true, true);
  }


  #endif
}


void BatteryEmptyReceived(ProtocolBase * protocol, byte device)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print(device,DEC);
    Serial.println("Battery");
  #endif

}

void TwoButtonsPressedReceived(ProtocolBase * protocol)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.println("Alarm");
  #endif
  
  #ifdef Automate  
  // If the two buttons are pressed, turn on the front-porch-lighting for a minute
  byte * bitbuffer = 0;
  byte bitbufferlength = 0;
  skytronic.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
  int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &skytronic , bitbuffer,  bitbufferlength);
  if (ii==-1)
  {       
    byte * bitbuffer2 = 0;
    byte bitbufferlength2 = 0;
    skytronic.EncodeDeviceCommand( 0 , true , bitbuffer2 , bitbufferlength2 );
    ScheduleCommand( 0.0f  , &skytronic , bitbuffer2 , bitbufferlength2 , true, true);
    ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
  } else
  {
    ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
  }
  
  // turn all lights in the living room on
  for (int dev=0;dev<5;dev++)
  {    
    byte * bitbuffer;
    byte bitbufferlength ;
    ranex.EncodeDeviceCommand( dev , true , bitbuffer , bitbufferlength );
    if (bitbuffer==0) return;
    
    int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &ranex , bitbuffer,  bitbufferlength);
    if (ii==-1)
    {
       ScheduleCommand( 0.0f  , &ranex , bitbuffer , bitbufferlength , true, true);
    } 
  }
  #endif

  
  
}

void AllDevicesCommandReceived(ProtocolBase * protocol, bool lightoncommand)
{
  #ifdef ShowReceivedCommands
    PrintTime(currenttime);  
    PrintProtocolId(protocol);
    Serial.print("All ");
    if (lightoncommand) Serial.println("On"); else Serial.println("Off");
  #endif
}

void FranElecTrigger(ProtocolBase * protocol, byte device)
{
    #ifdef ShowReceivedCommands
    PrintTime(currenttime);    
    PrintProtocolId(protocol);
    Serial.println(device,DEC);
    #endif

    #ifdef Automate
    // Als beweging gemeld in de gang begane grond, licht aanzetten voor 1 minuut
    if (device==47)
    {
      byte * bitbuffer = 0;
      byte bitbufferlength = 0;
      skytronic.EncodeDeviceCommand( 1 , false , bitbuffer , bitbufferlength );
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &skytronic , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {       
       byte * bitbuffer2 = 0;
       byte bitbufferlength2 = 0;
       skytronic.EncodeDeviceCommand( 1 , true , bitbuffer2 , bitbufferlength2 );
       ScheduleCommand( 0.0f  , &skytronic , bitbuffer2 , bitbufferlength2 , true, true);
       ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
      } else
      {
        // Voordat het licht weer uitgeschakeld is, wordt weer beweging geconstateerd. Hou dan het licht langer aan.
        ScheduleCommand( 120.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
      }
    }
    
    // Als beweging gemeld in gang 1e verdieping, verlichting gang 1 minuut aan
    if (device==35)
    {
      byte * bitbuffer = 0;
      byte bitbufferlength = 0;
      skytronic.EncodeDeviceCommand( 2 , false , bitbuffer , bitbufferlength );
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &skytronic , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {       
       byte * bitbuffer2 = 0;
       byte bitbufferlength2 = 0;
       skytronic.EncodeDeviceCommand( 2 , true , bitbuffer2 , bitbufferlength2 );
       ScheduleCommand( 0.0f , &skytronic , bitbuffer2 , bitbufferlength2 , true, true);
       ScheduleCommand( 60.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
      } else
      {
        // Voordat het licht weer uitgeschakeld is, wordt weer beweging geconstateerd. Hou dan het licht langer aan.        
        ScheduleCommand( 120.0f , &skytronic , bitbuffer , bitbufferlength , true, true);
      }
    }

    // Als beweging op 2e verdieping gang, aanzetten verlichting voor 1 minuut
    if (device==1)
    {
      byte * bitbuffer = 0;
      byte bitbufferlength = 0;
      kaku.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &kaku , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {       
       byte * bitbuffer2 = 0;
       byte bitbufferlength2 = 0;
       kaku.EncodeDeviceCommand( 0 , true , bitbuffer2 , bitbufferlength2 );
       ScheduleCommand( 0.0f , &kaku , bitbuffer2 , bitbufferlength2 , true, true);
       ScheduleCommand( 60.0f  , &kaku , bitbuffer , bitbufferlength , true, true);
      } else
      {
        // Voordat het licht weer uitgeschakeld is, wordt weer beweging geconstateerd. Hou dan het licht langer aan.
        ScheduleCommand( 120.0f , &kaku , bitbuffer , bitbufferlength , true, true);
      }
    }
    
    // Als beweging in het CV hok, 2e verdieping, aanzetten gang verlichting 2e verdieping
    if (device==0)
    {
      byte * bitbuffer = 0;
      byte bitbufferlength = 0;
      kaku.EncodeDeviceCommand( 0 , false , bitbuffer , bitbufferlength );
      int ii = IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, &kaku , bitbuffer,  bitbufferlength);
      if (ii==-1)
      {       
       byte * bitbuffer2 = 0;
       byte bitbufferlength2 = 0;
       kaku.EncodeDeviceCommand( 0 , true , bitbuffer2 , bitbufferlength2 );
       ScheduleCommand( 0.0f , &kaku , bitbuffer2 , bitbufferlength2 , true, true);
       ScheduleCommand( 120.0f  , &kaku , bitbuffer , bitbufferlength , true, true);
      } else
      {
        // Voordat het licht weer uitgeschakeld is, wordt weer beweging geconstateerd. Hou dan het licht langer aan.
        ScheduleCommand( 120.0f , &kaku , bitbuffer , bitbufferlength , true, true);
      }
    }
    #endif

}

// This method updates the various variables for sending the next set of data.
// It uses the "sendreceivestate" to determine the previous thing we were doing.
void GetNextSendPulses()
{
  // Was the sent pulsebuffer allocated?
  if (send_pulsebuffer!=0) 
  { // Yes, Free its memory
    free(send_pulsebuffer);
    send_pulsebuffer = 0;
    send_pulsebufferlength = 0;
  }
  
  // Was the sent pulsebuffer the terminator? 
  if (sendreceivestate == sendingterminator)
  { // Yes
    // Did we complete the number of repeats?
    if (send_repeats==0)
    { // Yes
      sendreceivestate = sendingfinished ;
      
      // We can now also release the bitstream-buffer of the command
      free(send_bitbuffer);
      send_bitbuffer = 0;
      
      // Check if a next command is queued for sending
      sendnextcommand();
    } else
    { // No, we still need to send another repitition of the bitstream
      send_repeats -- ;
      
      // Reset the sendreceivestate to be able to retrieve the first PulseBuffer we need to send
      sendreceivestate = stopped;

      // Recurse this method and fetch the pulses (header)
      GetNextSendPulses();
    }
    return; // Leave this function
  }
  
  if (sendreceivestate == stopped)
  {
    send_pulsepos = 0;
    send_protocol->EncodeHeader( send_pulsebuffer , send_pulsebufferlength );
    sendreceivestate = sendingheader;
    
    // If the returned buffer is null, this protocol doesn't have a header. 
    if (send_pulsebuffer==0)
    {
      send_bitpos = 0;
      
      // Recurse this method and fetch the pulses of the next part (body bitstream)
      GetNextSendPulses();
    }
    
    return; // Leave this function
  }

  // We were "sendingheader" we will now continue with the body bitstream
  if (sendreceivestate == sendingheader)
  {  
      sendreceivestate = sendingbits;      
      send_bitpos = 0;
      
      // Get the bit which will be send next
      bool bitvalue = send_protocol->GetBit( send_bitbuffer , send_bitbufferlength , send_bitpos );

      send_pulsepos = 0;
      // Get a buffer for the bit which will be send next
      send_protocol->EncodeBit( send_pulsebuffer , send_pulsebufferlength , bitvalue );

      // If the returned buffer is null, probably a memory error or incomplete protocol-code
      if (send_pulsebuffer==0) 
      {
        GetNextSendPulses();
      }
      return;
  } 
  
  if (sendreceivestate == sendingbits)
  {
      send_bitpos++;
      // Have we send all the databits?
      if (send_bitpos >= send_protocol->GetBitstreamLength())
      { // Yes
        sendreceivestate = sendingterminator;
        send_pulsepos = 0;
        send_protocol->EncodeTerminator( send_pulsebuffer , send_pulsebufferlength );

        // If the returned buffer is null, this protocol doesn't have a terminator.
        if (send_pulsebuffer==0)
        {
          GetNextSendPulses();
        }
        return; // Leave the function
      } 
      // Get the bit which will be send next
      bool bitvalue = send_protocol->GetBit( send_bitbuffer , send_bitbufferlength , send_bitpos );

      send_pulsepos = 0;
      send_protocol->EncodeBit( send_pulsebuffer , send_pulsebufferlength , bitvalue );
      if (send_pulsebuffer==0)
      {
        GetNextSendPulses();
      }
      return; // Leave the function
    } 
}

// Check wether the command is already in a collection of commands. 
// Returns the index within the collection if found. Otherwise -1 if not found in collection.
int IsCommandInCollection( protocolcommand* commands, byte commandscount , ProtocolBase *protocol, byte * bitbuffer, byte bitbufferlength)
{
  // Loop through all commands in the collection
  for (int idx=0; idx < commandscount; idx++)
  {
    protocolcommand* c = &commands[idx];
    
    // Is the protocol the same?
    if (c->protocol == protocol)
    {
      bool equalcommand = true;
      
      // Loop through all bits
      for (int byteidx=0;byteidx<bitbufferlength;byteidx++)
      {
        // Do the bits differ?
        if (bitbuffer[byteidx]!=(*c).bitbuffer[byteidx])
        { // Yes, so NOT equal
          equalcommand = false;
          break;
        }
      }
      if (equalcommand) return idx;
    }
  }
  return -1;
}

void ScheduleCommand( float seconds , ProtocolBase *protocol, byte * bitbuffer, byte bitbufferlength , bool overwritewhenlater, bool overwritewhenearlier)
{
  int idx=IsCommandInCollection( QueuedCommands, QueuedCommandsCount, protocol, bitbuffer,  bitbufferlength);
  if (idx!=-1) 
  {
    PrintTime(currenttime);
    Serial.println(" Command not scheduled. Already queued for send.");
    return;
  }
  
  idx=IsCommandInCollection( ScheduledCommands, ScheduledCommandsCount, protocol, bitbuffer,  bitbufferlength);

  unsigned long scheduledtime = currenttime ;
  scheduledtime += (seconds * 1000l) ;
  
  // Is this Command aleady scheduled
  if (idx!=-1)
  { // Yes    
    PrintTime(currenttime);
    Serial.print("Already scheduled. ");
    protocolcommand* sc = &(ScheduledCommands[idx]);
    if ( overwritewhenlater && (scheduledtime > sc->scheduledtime)) 
    {
      Serial.print("Postponed to "); PrintTime(scheduledtime);Serial.println("");
      sc->scheduledtime = scheduledtime;
    } else if (overwritewhenearlier && (scheduledtime < sc->scheduledtime))
    {
      Serial.print("Advanced to "); PrintTime(scheduledtime);Serial.println("");
      sc->scheduledtime = scheduledtime;
    } 
    
    free(bitbuffer);
    return;
  }

  protocolcommand * c = (protocolcommand *) malloc(sizeof(protocolcommand));
  if (c==0) return;
  c->protocol = protocol;
  c->scheduledtime = scheduledtime;
  c->bitbuffer = bitbuffer;
  c->bitbufferlength = bitbufferlength;
  dynamicarrayhelper.AddToArray((void *&)ScheduledCommands, c , ScheduledCommandsCount , sizeof(protocolcommand) );
  free(c);
  
  PrintTime(currenttime);
  Serial.println("Item scheduled");
}


void loop_scheduledcommands()
{
    if (ScheduledCommandsCount>0)
    {
      digitalWrite(STATUSLEDPIN, !digitalRead(STATUSLEDPIN));
    }
    for (int idx=ScheduledCommandsCount-1; idx>=0; idx--)
    {
      if (currenttime >= ScheduledCommands[idx].scheduledtime )
      {
        PrintTime(currenttime);
        Serial.println(" Item queued for sending");

        // Queue command for sending
        dynamicarrayhelper.AddToArray( (void *&)QueuedCommands , &ScheduledCommands[idx] , QueuedCommandsCount , sizeof(protocolcommand) );

        // Remove ScheduledCommand
        dynamicarrayhelper.RemoveFromArray( (void *&)ScheduledCommands , idx , ScheduledCommandsCount , sizeof(protocolcommand) );
      } 
    }
}

void sendnextcommand()
{
  if (QueuedCommandsCount>0)
  {
      sendreceivestate = stopped;
      protocolcommand pc = QueuedCommands[0];      
      send_protocol = pc.protocol;
      send_bitbuffer = pc.bitbuffer;
      send_bitbufferlength = pc.bitbufferlength;
      send_repeats = send_protocol->GetSendRepeats();

      dynamicarrayhelper.RemoveFromArray( (void *&) QueuedCommands , 0 , QueuedCommandsCount, sizeof(protocolcommand) );
    
      #ifdef ShowSendBitstream
        PrintTime(currenttime);
        Serial.print (send_protocol->GetId());
        Serial.print(" Sending: ");
        for (int bitpos=0;bitpos<send_protocol->GetBitstreamLength();bitpos++)
        {
          PrintBit((send_bitbuffer[ bitpos >> 3 ] & (128 >> (bitpos % 8))) != 0) ;
        }
        PrintNewLine();        
      #endif
      GetNextSendPulses();
  }
}

void loop_queuedcommands()
{
  if (QueuedCommandsCount>0)
  {
    // Calculate the number of milliseconds after the last RSSI trigger
    unsigned long delta = currenttime - lastrssitrigger;

    // Are all receivedpulses processed and
    // are we waiting for an rssi-trigger or listening for pulses and the duration has passed?
    if ( receivedpulsesCircularBuffer_readpos == receivedpulsesCircularBuffer_writepos && 
          ((sendreceivestate == waitingforrssitrigger) || 
          (sendreceivestate==listeningforpulses && delta > SEND_SILENCEDURATION)) )
    { // Yes
      sendreceivestate = stopped;
      
      #ifdef RSSIIRQNR
        detachInterrupt(RSSIIRQNR);      
      #endif
      
      TIMSK1=0; //Disable timer interrupt; Stop edge timer, stop overflow interrupt

      sendnextcommand();

      // Did we succesfully get a buffer with pulses?
      if (send_pulsebuffer!=0)
      { // Yes!
        unsigned int duration = send_pulsebuffer[ send_pulsepos ]; 

        sendpulsestate = HIGH;    
        
        #ifdef ShowSendPulses
            ShowPulse(sendpulsestate, duration);
        #endif
        
        TCNT1 = ((unsigned int) 65536)- duration;        
        digitalWrite(TXPIN, sendpulsestate);
        TIMSK1= 1<<TOIE1 /* enable timer overflow interrupt */;
      } else
      { // No
        sendreceivestate = waitingforrssitrigger;
        attachInterrupt(RSSIIRQNR, rssiPinTriggered , RISING);
      }
    }
  }
}

void loop_receive()
{  
  // Is there still a pulse in the receivedpulses Circular buffer?
  while (receivedpulsesCircularBuffer_readpos != receivedpulsesCircularBuffer_writepos)
  { // yes
    unsigned int duration = receivedpulsesCircularBuffer[receivedpulsesCircularBuffer_readpos].duration;
    byte state = receivedpulsesCircularBuffer[receivedpulsesCircularBuffer_readpos].state;
    
    if (++receivedpulsesCircularBuffer_readpos >= ReceivedPulsesCircularBufferSize) 
    {
      receivedpulsesCircularBuffer_readpos = 0;
    }

#ifdef ShowReceivedPulses    
    ShowPulse(state , duration);
#endif

    // Feed the signal to all the decoders
//    franelec.Decode( state , duration );
//    ranex.Decode( state , duration );
//    elro.Decode( state , duration );
//    kaku.Decode( state , duration );
//    skytronic.Decode( state , duration );
//    lacrosse.Decode( state , duration );
    lacrosse2.Decode( state , duration );
//    siemens5wk4.Decode( state , duration );
//    mcvoice.Decode( state , duration );
//    houselink.Decode( state, duration );
//    x10.Decode( state, duration );
//    unknown1.Decode( state, duration );
//    elv.Decode( state, duration );
  }
}

void loop()
{
  currenttime = millis();
  
  if ( (currenttime - last_check_scheduledcommands) > UPDATEFREQUENCY_SCHEDULEDCOMMANDS )
  {
    loop_scheduledcommands();
    last_check_scheduledcommands = currenttime ;
  }

  if ( (currenttime - last_check_queuedcommands) > UPDATEFREQUENCY_QUEUEDCOMMANDS )
  {
    loop_queuedcommands();
    last_check_queuedcommands = currenttime;
  }

  loop_receive();
}

void setup()
{  
  // Initialize the serial-output
  Serial.begin(SerialBaudrate);
  
  Serial.println("http://arduinoha.googlecode.com");  
  Serial.print("CPU: ");Serial.print(F_CPU,DEC);Serial.println("Mhz"); 
  
  // Initialize the pins
  pinMode(DATAPIN, INPUT);
  pinMode(RSSIPIN, INPUT);
  pinMode(TXPIN, OUTPUT);
  pinMode(STATUSLEDPIN, OUTPUT);

  currenttime = millis();
  syncedtime = 0;
  syncedcurrenttime = currenttime;
  last_check_scheduledcommands = currenttime;
  last_check_queuedcommands = currenttime;
  lastrssitrigger = currenttime;
  
  sendreceivestate = waitingforrssitrigger;
  
  TCCR1A=0;   // Normal mode (Timer1)
  #ifdef PRESCALER256
    TCCR1B = 0<<ICES1 /* Input Capture Edge Select */ | 1<<CS12 /* prescaler */ | 0<<CS11 /* prescaler */  | 0<<CS10 /* prescaler */ | 1<<ICNC1 /* Capture Noice Canceler */;  
  #else
    TCCR1B = 0<<ICES1 /* Input Capture Edge Select */ | 0<<CS12 /* prescaler */ | 1<<CS11 /* prescaler */  | 1<<CS10 /* prescaler */ | 1<<ICNC1 /* Capture Noice Canceler */;  
  #endif
  TIMSK1 = 0<<ICIE1 /* Timer/Counter 1, Input Capture Interrupt disabled */ ;  

  attachInterrupt(RSSIIRQNR, rssiPinTriggered , RISING);
}
 
