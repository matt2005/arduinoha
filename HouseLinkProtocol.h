#ifndef HouseLinkProtocol_h
#define HouseLinkProtocol_h
#include <ConstantLengthHighPulseProtocolBase.h>

class HouseLinkProtocol : public ConstantLengthHighPulseProtocolBase {
	public:
		HouseLinkProtocol (
			char * id,
			double TimerFrequency,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*DeviceTrippedEvent)(ProtocolBase * protocol , byte device ),
			void (*DeviceBatteryEmptyEvent)(ProtocolBase * protocol , byte device )
		);

		void EncodeDeviceCommand(byte device, bool lighton, byte *& encodedpulsestream, byte &encodedpulsestreamlength );
	private:
		void (*_DeviceTrippedEvent)(ProtocolBase * protocol, byte device );
		void (*_DeviceBatteryEmptyEvent)(ProtocolBase * protocol, byte device );
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
