#include "ProtocolBase.h"
#include "ManchesterCodeProtocolBase.h"
#include <hardwareserial.h>

#define HIGH 0x1
#define LOW  0x0


ManchesterCodeProtocolBase::ManchesterCodeProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	int bitstreamlength,
	int sendrepeats,
	float Oscilator	,
	float TerminatorOscilators) : ProtocolBase(id, BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{	
	_oscilator = Oscilator;
	_terminatoroscilators = TerminatorOscilators;
	mc_insync = true;
	_highduration = 0;
}

void ManchesterCodeProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
	  int pulsecycleduration = duration + _highduration;
	  /*if (WithinExpectedDeviation( pulsecycleduration , _oscilator * 2 ,  _oscilator ) )
	  { // half high, single low or half low, single high
		if (_highduration > duration)
		{
			mc_insync = false;
		} else
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false );
			mc_insync = false;
		}
	  } else*/ if (WithinExpectedDeviation( pulsecycleduration , _oscilator * 2 ,  _oscilator / 4) )
	  { // single high, single low
		if (mc_insync)
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true );
		} else
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false );
		}		
	  } else if (WithinExpectedDeviation( pulsecycleduration , _oscilator * 3 ,  _oscilator / 4) )
	  { // single and double pulse
		if (_highduration>duration) 
		{	// double high, single low
			if (mc_insync)
			{ // Error, sync incorrect
				FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
				mc_insync = !mc_insync;
			}
			
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
			mc_insync = true;
		} else
		{ // single high, double low
			if (!mc_insync)
			{ // Error, sync incorrect
				FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
				mc_insync = !mc_insync;
			}
			
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
			mc_insync = false;
		}
	  } else if ( WithinExpectedDeviation( pulsecycleduration , _oscilator * 4 ,  _oscilator / 4))
      { // double high, double low
			if (mc_insync)
			{ // Error, sync incorrect
				FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
				mc_insync = !mc_insync;
			}
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);
      } else 
	  { // Terminator
		if (!mc_insync)
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
		}
		if (decoder_bitpos>=GetBitstreamLength() )
		{ 
			DecodeBitstream(_highduration, duration);
		}
		ResetDecoder();
		mc_insync = false;
		AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);
	  } 
    } else
	{	// High pulse
		_highduration = duration;
	}
}

void ManchesterCodeProtocolBase::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
}
