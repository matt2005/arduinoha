#ifndef HouseLinkProtocol_h
#define HouseLinkProtocol_h
#include <ConstantLengthHighPulseProtocolBase.h>

class HouseLinkProtocol : public ConstantLengthHighPulseProtocolBase {
	public:
		HouseLinkProtocol (
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
