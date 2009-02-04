#ifndef ProtocolCommand_h
#define ProtocolCommand_h
#include <ProtocolBase.h>

struct protocolcommand {
  unsigned long scheduledtime; // The time the command needs to be send
  ProtocolBase* protocol; // The protocol which will be used to send
  byte *  bitbuffer; // The bitstream which will be send
  byte bitbufferlength; // The length of the bitstream-buffer
};

#endif
