#include "vocAL.hxx"
#include <unistd.h>

vocAL::vocAL( int max_of_channel )
{
   maxOfChannel=max_of_channel;
   portNo = new portNumberDatabase[maxOfChannel];
   channel = new QUANTAnet_udp_c* [maxOfChannel];
   bunchOfAudiences = new  QUANTAnet_udp_c* [maxOfChannel];
   rcvSamples = new SAMPLE* [maxOfChannel];
   numberOfAudiences=0;
   aStream = new PABLIO_Stream;
}

int vocAL::init(int defaultPort )
{
  int status;
 
  setPortNumberDB(defaultPort);
  

  cout<<"init"<<endl;
  samplesPerFrame=2;
  framesPerBlock=64;
  numSamples = samplesPerFrame * framesPerBlock;
  cout<<"numSamples="<<numSamples<<endl;

  playback = new SAMPLE[numSamples];
  sndSamples = new SAMPLE[numSamples];
  receiveBuffer = new char [numSamples];
  sendBuffer = new char[numSamples];
  dataSize=packer.sizeof_int()*numSamples;
  
  for(int i=0; i<maxOfChannel; i++){
    cout<<"vocal:: channel set -"<<i<<"- ";
    channel[i]=new  QUANTAnet_udp_c;
    channel[i]->enableInstrumentation();
    channel[i]->init(portNo[i].port);
    channel[i]->makeNonBlocking();
   
    bunchOfAudiences[i] = new  QUANTAnet_udp_c;
    bunchOfAudiences[i]->enableInstrumentation();
    bunchOfAudiences[i]->init();
    bunchOfAudiences[i]->makeNonBlocking();

    rcvSamples[i] = new SAMPLE[numSamples];

    cout<<" OK"<<endl;
  }
  return 0;
}

int vocAL::openAudio( double sampleRate, PaSampleFormat format, long flags )
{

   err = OpenAudioStream( &aStream, sampleRate, format,flags );
   if( err != paNoError ) {
     paError(err);
     return -1;
   }else
     return 0;
}

  
int vocAL::connectToDirect(char *host, int port, int id)
{
  int i=id-1;
  int status;
  cout<<"host="<<host<<" port="<<port<<" i="<<i<<endl;
  status=bunchOfAudiences[i]->setSendAddress(host, port);
  char dummy[numSamples];
  memset(dummy,0, numSamples);
  bunchOfAudiences[i]->enableInstrumentation();
  bunchOfAudiences[i]->init();
  bunchOfAudiences[i]->send(dummy, numSamples, QUANTAnet_udp_c::NON_BLOCKING);
  bunchOfAudiences[i]->makeNonBlocking();
  numberOfAudiences++;
  cout<<"status="<<status<<endl;
  if(!status){
    cout<<"vocAL::connectToDirect Error"<<endl;
    return -1;
  }else
    return 0;
}


void vocAL::process()
{
  //SAMPLE playback[numSamples];
  
  int rcvChk;
  memset(playback, 0, sizeof(SAMPLE)*numSamples);
  
  /*** RECEIVE AUDIO DATA***/
  for(int i = 0 ; i<maxOfChannel; i++) {
    rcvChk=channel[i]->receive(receiveBuffer, dataSize, QUANTAnet_udp_c::NON_BLOCKING);
    // cout<<"rcvChk="<<rcvChk<<endl;
    memset(rcvSamples[i], 0, sizeof(SAMPLE)*numSamples);
    if(rcvChk>0){
      packer.initUnpack(receiveBuffer,dataSize);
      packer.unpackIntArray((int *)rcvSamples[i],numSamples);
    }   
    /*** MASTER MIXER ***/
    for(int j=0; j<numSamples; j++){
      playback[j]+=(SAMPLE)(1/(float)maxOfChannel*(rcvSamples[i][j]));
    }
  } 
  WriteAudioStream(aStream, playback, framesPerBlock);
  
  /*** RECORD AUDIO DATA*** */
  memset(sndSamples, 0, sizeof(SAMPLE)*numSamples);
  ReadAudioStream( aStream, sndSamples, framesPerBlock );
  packer.initPack(sendBuffer,dataSize);
  packer.packIntArray((int *)sndSamples,numSamples);
  int realSize=packer.getBufferFilledSize();
  /*** SEND AUDIO DATA***/
  for(int i = 0 ; i<numberOfAudiences; i++) {
    bunchOfAudiences[i]->send(sendBuffer, realSize, QUANTAnet_udp_c::NON_BLOCKING);
    //channel[i]->send(sendBuffer, realSize, QUANTAnet_udp_c::NON_BLOCKING);
  }
}

int vocAL::connectToNewAudience(const char *remoteHost, int port)
{
  bunchOfAudiences[numberOfAudiences]->setSendAddress(remoteHost, port);
  numberOfAudiences++;
  
  return 1;
}


int vocAL::setPortNumberDB(int defaultPort)
{
 
  for(int i=0; i<maxOfChannel; i++){
    portNo[i].status=0;
    portNo[i].port=defaultPort+i;
    printf("vocal:: established %d portDB %d\n",i, portNo[i].port);
  }
  return 1;
}

int vocAL::findEmptyPortNumber()
{
  cout<<"findEmptyPortNumber"<<endl;
  for(int i=0; i<maxOfChannel; i++){
    if(portNo[i].status==0){
      portNo[i].status=1;
      return portNo[i].port;
    }
  }
  return -1;
}


// void vocAL::connectionCheck()
// {
//   int status;
//   aClient = server->checkForNewConnections();
//   if (aClient) {
//     printf("^GClient %d connected\n", numberOfClients); fflush(stdout);  
//     char name[256];
//     aClient->getSelfIP(name);
//     printf(">>> %s,%u\n", name, aClient->getSelfPort());
//     client[numberOfClients] = aClient;
//     numberOfClients++;
    
//     dataSize = BROADCAST_DATA_SIZE;

//     char *emprtPortNumber;
//     sprintf(emprtPortNumber,"%s",findEmptyPortNumber());
//     status = client[numberOfClients]->write(emptyPortNumber,&dataSize,QUANTAnet_tcpClient_c::BLOCKING);
//  }
// }

int vocAL::paError(PaError err)
{

  Pa_Terminate();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  return -1;
}

