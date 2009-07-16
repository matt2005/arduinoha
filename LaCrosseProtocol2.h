#ifndef LaCrosseProtocol2_h
#define LaCrosseProtocol2_h
#include <ConstantLengthLowPulseProtocolBase.h>

// value1 = 31 (8) + 32 (4) + 33 (2) + 34 (1)
// value2 = 35 + 36 + 37 + 38

class LaCrosseProtocol2 : public ConstantLengthLowPulseProtocolBase {
	public:
		LaCrosseProtocol2(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*RainReceivedEvent)(ProtocolBase * protocol, byte device , int rain),
			void (*WindReceivedEvent)(ProtocolBase * protocol, byte device , int wind)			
		);
		void EncodeTemperatureCommand(byte device, float value , byte *& bitbuffer, byte &bitbufferlength );
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
	private:
		void (*_RainReceivedEvent)(ProtocolBase * protocol, byte device , int rain);
		void (*_WindReceivedEvent)(ProtocolBase * protocol, byte device , int wind);
};

#endif
