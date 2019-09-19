#include "HybridP2Ptcp.h"
#include <stdlib.h>
#include <iostream.h>

const int HybridP2Ptcp::OK=1;
const int HybridP2Ptcp::FAILED=-1;
const int HybridP2Ptcp::HAVE_ANY_DATA = 2;
const int HybridP2Ptcp::MEM_ALLOC_ERR=3;
const int HybridP2Ptcp::NEW_CONNECTION_ESTABLISHED = 4;
const int HybridP2Ptcp::TOO_MANY_CLIENTS = 5;
const int HybridP2Ptcp::NO_NEW_CONNECTION= 6;
const int HybridP2Ptcp::CONNECTION_TERMINATED=8;

HybridP2Ptcp::HybridP2Ptcp()
{
}

HybridP2Ptcp::~HybridP2Ptcp()
{
  if (perfDaemonIP){
    exitSendStats();
    delete perfDaemonIP;
  }
  closeDownSockets();
}

int HybridP2Ptcp::init(int incomingPort, int maxPeers)
{
  maxOfPeers=maxPeers;
  fowordPort=incomingPort;
  alreadyExistedPeers=0;
  //As Server Process Initialization
  asServer = new QUANTAnet_tcpServer_c;
  if (asServer->init(incomingPort) == 0)    {
    printf("Can not establish a server process\n");
    return  HybridP2Ptcp::FAILED;
  }
  
  //As Client Process Initialization
  tcppeer = new QUANTAnet_tcpClient_c*[maxOfPeers];
  if(tcppeer==NULL)
    return  HybridP2Ptcp::MEM_ALLOC_ERR;
  for(int i = 0; i < maxOfPeers; i++)
    tcppeer[i] = NULL;
  
  return  HybridP2Ptcp::OK;
}

int HybridP2Ptcp::connectToSeed(char* seedIP)
{
    //int maxOfPeers=10;
    int dataSize=256;
    char *peersNo = new char[32];
    toSeed = new QUANTAnet_tcpClient_c;
    if (toSeed->connectToServer(seedIP, 6544) < 0){
        printf("connect error\n");
        return HybridP2Ptcp::FAILED;
    }

    toSeed->read(peersNo, &dataSize, QUANTAnet_tcpClient_c::BLOCKING);

    alreadyExistedPeers=atoi(peersNo);

    printf("alreadyExistedPeers=%d\n", alreadyExistedPeers);
    if(alreadyExistedPeers>=maxOfPeers){
        printf("Too many Peers\n");
        return  HybridP2Ptcp::FAILED;
    }

    if(alreadyExistedPeers>0){
        for(int i=0; i<maxOfPeers; i++){
            toSeed->read(peersIP[i], &dataSize, QUANTAnet_tcpClient_c::BLOCKING);
            printf("PeersIP %d %s\n", i, peersIP[i]);
        }
    }

    //client->close();
    //   delete client;
    return HybridP2Ptcp::OK;
}

void HybridP2Ptcp::connectToPeers()
{

    QUANTAnet_tcpClient_c *aClient = new QUANTAnet_tcpClient_c[maxOfPeers];

    if(aClient==NULL)
        //HybridP2Ptcp::FAILED;

    for(int i=0; i<maxOfPeers; i++){
        printf("PeersID::%d IP::%s\n", i, peersIP[i]);

        printf(" strlen=%d\n",strlen(peersIP[i]));
        if(strlen(peersIP[i])>0)
            if(aClient[i].connectToServer(peersIP[i], fowordPort) <0){
                printf("Can not connect to %s:%d\n",peersIP[i],fowordPort);
                aClient=NULL;
            }else
                int status = addNewPeer(&aClient[i]);
    }
    //HybridP2Ptcp::OK;
}

int HybridP2Ptcp::addNewPeer(QUANTAnet_tcpClient_c *newPeer)
{
    if(newPeer){
        for(int i=0; i<maxOfPeers; i++){
	  if(tcppeer[i]==NULL){
                tcppeer[i]=newPeer;
                return HybridP2Ptcp::NEW_CONNECTION_ESTABLISHED;
            }
        }
        return HybridP2Ptcp::TOO_MANY_CLIENTS;
    }
    return HybridP2Ptcp::NO_NEW_CONNECTION;
}


int HybridP2Ptcp::checkForNewPeers()
{
    QUANTAnet_tcpClient_c *newPeer;
    newPeer = asServer->checkForNewConnections();
    int i=0;
    if(newPeer){
      printf("\nNew peer %d connected\n", alreadyExistedPeers);
      fflush(stdout);
      char name[256];
      newPeer->getRemoteIP(name);
      printf(">>> %s:%u\n", name, newPeer->getRemotePort());
    
      while(1){
	if(!tcppeer[i]){
	  tcppeer[i] = newPeer;
	  tcppeer[i]->getRemoteIP(PeersIP[i]);
	  cout<<i<<" IP::"<<PeersIP[i]<<endl;

	  printf("New peer add to No. %d\n\n", i);
	  NewPeerNo=i;
	  break;
	}else
	  i++;
      }
      alreadyExistedPeers++;
      return alreadyExistedPeers;
    }
    return alreadyExistedPeers;
}

