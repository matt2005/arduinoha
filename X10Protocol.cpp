#include "X10Protocol.h"

X10Protocol::X10Protocol(
	char * id,
	double TimerFrequency,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte group , byte device , bool state )
	) : ConstantLengthHighPulseProtocolBase(id, BitsstreamReceivedEvent, 32 , 3, (TimerFrequency / 946.97f) , 1 , 2)
{
	_DeviceTrippedEvent = DeviceTrippedEvent;
}

void X10Protocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if ( decoder_bitpos==GetBitstreamLength() )
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		FlipBits(decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos);
		byte device = 0;
		
		device |= (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 27)? 1:0) ;
		device |= (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 28)? 2:0) ;
		device |= (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 25)? 4:0) ;		
		device |= (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 26)? 8:0) ;
		
		byte group = 0;
		
		//bool state = GetBit(decoder_bitbuffer, decoder_bitbufferlength, 19);
		bool state =false;
		if (_DeviceTrippedEvent!=0) _DeviceTrippedEvent(this, group, device, state );
	}
}

void X10Protocol::EncodeDeviceCommand(byte group , byte device, bool state, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;
}