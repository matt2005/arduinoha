#ifndef X10Protocol_h
#define X10Protocol_h
#include <ConstantLengthHighPulseProtocolBase.h>

class X10Protocol : public ConstantLengthHighPulseProtocolBase {
	public:
		X10Protocol (
			char * id,
			double TimerFrequency,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte group , byte device , bool state)
		);

		void EncodeDeviceCommand(byte group, byte device, bool state, byte *& encodedpulsestream, byte &encodedpulsestreamlength );
	private:
		void (*_DeviceTrippedEvent)(ProtocolBase * protocol, byte group, byte device , bool state);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
