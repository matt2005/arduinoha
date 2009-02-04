#include "KlikAanKlikUitProtocol.h"

// Klik Aan Klik Uit - TMT-502
//[  20]   68 [  20]   68 [  19]   68 [  64]   25 [  18]   69 [  19]   69 [  19]   69 [  19]   68 [  19]   69 [  19]   69 [  19]   69 [  19]   68 [  19]   69 [  19]   69 [  19]   69 [  19]   68 [  19]   69 [  19]   69 [  19]   69 [  62]   26 [  18]   69 [  63]   25 [  18]   70 [  62]   26 [  18]  623 
//[  12]   53 [  32]   55 [  31]   57 [  71]   18 [  25]   62 [  26]   62 [  25]   64 [  24]   64 [  23]   65 [  23]   64 [  24]   64 [  24]   64 [  23]   66 [  22]   65 [  22]   66 [  22]   66 [  21]   66 [  22]   66 [  22]   66 [  65]   23 [  21]   67 [  64]   24 [  19]   69 [  63]   25 [  18]  626 
//[  20]   67 [  21]   67 [  21]   67 [  64]   23 [  20]   69 [  19]   69 [  19]   68 [  20]   68 [  19]   69 [  19]   69 [  19]   68 [  20]   68 [  20]   68 [  20]   67 [  20]   68 [  20]   68 [  19]   69 [  19]   68 [  20]   68 [  63]   25 [  19]   69 [  62]   26 [  18]   70 [  61]   27 [  17]  620 
//[  20]   68 [  20]   68 [  19]   69 [  63]   25 [  18]   69 [  19]   69 [  19]   69 [  18]   70 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  19]   68 [  19]   69 [  19]   69 [  19]   68 [  19]   69 [  62]   26 [  18]   70 [  61]   27 [  17]   71 [  60]   28 [  16]  628 
//[  19]   68 [  19]   69 [  19]   69 [  62]   26 [  18]   69 [  20]   68 [  18]   70 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  62]   26 [  18]   69 [  62]   26 [  18]   70 [  61]   27 [  17]  623
// [  19]   69 [  19]   69 [  18]   69 [  62]   26 [  18]   70 [  18]   70 [  18]   69 [  19]   69 [  18]   70 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  19]   69 [  19]   69 [  18]   69 [  63]   25 [  18]   70 [  62]   26 [  17]   71 [  61]   27 [  17]  623

KlikAanKlikUitProtocol::KlikAanKlikUitProtocol(
	char * id,
	double TimerFrequency,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand)
	) : ConstantLengthPulseCycleProtocolBase(id, BitsstreamReceivedEvent, 24 , 4, (TimerFrequency / 12480.53f) , 4 , 13.5)
{
	_DeviceCommandReceivedEvent = DeviceCommandReceivedEvent;
}

void KlikAanKlikUitProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
		byte device = (GetBit(decoder_bitbuffer,decoder_bitbufferlength,9)?1:0)  + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,11)?2:0) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,13)?4:0) + (GetBit(decoder_bitbuffer,decoder_bitbufferlength,15)?8:0) ;
		bool lighton = GetBit(decoder_bitbuffer, decoder_bitbufferlength,23);
		if (_DeviceCommandReceivedEvent!=0) _DeviceCommandReceivedEvent( this , device , lighton);
	}
}

void KlikAanKlikUitProtocol::EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;

  SetBit(bitbuffer, bitbufferlength ,  9 , (device & 1) == 1);
  SetBit(bitbuffer, bitbufferlength , 11 , (device & 2) == 2);
  SetBit(bitbuffer, bitbufferlength , 13 , (device & 4) == 4);
  SetBit(bitbuffer, bitbufferlength , 15 , (device & 8) == 8);
  SetBit(bitbuffer, bitbufferlength , 23 , lighton );

  SetBit(bitbuffer, bitbufferlength ,  3 , true);
  SetBit(bitbuffer, bitbufferlength ,  19 , true);
  SetBit(bitbuffer, bitbufferlength ,  21 , true);
}

