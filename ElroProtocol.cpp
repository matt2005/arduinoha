#include "ElroProtocol.h"

//Based upon the ElroAB600R remote, with the Laipac RLP434, OOK
ElroProtocol::ElroProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte group, byte device , bool turnoncommand)
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent , 24 , 4 , (TimerFrequency / 3125.0f) , 1 , 3 )
{	
	_DeviceCommandReceivedEvent = DeviceCommandReceivedEvent;
}

void ElroProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{	
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		byte group = 0;	
		for (int idx = 0 ; idx<4; idx++) group |= ( GetBit( decoder_bitbuffer,decoder_bitbufferlength,1 + (idx << 1) ) ?  (1 << idx) : 0 ) ;
		
		byte device = 0;	
		for (int idx = 0 ; idx<5; idx++) device |= ( GetBit( decoder_bitbuffer,decoder_bitbufferlength,9 + (idx << 1) ) ?  (1 << idx) : 0 ) ;
		bool lighton = GetBit(decoder_bitbuffer, decoder_bitbufferlength, 23);
		bool valid = true;
		for (int idx = 0;idx<=11;idx++) if (GetBit( decoder_bitbuffer,decoder_bitbufferlength,(idx << 1))) valid = false;
		if (valid)
		{
			if 	(_DeviceCommandReceivedEvent!=0) _DeviceCommandReceivedEvent( this , group, device , lighton);
		}
	}
}
