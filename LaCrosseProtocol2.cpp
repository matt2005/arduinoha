#include "LaCrosseProtocol2.h"
#include <hardwareserial.h>

#define HIGH 0x1
#define LOW  0x0

// Based upon an "WS-2300-25"
// [  98]   78 [  96]   81 [  93]   75 [  24]   83 [  92]   83 [  92]   84 [  23]   84 [  90]   85 [  22]   85 [  22]   85 [  90]   86 [  89]   86 [  89]   87 [  89]   86 [  20]   87 [  20]   87 [  89]   86 [  20]   87 [  89]   87 [  88]   87 [  20]   87 [  20]   87 [  19]   88 [  19]   88 [  87]   88 [  88]   88 [  87]   88 [  87]   89 [  87]   89 [  86]   89 [  86]   90 [  86]   89 [  86]   90 [  86]   89 [  86]   90 [  85]   90 [  86]   90 [  85]   91 [  85]   90 [  16]   91 [  16]   91 [  16]   91 [  16]   90 [  17]   90 [  17]   90 [  16]   91 [  16]   91 [  85]   90 [  16]   91 [  16]   91 [  16] 12174 

LaCrosseProtocol2::LaCrosseProtocol2(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),
	void (*RainReceivedEvent)(ProtocolBase * protocol , byte device , int rain),
	void (*WindReceivedEvent)(ProtocolBase * protocol , byte device , int rain)
	) : ConstantLengthLowPulseProtocolBase(id,BitsstreamReceivedEvent , 50, 3, (TimerFrequency / 3531.0f) , /*long high factor */ 5.0f , /* short high factor */ 1.0f , /* constant low factor*/ 5.0f)
{	
	_RainReceivedEvent = RainReceivedEvent;
	_WindReceivedEvent = WindReceivedEvent;
}

	void LaCrosseProtocol2::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
	{
		if (decoder_bitpos==GetBitstreamLength())
		{
			byte retransmit = GetBit(decoder_bitbuffer,decoder_bitbufferlength, 8 ); 
			
			byte device= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 9 )?4:0; 
			device |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 10 )?2:0; 
			device |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 11 )?1:0; 

			if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
				
			if (device==4)
			{					
				byte v1 = GetBit(decoder_bitbuffer,decoder_bitbufferlength, 31 )?128:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 32 )?64:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 33 )?32:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 34 )?16:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 35 )?8:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 36 )?4:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 37 )?2:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 38 )?1:0; 

				if (_RainReceivedEvent!=0) _RainReceivedEvent( this , 0 , v1 );
			}
			if (device==0)
			{
			    // 30-36 is inverse of 28-34 
				byte v1 = GetBit(decoder_bitbuffer,decoder_bitbufferlength, 28 )?64:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 29 )?32:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 30 )?16:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 31 )?8:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 32 )?4:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 33 )?2:0; 
				v1 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 34 )?1:0; 
				
				byte v2 = GetBit(decoder_bitbuffer,decoder_bitbufferlength, 35 )?8:0; 
				v2 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 36 )?4:0; 
				v2 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 37 )?2:0; 
				v2 |= GetBit(decoder_bitbuffer,decoder_bitbufferlength, 38 )?1:0;
				Serial.print(" ");
				Serial.print(v1,DEC);
				Serial.print(" ");
				Serial.println(v2,DEC);
				//if (_WindReceivedEvent!=0) _WindReceivedEvent( this , 0 , v1 );
			}
		}
	}
	
void LaCrosseProtocol2::EncodeTemperatureCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength )
{
  bitbuffer = 0;
  bitbufferlength = 0;  
  
  SetBitBufferLength(bitbuffer, bitbufferlength , GetBitstreamLength() );

  SetBit(bitbuffer, bitbufferlength ,  3 , true );
  SetBit(bitbuffer, bitbufferlength ,  6 , true );
  
  SetBit(bitbuffer, bitbufferlength ,  14 , true );
  SetBit(bitbuffer, bitbufferlength ,  15 , true );
  SetBit(bitbuffer, bitbufferlength ,  16 , true );
  SetBit(bitbuffer, bitbufferlength ,  17 , true );
  SetBit(bitbuffer, bitbufferlength ,  18 , true );
  
  SetBit(bitbuffer, bitbufferlength ,  20 , true );
  SetBit(bitbuffer, bitbufferlength ,  21 , true );
  SetBit(bitbuffer, bitbufferlength ,  22 , true );
  SetBit(bitbuffer, bitbufferlength ,  23 , true );
  SetBit(bitbuffer, bitbufferlength ,  24 , true );
  SetBit(bitbuffer, bitbufferlength ,  25 , true );
  
  SetBit(bitbuffer, bitbufferlength ,  39 , true );
}
	
