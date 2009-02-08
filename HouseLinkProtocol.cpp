#include "HouseLinkProtocol.h"

HouseLinkProtocol::HouseLinkProtocol(
	char * id,
	double TimerFrequency,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte group , byte device , bool state )
	) : ConstantLengthHighPulseProtocolBase(id, BitsstreamReceivedEvent, 14 , 1, (TimerFrequency / 994.7477f) , 1 , 2)
{
	_DeviceTrippedEvent = DeviceTrippedEvent;
}

void HouseLinkProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if ( decoder_bitpos==GetBitstreamLength() )
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		
		byte device = 0;
		for (int idx=0;idx<=5;idx++)
		{ 
			device |= (GetBit(decoder_bitbuffer, decoder_bitbufferlength, idx)? (32 >>idx):0) ;
		}
		byte group = (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 9)?2:0)  + (GetBit(decoder_bitbuffer, decoder_bitbufferlength, 10)? 1:0);
		
		bool checkbit = false;
		for (int idx=0;idx<=11;idx++)
		{
			if (GetBit(decoder_bitbuffer, decoder_bitbufferlength, idx)) checkbit = !checkbit;
		}
		bool state = GetBit(decoder_bitbuffer, decoder_bitbufferlength, 11);
		if (checkbit == GetBit(decoder_bitbuffer, decoder_bitbufferlength, 12))
		{
			if (_DeviceTrippedEvent!=0) _DeviceTrippedEvent(this, group, device, state );
		}
	}
}

void HouseLinkProtocol::EncodeDeviceCommand(byte group , byte device, bool state, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;
}