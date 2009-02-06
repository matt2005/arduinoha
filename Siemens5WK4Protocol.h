#ifndef Siemens5WK4Protocol_h
#define Siemens5WK4Protocol_h
#include <ManchesterCodeProtocolBase.h>

class Siemens5WK4Protocol : public ManchesterCodeProtocolBase {
	public:
		Siemens5WK4Protocol(
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
