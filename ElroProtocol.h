#ifndef ElroProtocol_h
#define ElroProtocol_h
#include <ConstantLengthPulseCycleProtocolBase.h>

class ElroProtocol : public ConstantLengthPulseCycleProtocolBase {
	public:
		ElroProtocol(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)( ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte group, byte device , bool turnoncommand)
		);
		
	private:
		void (*_DeviceCommandReceivedEvent)( ProtocolBase * protocol , byte group, byte device , bool turnoncommand);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
