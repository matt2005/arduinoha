#include "ConstantLengthDoublePulseCycleProtocolBase.h"


// Skytronic remote with TM58P10SDCW-0547 IC

#define HIGH 0x1
#define LOW  0x0


ConstantLengthDoublePulseCycleProtocolBase::ConstantLengthDoublePulseCycleProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),	
	int bitstreamlength,
	int sendrepeats , 
	float timeperiodduration , // The duration of one oscillation
	float syncbitperiods , // The number of timeperiods in the entire syncbit-pulsecycle
	float shortperiods , // The number of timeperiods in a short-pulse
	float longperiods // The number of timeperiods in a long-pulse
	) : ProtocolBase(id , BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{
	_timeperiodduration = timeperiodduration;
	_syncbitperiods = syncbitperiods;
	_shortperiods = shortperiods;
	_longperiods = longperiods;
	
	_highpulseduration = 0;
	_prev_pulsecycleduration = 0;
}

void ConstantLengthDoublePulseCycleProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
		unsigned int pulsecycleduration = duration + _highpulseduration ;
		if (WithinExpectedDeviation( pulsecycleduration , (_shortperiods+_longperiods) * _timeperiodduration ,  _timeperiodduration / 8) )
		{
			if (_prev_pulsecycleduration!=0)
			{
				if (WithinExpectedDeviation( _prev_pulsecycleduration , (_shortperiods+_longperiods) * _timeperiodduration ,  _timeperiodduration / 8) )
				{
					FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
				} else
				{
					AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);				
					pulsecycleduration = 0;
				}
			}
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos > GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
		} else if (WithinExpectedDeviation( pulsecycleduration , ( _shortperiods + _shortperiods ) * _timeperiodduration ,  _timeperiodduration / 8) )
		{
			if (_prev_pulsecycleduration!=0)
			{
				if (WithinExpectedDeviation( _prev_pulsecycleduration , ( _shortperiods + _shortperiods ) * _timeperiodduration ,  _timeperiodduration / 8) ) 
				{
					FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
				} else
				{
					AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);			
					pulsecycleduration = 0;
				}
			}
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos > GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
		} else 
		{
			DecodeBitstream(_highpulseduration, duration);
			ResetDecoder();
			pulsecycleduration = 0;
		}
		_prev_pulsecycleduration = pulsecycleduration;
    } else if (state==HIGH)
	{
		_highpulseduration = duration;
	}
}

// This function returns a buffer with the pulse data of a bit for this protocol
void ConstantLengthDoublePulseCycleProtocolBase::EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue)
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , 33/*1 * _timeperiodduration */);
  if (bitvalue) 
  {	
	SetPulse(pulsebuffer, pulsebufferlength, 1 , 68/*2 * _timeperiodduration */);
  } else
  {
  	SetPulse(pulsebuffer, pulsebufferlength, 1 , 34/*1 * _timeperiodduration */);
  }
  
  SetPulse(pulsebuffer, pulsebufferlength, 2 , 33/*1 * _timeperiodduration */);
  if (bitvalue) 
  {	
	SetPulse(pulsebuffer, pulsebufferlength, 3 , 34/*1 * _timeperiodduration */);
  } else
  {
  	SetPulse(pulsebuffer, pulsebufferlength, 3 , 68/*2 * _timeperiodduration */);
  }
}

void ConstantLengthDoublePulseCycleProtocolBase::EncodeHeader(unsigned int *& pulsebuffer, byte & pulsebufferlength) 
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  SetPulse(pulsebuffer, pulsebufferlength, 0 ,  33 /*_timeperiodduration  / 2*/);
  SetPulse(pulsebuffer, pulsebufferlength, 1 ,  34 /*_timeperiodduration  / 2*/);
  SetPulse(pulsebuffer, pulsebufferlength, 2 ,  33);
  SetPulse(pulsebuffer, pulsebufferlength, 3 ,  34);
  SetPulse(pulsebuffer, pulsebufferlength, 4 ,  33);
  SetPulse(pulsebuffer, pulsebufferlength, 5 ,  34);
  SetPulse(pulsebuffer, pulsebufferlength, 6 ,  33);
  SetPulse(pulsebuffer, pulsebufferlength, 7 ,  34);
}


// This function returns a buffer with the pulse data of the terminator for this protocol
void ConstantLengthDoublePulseCycleProtocolBase::EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength) 
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , 33/*1 * _timeperiodduration */);
  SetPulse(pulsebuffer, pulsebufferlength, 1 , 650 /*_syncbitperiods * _timeperiodduration */);
}

void ConstantLengthDoublePulseCycleProtocolBase::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
}
