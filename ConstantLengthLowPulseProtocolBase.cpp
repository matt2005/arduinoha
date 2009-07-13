#include "ConstantLengthLowPulseProtocolBase.h"
#include <hardwareserial.h>

#define HIGH 0x1
#define LOW  0x0


ConstantLengthLowPulseProtocolBase::ConstantLengthLowPulseProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),	
	int bitstreamlength,
	int sendrepeats , 
	float timeperiodduration , // The duration of one oscillation
	float longhighperiods , // The number of timeperiods in a long high
	float shorthighperiods, // The number of timeperiods in a short high
	float constantlowperiods // The number of timeperiods in low
	) : ProtocolBase(id , BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{
	_timeperiodduration = timeperiodduration;
	_longhighperiods = longhighperiods;
	_shorthighperiods = shorthighperiods;
	_constantlowperiods = constantlowperiods;
	
	_highpulseduration = 0;
}

void ConstantLengthLowPulseProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
		unsigned int pulsecycleduration = duration + _highpulseduration ;
		if (WithinExpectedDeviation( pulsecycleduration , (_shorthighperiods + _constantlowperiods ) * _timeperiodduration ,  _timeperiodduration) )
		{
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos == GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);				

		} else if (WithinExpectedDeviation( pulsecycleduration , (_longhighperiods + _constantlowperiods) * _timeperiodduration ,  20) )
		{
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos == GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);			
		} else 
		{
			DecodeBitstream(_highpulseduration, duration);
			ResetDecoder();
		}
    } else if (state==HIGH)
	{
		_highpulseduration = duration;
	}
}

void ConstantLengthLowPulseProtocolBase::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (_BitsstreamReceivedEvent!=0 && decoder_bitpos == GetBitstreamLength()) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
}

void ConstantLengthLowPulseProtocolBase::EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue)
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  if (bitvalue)
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , _shorthighperiods * _timeperiodduration );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , _constantlowperiods * _timeperiodduration );
  } else
  {
    SetPulse(pulsebuffer, pulsebufferlength, 0 , _longhighperiods * _timeperiodduration );
    SetPulse(pulsebuffer, pulsebufferlength, 1 , _constantlowperiods * _timeperiodduration );
  }
}
