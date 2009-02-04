#include "SkytronicProtocol.h"
#include <HardwareSerial.h>

//[  28]   37 [  27]   38 [  27]   38 [  26]   39 [  26]   69 [  26]   38 [  27]   37 [  27]   69 [  26]   68 [  27]   38 
//[  27]   68 [  26]   39 [  26]   68 [  27]   39 [  26]   38 [  27]   68 [  27]   68 [  26]   39 [  26]   39 [  26]   69 
//[  26]   68 [  26]   39 [  27]   38 [  26]   69 [  26]   68 [  26]   40 [  25]   39 [  26]   69 [  26]   68 [  26]   40 
//[  26]   38 [  26]   69 [  25]   71 [  24]   41 [  25]   39 [  25]   71 [  24]   69 [  26]   39 [  26]   69 [  25]   40 
//[  26]   68 [  26]   40 [  25]   41 [  23]   72 [  24]   40 [  24]   71 [  24]   70 [  25]   41 [  24]   41 [  23]   71 
//[  25]   40 [  24]   71 [  24]   40 [  25]   70 [  25]   39 [  25]   70 [  26]   39 [  25]   70 [  25]   40 [  24]   71 
//[  25]   39 [  25]   70 [  25]   40 [  25]   70 [  25]   40 [  24]   71 [  25]   41 [  22]   71 [  24]  652 

// [  30]   34 [  29]   34 [  29]   35 [  29]   35 [  29]   35 [  28]   66 [  28]   65 [  28]   36 
// [  28]   35 [  28]   67 [  27]   65 [  28]   37 [  27]   66 [  27]   37 [  27]   36 [  27]   67 [  27]   66 [  27]   37 [  27]   37 
//[  27]   67 [  27]   36 [  26]   68 [  27]   36 [  27]   67 [  26]   67 [  26]   38 [  27]   66 [  27]   37 [  27]   66 [  27]   38 
//[  26]   37 [  27]   67 [  27]   66 [  27]   37 [  27]   37 [  26]   68 [  26]   67 [  26]   38 [  26]   67 [  27]   38 [  26]   67 
//[  26]   38 [  26]   38 [  26]   68 [  26]   37 [  26]   68 [  26]   67 [  26]   39 [  26]   37 [  26]   68 [  26]   37 [  27]   68 
//[  26]   37 [  26]   68 [  26]   38 [  25]   69 [  26]   37 [  26]   68 [  26]   38 [  26]   67 [  27]   37 [  26]   68 [  26]   38 
//[  26]   68 [  26]   37 [  26]   68 [  26]   38 [  26]   67 [  26]  642

SkytronicProtocol::SkytronicProtocol(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol, byte device , bool turnoncommand),
	void (*AllDevicesCommandReceivedEvent)(ProtocolBase * protocol, bool turnoncommand) )  : ConstantLengthDoublePulseCycleProtocolBase(id, BitsstreamReceivedEvent, 32 , 4 , (TimerFrequency / 1860.119f)  , 21.0f, 1 , 2 )
{	
	_DeviceCommandReceivedEvent = DeviceCommandReceivedEvent;
	_AllDevicesCommandReceivedEvent = AllDevicesCommandReceivedEvent;
}

void SkytronicProtocol::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (decoder_bitpos==GetBitstreamLength())
	{
		if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent(this, decoder_bitbuffer , decoder_bitpos);
		byte device = 0;
		if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 11))
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 10))
			{
				device = 12;
			} else
			{
				device = 8;
			}
		} else
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 10))
			{
				device = 0;
			} else
			{
				device = 4;
			}
		}
		
		if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 8))
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 9))
			{
				device |= 1;
			} else
			{
				device |= 2;
			}
		} else
		{
			if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 9))
			{
				device |= 0;
			} else
			{
				device |= 3;
			}
		}
	
		bool lighton = GetBit(decoder_bitbuffer,decoder_bitbufferlength, 4); 

		if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 7))
		{
			if (_AllDevicesCommandReceivedEvent!=0) _AllDevicesCommandReceivedEvent(this, lighton);
		} 
		
		if (GetBit(decoder_bitbuffer,decoder_bitbufferlength, 6))
		{
			if (_DeviceCommandReceivedEvent!=0) _DeviceCommandReceivedEvent(this, device , lighton);
		}
	}
}

void SkytronicProtocol::EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;
  
  SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );
  if (bitbuffer==0) return;
  
  // Device bits
  if ( ((device & 3) == 1) || ((device & 3) == 2) )  SetBit(bitbuffer,bitbufferlength , 8 , true );
  if ( (device & 2) == 0 )  SetBit(bitbuffer,bitbufferlength , 9 , true );
  if ( (device>=0 && device<=3) || (device>=12 && device<=15)) SetBit(bitbuffer,bitbufferlength , 10 , true );
  if ( (device & 8) != 0) SetBit(bitbuffer,bitbufferlength , 11 , true );
  
  if (lighton)
  {
	SetBit(bitbuffer,bitbufferlength , 0 , GetBit(bitbuffer,bitbufferlength , 8) );
  } else
  {
	SetBit(bitbuffer,bitbufferlength , 0 , !GetBit(bitbuffer,bitbufferlength , 8) );
  }
  
  if ( ((device & 3) == 1) || ((device & 3) == 3) ) SetBit(bitbuffer,bitbufferlength , 1 , true );
  if ( (device>=0 && device<=2) || (device==7) || (device>=11 && device<=14)) SetBit(bitbuffer,bitbufferlength , 2 , true );
  if ( (device>=0 && device<=2) || (device>=8 && device<=11) || (device==15)) SetBit(bitbuffer,bitbufferlength , 3 ,true);
  
  SetBit(bitbuffer,bitbufferlength , 4 , lighton);
  SetBit(bitbuffer,bitbufferlength , 6 , true);
  
  SetBit(bitbuffer,bitbufferlength , 12 , true);
  SetBit(bitbuffer,bitbufferlength , 14 , true);
  SetBit(bitbuffer,bitbufferlength , 16 , true);
  SetBit(bitbuffer,bitbufferlength , 17 , true);
  SetBit(bitbuffer,bitbufferlength , 18 , true);
  SetBit(bitbuffer,bitbufferlength , 21 , true);
}


void SkytronicProtocol::EncodeAllDevicesCommand(bool lighton, byte *& bitbuffer, byte &bitbufferlength )
{
	bitbuffer = 0;
	bitbufferlength = 0;
  
	SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );
	
	SetBit(bitbuffer,bitbufferlength , 0 , !lighton);
	SetBit(bitbuffer,bitbufferlength , 1 , true);
	SetBit(bitbuffer,bitbufferlength , 3 , true);
	SetBit(bitbuffer,bitbufferlength , 4 , lighton);
	SetBit(bitbuffer,bitbufferlength , 7 , true);	
	SetBit(bitbuffer,bitbufferlength , 12 , true);
	SetBit(bitbuffer,bitbufferlength , 14 , true);
	SetBit(bitbuffer,bitbufferlength , 16 , true);
	SetBit(bitbuffer,bitbufferlength , 17 , true);
	SetBit(bitbuffer,bitbufferlength , 18 , true);
	SetBit(bitbuffer,bitbufferlength , 21 , true);
}