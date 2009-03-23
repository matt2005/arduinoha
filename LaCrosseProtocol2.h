#ifndef LaCrosseProtocol2_h
#define LaCrosseProtocol2_h
#include <ConstantLengthLowPulseProtocolBase.h>

class LaCrosseProtocol2 : public ConstantLengthLowPulseProtocolBase {
	public:
		LaCrosseProtocol2(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length)
		);
	private:
	protected:
		//void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
