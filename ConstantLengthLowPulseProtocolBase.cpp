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
	float shortperiods , // The number of timeperiods in a short-pulsecycle
	float longperiods // The number of timeperiods in a long-pulsecycle
	) : ProtocolBase(id , BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{
	_timeperiodduration = timeperiodduration;
	_shortperiods = shortperiods;
	_longperiods = longperiods;
	
	_highpulseduration = 0;
}

void ConstantLengthLowPulseProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
		unsigned int pulsecycleduration = duration + _highpulseduration ;
		if (WithinExpectedDeviation( pulsecycleduration , _shortperiods * _timeperiodduration ,  20) )
		{
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos == GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);				

		} else if (WithinExpectedDeviation( pulsecycleduration , _longperiods * _timeperiodduration ,  20) )
		{
			// Are there more bits in the buffer than is expected for this protocol?
			if (decoder_bitpos == GetBitstreamLength()) 
			{ 
				ShiftFirstBitOut(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
			}
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);			
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