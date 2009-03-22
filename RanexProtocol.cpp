#include "RanexProtocol.h"

//Based upon the Ranex RX 2585 remote, with the Laipac RLP434, OOK
//[  13]   44 [  13]   46 [  39]   12 [  45]   13 [  46]   12 [  43]   15 [  44]   14 [  41]   21 [   7]   47 [  11]   47 [  10]   47 [  12]   45 [  13]   44 [  11]   50 [  11]   43 [  14]   43 [  12]   48 [  12]   43 [  13]   44 [  15]   46 [  11]   43 [  11]   46 [  12]   46 [  12]   45 [  12]  451 
//[   9]   45 [  13]   44 [  42]   14 [  46]   12 [  41]   18 [  40]   17 [  41]   17 [  39]   18 [  11]   47 [  10]   48 [  12]   45 [  10]   48 [  10]   48 [   9]   48 [  12]   45 [  11]   45 [  12]   50 [   7]   49 [  11]   44 [  10]   47 [  11]   46 [  11]   51 [   7]   46 [  16]   42 [  11]  448
//[  12]   45 [  12]   45 [  45]   12 [  44]   18 [  37]   18 [  42]   15 [  42]   17 [  38]   18 [  11]   47 [  10]   51 [  11]   43 [  11]   46 [  11]   47 [  10]   47 [  11]   47 [  13]   44 [  12]   45 [  12]   46 [  11]   46 [  12]   46 [  11]   50 [   7]   47 [  11]   47 [  10]   48 [  10]  449
// The light outlets are quite resentive to overdriving the timerfrequency. They responded correctly even though i've upped the frequency by 1000herz..
RanexProtocol::RanexProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand)
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent , 24 , 4 , (TimerFrequency / 4369.993f ) , 1 , 3 )
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


