#include "LaCrosseProtocol.h"
#include <hardwareserial.h>

#define CombinationDuration1 147
#define CombinationDuration2 99
#define Terminator 970

#define HIGH 0x1
#define LOW  0x0

// The LaCrosse-protcol looks like it's using some kind of Manchester-related-code.
// I don't think it's a pure MC-implementation as i don't find the 2:3:4 proportions, but a 2 : 3 : 4.67 proportion-ratio.

LaCrosseProtocol::LaCrosseProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*TemperatureReceivedEvent)(ProtocolBase * protocol , byte device , float temperature),
	void (*HygroReceivedEvent)(ProtocolBase * protocol , byte device , float hygro),
	void (*RainReceivedEvent)(ProtocolBase * protocol , byte device , int rain)
	) : ProtocolBase(id,BitsstreamReceivedEvent , 42 , 4)
{	
	_TemperatureReceivedEvent = TemperatureReceivedEvent;
	_HygroReceivedEvent = HygroReceivedEvent;
	_RainReceivedEvent = RainReceivedEvent;
	
	_timeperiodduration = (TimerFrequency / 5953.38f);
	_maxdeviation = _timeperiodduration;
	
	prevduration = 0;
}

void LaCrosseProtocol::Decode(short state, unsigned int duration)
{  
  if (state==LOW)
  {
    // Calculate the length of the high and low 
	int pulsecycleduration = duration + prevduration;
	
	if (WithinExpectedDeviation( pulsecycleduration , (6 * _timeperiodduration) ,  _maxdeviation) )
	{ // 	 
		AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
	} else if (WithinExpectedDeviation( pulsecycleduration , (14 * _timeperiodduration) ,  _maxdeviation) )
	{ // 0
		AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
	} else if (WithinExpectedDeviation( pulsecycleduration , (9 * _timeperiodduration) ,  _maxdeviation) )
    { // 1
        AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
    } else if ( decoder_bitpos>=GetBitstreamLength() && decoder_bitpos<=GetBitstreamLength()+3 && pulsecycleduration>(160*_timeperiodduration))
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
			
		byte startbitpos = decoder_bitpos - 42;
			
		int deviceid=0;
		for (int idx=7;idx<=10;idx++) deviceid |= GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + idx  ) ? (1 << (idx-7)) : 0;
			
		if (deviceid==0 || deviceid==7)
		{ 
			byte frac=0;for (int idx=30;idx>=27;idx--) frac |= GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + idx) ? (1 << (30-idx)):0;
			
			byte absvalues[] = {30, 0 ,20 , 10 , 8 , 4, 2, 1 };
			byte abs=0;for (int idx=19;idx<=26;idx++) abs += GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + idx) ? absvalues[idx-19]:0;
			float value = abs + (frac/10.0f);
			
			if ((deviceid==0) && (_TemperatureReceivedEvent!=0)) _TemperatureReceivedEvent( this , deviceid , value - 10.0f);
			if ((deviceid==7) && (_HygroReceivedEvent!=0)) _HygroReceivedEvent( this , deviceid , value + 40.0f);
		} else if (deviceid==5)
		{ 
			int value = 0;
			for (int idx=30;idx>=19;idx--) value |= GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + idx) ? (1 << (30-idx)):0;
		
			//int sequence = (GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + 31 ) ? 1 : 0) | (GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + 30  ) ? 2 : 0) | (GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + 29  ) ? 4 : 0) ;
			//bool oddbits = GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + 18);
			
			if (_RainReceivedEvent!=0) _RainReceivedEvent( this , deviceid , value );
		}  
		ResetDecoder();
    } else 
	{
		ResetDecoder();
	}
  }
  prevduration = duration;
}

void LaCrosseProtocol::EncodeTemperatureCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;  
  
  //SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );

  //float bitvalues[] = {30, 0 ,20 , 10 , 8 , 4, 2, 1 , 0.8f, 0.4f, 0.2f, 0.1f};
  //for (int idx=19;idx<=30;idx++) value += GetBit(decoder_bitbuffer, decoder_bitbufferlength , startbitpos + idx) ? bitvalues[idx-19]:0;
  
  /*SetBit(bitbuffer, bitbufferlength ,  2 , (device & (byte)1) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  3 , (device & (byte)1) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  4 , (device & (byte)2) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  5 , (device & (byte)2) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  6 , (device & (byte)4) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  7 , (device & (byte)4) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  16 , lighton );
  SetBit(bitbuffer, bitbufferlength ,  17 , lighton );*/
}

void LaCrosseProtocol::EncodeRainCommand(byte device, int value , byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;  
  
  //SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );
  
  /*SetBit(bitbuffer, bitbufferlength ,  2 , (device & (byte)1) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  3 , (device & (byte)1) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  4 , (device & (byte)2) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  5 , (device & (byte)2) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  6 , (device & (byte)4) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  7 , (device & (byte)4) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  16 , lighton );
  SetBit(bitbuffer, bitbufferlength ,  17 , lighton );*/
}

void LaCrosseProtocol::EncodeHygroCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;  
  
  //SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );
  
  /*SetBit(bitbuffer, bitbufferlength ,  2 , (device & (byte)1) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  3 , (device & (byte)1) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  4 , (device & (byte)2) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  5 , (device & (byte)2) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  6 , (device & (byte)4) == 0 );
  SetBit(bitbuffer, bitbufferlength ,  7 , (device & (byte)4) == 0 );
  
  SetBit(bitbuffer, bitbufferlength ,  16 , lighton );
  SetBit(bitbuffer, bitbufferlength ,  17 , lighton );*/
}


// This function returns a buffer with the pulse data of a bit for this protocol
void LaCrosseProtocol::EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue)
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  if (bitvalue)
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , 28 );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , 67 );
  } else
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , 30 );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , 34 );
  }
}

// This function returns a buffer with the pulse data of the terminator for this protocol
void LaCrosseProtocol::EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength) 
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , 25 );
  SetPulse(pulsebuffer, pulsebufferlength, 1 , Terminator);
}
