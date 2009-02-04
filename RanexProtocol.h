#ifndef RanexProtocol_h
#define RanexProtocol_h
#include <ConstantLengthPulseCycleProtocolBase.h>

class RanexProtocol : public ConstantLengthPulseCycleProtocolBase {
	public:
		RanexProtocol(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)( ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceCommandReceivedEvent)(ProtocolBase * protocol , byte device , bool turnoncommand)
		);
		
		void EncodeDeviceCommand(byte device, bool lighton, byte *& bitbuffer, byte &bitbufferlength );
	private:
		void (*_DeviceCommandReceivedEvent)( ProtocolBase * protocol , byte device , bool turnoncommand);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
