#ifndef _HybridP2Ptcp_h_
#define _HybridP2Ptcp_h_

#include "QUANTA/QUANTAnet_tcp_c.hxx"
#include <stdlib.h>
#include <string.h>
#ifdef IRIX
#include <time.h>
#include <utmpx.h>
#endif

//#include <iostream.h>

class HybridP2Ptcp
{
  char numOfPeers[32];
  char peersIP[256][256];
  int fowordPort;
  
 public:
  static const int OK;
  static const int FAILED;
  static const int HAVE_ANY_DATA;
  static const int MEM_ALLOC_ERR;
  static const int NEW_CONNECTION_ESTABLISHED;
  static const int TOO_MANY_CLIENTS;
  static const int NO_NEW_CONNECTION;
  static const int CONNECTION_TERMINATED;
  
  int alreadyExistedPeers;
  int No;
  int NewPeerNo;
  int maxOfPeers;

  //**************************
  QUANTAnet_tcpServer_c *asServer;
  //HybridP2Ptcp  **peer;
  QUANTAnet_tcpClient_c **tcppeer;
  QUANTAnet_tcpClient_c *toSeed;
  HybridP2Ptcp();
  ~HybridP2Ptcp();
 
 int  init(int incomingPort, int maxPeers);
  int  connectToSeed(char* seedIP);
  void connectToPeers();
  int  checkForNewPeers();
  void removeClient(int i);
 
  int  write(char *ptr, int nbytes);
  int  writeToDirect(char *ptr, int nbytes, int PeerNo);
  int  read(char *ret[], int *readSize,  int i, int blockingType);
  void exitSendStats();
  void closeDownSockets();
  int  connectToDirect(char *IP, int, int port);
  char *getRemoteIP(int);

  //QUANTAnet_tcpClient_c* getClientPtr();
  //QUANTAnet_tcpClient_c *getPeerPtr();

  char PeersIP[10][256];

 protected:
  //HybridP2Ptcp **peer;
  // QUANTAnet_tcpClient_c *client;

  QUANTAnet_tcpClient_c *client;

  int addNewPeer(QUANTAnet_tcpClient_c *newPeer);
  char *perfDaemonIP;
  char ip[20];
  //QUANTAnet_tcpClient_c *getPeerPtr();
  //
 private:
  int countcount;
  char* checkBuf;

};
#endif
