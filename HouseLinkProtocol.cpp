#include "HouseLinkProtocol.h"

HouseLinkProtocol::HouseLinkProtocol(
	char * id,
	double TimerFrequency,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte device ),
	void (*DeviceBatteryEmptyEvent)(ProtocolBase * protocol , byte device )
	) : ConstantLengthHighPulseProtocolBase(id, BitsstreamReceivedEvent, 15 , 1, (TimerFrequency / 1953.125f) , (TimerFrequency / 771.6049f) , 1 , 3)
{
	_DeviceTrippedEvent = DeviceTrippedEvent;
	_DeviceBatteryEmptyEvent = DeviceBatteryEmptyEvent;
}

void HouseLinkProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if ( decoder_bitpos==GetBitstreamLength() )
	{
	if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
	
	
/*	bool BatteryEmpty = !GetBit(decoder_bitbuffer, decoder_bitbufferlength, 16);

	unsigned short int device = (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 15)?1:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 13)? 2:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 11)? 4:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 9)?8:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 7)?16:0)  + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 5)? 32:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 3)?64:0) + (!GetBit(decoder_bitbuffer, decoder_bitbufferlength, 1)?128:0) ;

	if (_DeviceTrippedEvent!=0) _DeviceTrippedEvent(this, device);
	if (_DeviceBatteryEmptyEvent!=0 && BatteryEmpty)  _DeviceBatteryEmptyEvent(this, device);*/
}
}

void HouseLinkProtocol::EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;
}