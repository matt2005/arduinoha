#include "LaCrosseProtocol2.h"
#include <hardwareserial.h>

#define HIGH 0x1
#define LOW  0x0

// [  98]   78 [  96]   81 [  93]   75 [  24]   83 [  92]   83 [  92]   84 [  23]   84 [  90]   85 [  22]   85 [  22]   85 [  90]   86 [  89]   86 [  89]   87 [  89]   86 [  20]   87 [  20]   87 [  89]   86 [  20]   87 [  89]   87 [  88]   87 [  20]   87 [  20]   87 [  19]   88 [  19]   88 [  87]   88 [  88]   88 [  87]   88 [  87]   89 [  87]   89 [  86]   89 [  86]   90 [  86]   89 [  86]   90 [  86]   89 [  86]   90 [  85]   90 [  86]   90 [  85]   91 [  85]   90 [  16]   91 [  16]   91 [  16]   91 [  16]   90 [  17]   90 [  17]   90 [  16]   91 [  16]   91 [  85]   90 [  16]   91 [  16]   91 [  16] 12174 

LaCrosseProtocol2::LaCrosseProtocol2(
	char * id,
	double TimerFrequency ,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length )
	) : ConstantLengthLowPulseProtocolBase(id,BitsstreamReceivedEvent , 50, 3, (TimerFrequency / 3472.0f) , 5.94f , 9.77f)
{	
	/*void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
	{
	}*/
}

