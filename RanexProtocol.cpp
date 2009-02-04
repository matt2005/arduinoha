#include "RanexProtocol.h"

RanexProtocol::RanexProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand)
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent , 24 , 4 , (TimerFrequency / 17857.142f) , 4 , 12 )
{	
	_DeviceCommandReceivedEvent = DeviceCommandReceivedEvent;
}

void RanexProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{	
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		byte device = 0;	
		for (int idx = 0 ; idx<3; idx++) device |= ( GetBit( decoder_bitbuffer,decoder_bitbufferlength,2 + (idx << 1) ) ? 0 : (1 << idx) ) ;
		bool lighton = GetBit(decoder_bitbuffer, decoder_bitbufferlength, 16);
		if 	(_DeviceCommandReceivedEvent!=0) _DeviceCommandReceivedEvent( this , device , lighton);
	}
}

void RanexProtocol::EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;

  SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );
  if (bitbuffer==0) return;
  
  for (int idx=0;idx<3;idx++) 
  {
	bool v = (device & ( 1 << (byte)idx ) ) == 0;
	SetBit(bitbuffer, bitbufferlength ,  2 + (idx << 1 ) , v );
	SetBit(bitbuffer, bitbufferlength ,  3 + (idx << 1 ) , v );
  }
  
  SetBit(bitbuffer, bitbufferlength ,  16 , lighton );
  SetBit(bitbuffer, bitbufferlength ,  17 , lighton );
}


