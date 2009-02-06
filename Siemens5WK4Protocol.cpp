#include "Siemens5WK4Protocol.h"

#define HIGH 0x1
#define LOW  0x0

// These protocol settings are based on the car-keys of my Opel Tigra Twintop 2005.
// The key: Siemens 433 MHz, 5WK4 8668
// Apparently the code is sent with 5,3kbit/sec (http://kfiducia.com/bmw_ca/FOBtechnicalDesc.pdf)
// The bitstream is Manchester encoded.
// The first bitstream is 186-bits long (96 bits preamble of "0"s), followed by 90 data-bits of the command)
// Next 3 repetitions of the 90 data-bits of the command

Siemens5WK4Protocol::Siemens5WK4Protocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*LockCommandReceivedEvent)(ProtocolBase * protocol , bool lockcommand),
	void (*TwoButtonsPressedReceivedEvent)(ProtocolBase * protocol)
	) : ManchesterCodeProtocolBase(id,BitsstreamReceivedEvent , 90 , 4 , (TimerFrequency / 1953.125f) , 10.0f)
{	
	_LockCommandReceivedEvent = LockCommandReceivedEvent;
	_TwoButtonsPressedReceivedEvent = TwoButtonsPressedReceivedEvent;
}

void Siemens5WK4Protocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (WithinExpectedDeviation( _highduration , _oscilator * 8.5f ,  _oscilator / 4) || WithinExpectedDeviation( _highduration , _oscilator * 11.5f ,  _oscilator / 4) )
	{
		int startpos = decoder_bitpos - 90;
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent(this, decoder_bitbuffer , decoder_bitpos);
		if (GetBit(decoder_bitbuffer, decoder_bitbufferlength, startpos + 14))
		{		
			if (GetBit(decoder_bitbuffer, decoder_bitbufferlength, startpos + 15) && _TwoButtonsPressedReceivedEvent!=0 )  _TwoButtonsPressedReceivedEvent(this);
		} else
		{
			if (GetBit(decoder_bitbuffer, decoder_bitbufferlength, startpos + 16) && _LockCommandReceivedEvent!=0 )  _LockCommandReceivedEvent(this,  true);
			if (GetBit(decoder_bitbuffer, decoder_bitbufferlength, startpos + 15) && _LockCommandReceivedEvent!=0 )  _LockCommandReceivedEvent(this,  false);
		}
	}	
}