/****************  READ ******************************************/
int HybridP2Ptcp::read(char *ret[], int *readSize, int i, int blockingType)
{
    int status;
    int incomingSize;
    int dataSize=16;
    char data[32];

    //static int maxNumClients = maxOfPeers;
    /******** Additional be able to loop without client say nothing *******/
    int* socketStatus = new int[maxOfPeers];
	
    QUANTAnet_tcpClient_c** thePeer;
    thePeer = new QUANTAnet_tcpClient_c*[maxOfPeers];
	
    for (int count=0; count<maxOfPeers; count++) {
      if (tcppeer[count])
	thePeer[count] = tcppeer[count];//->isReady();//getPeerPtr();
      else
	thePeer[count] = NULL;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;
    
    QUANTAnet_socketbase_c someSocketPtr;
    int retval =  someSocketPtr.selectSock(thePeer, socketStatus, maxOfPeers, &timeout);
    
    if (retval != 1)  {
      delete thePeer;
      delete socketStatus;
      return HybridP2Ptcp::FAILED;
    }
    
    /*****************************************************************/

    
    if(thePeer[i]!=NULL && (socketStatus[i] == 1)  ) {  
      
      if(tcppeer[i]->read((char *)data, &dataSize, QUANTAnet_tcpClient_c::BLOCKING)==QUANTAnet_tcpClient_c::OK){
	//cout<<"Hi:"<<i<<endl;
	incomingSize=atoi(data);

	if(checkBuf != NULL){
          delete checkBuf;
	  checkBuf = NULL;
        }
	char *buffer =  new char[incomingSize];
        checkBuf = buffer;
	
	status=tcppeer[i]->read((char *)buffer, &incomingSize, QUANTAnet_tcpClient_c::BLOCKING);
	if(status==QUANTAnet_tcpClient_c::OK){
	  No=i;
	  *readSize = incomingSize;
	  *ret = buffer;
	  return HybridP2Ptcp::HAVE_ANY_DATA;
	}else{
	  *readSize = 0;
	  *ret = NULL;
	  if (status == QUANTAnet_tcpClient_c::CONNECTION_TERMINATED)
	    return HybridP2Ptcp::CONNECTION_TERMINATED;
	  else
	    return HybridP2Ptcp::FAILED;
	}
      }
      else{
	removeClient(i);
	return HybridP2Ptcp::CONNECTION_TERMINATED;
      }
    }
    return HybridP2Ptcp::OK;
}

int HybridP2Ptcp::write(char *ptr, int nbytes)
{

    int status;
    char buf[16];
    int bufSize=16;

    for(int i=0; i<maxOfPeers-1; i++){
      sprintf(buf, "%d", nbytes);
      if(tcppeer[i]){
	status = tcppeer[i]->write(buf, &bufSize, QUANTAnet_tcpClient_c::BLOCKING);
	if(status==QUANTAnet_tcpClient_c::OK){
	  status =tcppeer[i]&& tcppeer[i]->write(ptr, &nbytes, QUANTAnet_tcpClient_c::BLOCKING);
	  if (status != QUANTAnet_tcpClient_c::OK) {
	    printf("write error\n");
	    return HybridP2Ptcp::CONNECTION_TERMINATED;
	  }
	}else{
	  printf("Can not send data size\n");
	  removeClient(i);
	}
      }
      
    }
    return HybridP2Ptcp::OK;
}


void HybridP2Ptcp::removeClient(int i)
{
  printf("remove::%d\n", tcppeer[i]);
  if (tcppeer[i]){
    tcppeer[i]->exitSendStats();
    tcppeer[i] = NULL;
    alreadyExistedPeers--;
  }
  printf("alreadyExistedPeers %d\n",alreadyExistedPeers);
}

void HybridP2Ptcp::exitSendStats()
{
  int i;
  //printf("---------15--------\n");
  for (i = 0; i < alreadyExistedPeers; i++) {
    if (tcppeer[i])
      tcppeer[i]->exitSendStats();
  }
}

void HybridP2Ptcp::closeDownSockets()
{
  int i;
  
    // Iterate thru list of clients and close them down
  for (i = 0; i < alreadyExistedPeers; i++){
    if (tcppeer[i]) {
      tcppeer[i]->close();
      delete tcppeer[i];
    }
  }
  delete tcppeer;
  //server->close();
  //delete server;
}


int  HybridP2Ptcp::connectToDirect(char *IP ,int i, int port)
{
  QUANTAnet_tcpClient_c *client = new QUANTAnet_tcpClient_c ;
  
  if(client->connectToServer(IP, port) <0){
    printf("Ooops\n");
    return -1;
  }else
    tcppeer[i-1]=client;
  printf("------ %d::%d\n",tcppeer[i-1],port);
  alreadyExistedPeers=++countcount;//i;
  
  return  HybridP2Ptcp::OK; 
}
int HybridP2Ptcp::writeToDirect(char *ptr, int nbytes, int PeerNo)
{
  int status;
  char buf[16];
  int bufSize=16;
  
  sprintf(buf, "%d", nbytes);
  cout<<PeerNo<<endl;
  if(tcppeer[PeerNo]){
    status = tcppeer[PeerNo]->write(buf, &bufSize, QUANTAnet_tcpClient_c::BLOCKING);
    if(status==QUANTAnet_tcpClient_c::OK){
      status =tcppeer[PeerNo]&& tcppeer[PeerNo]->write(ptr, &nbytes, QUANTAnet_tcpClient_c::BLOCKING);
      if (status != QUANTAnet_tcpClient_c::OK) {
	printf("write error\n");
	return HybridP2Ptcp::CONNECTION_TERMINATED;
      }
    }else{
      printf("Can not send data size\n");
      removeClient(PeerNo);
    }
  }
  return HybridP2Ptcp::OK;
}


char * HybridP2Ptcp::getRemoteIP(int i)
{
  //char ip[20];
  tcppeer[i]->getRemoteIP(ip);
  cout<<"IP>>"<<ip<<endl;
  return ip;
}
