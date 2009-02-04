#ifndef OpelCarkeyProtocol_h
#define OpelCarkeyProtocol_h
#include <ManchesterCodeProtocolBase.h>

class OpelCarkeyProtocol : public ManchesterCodeProtocolBase {
	public:
		OpelCarkeyProtocol(
			char * id,
			double TimerFrequency ,
			void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length),
			void (*LockCommandReceivedEvent)(ProtocolBase * protocol, bool lockcommand),
			void (*TwoButtonsPressedReceivedEvent)(ProtocolBase * protocol)
		);
	private:
		void (*_LockCommandReceivedEvent)(ProtocolBase * protocol , bool lockcommand);
		void (*_TwoButtonsPressedReceivedEvent)(ProtocolBase * protocol);
	protected:
		void DecodeBitstream(unsigned int lasthigh, unsigned int lastlow);
};

#endif
