
#include "QUANTA/QUANTA.hxx"

#include <stdio.h>
#include <stdlib.h>
#include "pablio.h"
#include "ringbuffer.h"

typedef short SAMPLE;

struct portNumberDatabase{
   int port;
   int status;
};// *portNo;

class vocAL{
public:
  vocAL::vocAL(int);
  int init(int);
  int openAudio( double sampleRate, PaSampleFormat format, long flags);
  int openAudio();
  int getRemotePort(char *IP);
  int connectToDirect(char *host, int port,int id);
  // void checkForNewAudience();
  void process();
  int findEmptyPortNumber();
  int samplesPerFrame;
  int framesPerBlock;  
  int connectToNewAudience(const char *remoteHost, int port);
private:
  // int vocAL::checkNewAudience(QUANTAnet_udp_c *eachChannel);
  int vocAL::paError(PaError err);
  // void connectionCheck();
  int setPortNumberDB(int);


  int *portNoDB;
  int maxOfChannel;
  int err;
  int numSamples;
  int dataSize;
  int numberOfAudiences;
  
  QUANTAnet_udp_c **channel;
  QUANTAnet_udp_c **bunchOfAudiences;
  QUANTAnet_datapack_c packer;
  
  PABLIO_Stream *aStream;	      
  
  SAMPLE **rcvSamples;
  SAMPLE *sndSamples;
  SAMPLE *playback;
  char *receiveBuffer;
  char *sendBuffer;
  portNumberDatabase *portNo;
};


