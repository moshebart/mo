#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include<stdio.h>
#include <stdlib.h>
# include <stdio.h>
# include <pthread.h>
# include <time.h>
# include <pthread.h>

#include "Enums.h"
struct bytes {
	int size, mode;
	char* byteArr;
	PHYSICAL_STATE statePhy;
	TRANSPORT_GET_FROM_STATE transportGetState;
};

typedef struct bytes BYTES;

//************application Layer****************
void applicationLayerByteGenerator();
//****************transport Layer************
void transportLayer(BYTES *_bytes);
void sendTobytePrinterLayer(BYTES *_bytes);
void transportToPhysicalLayer(BYTES *_bytes);
//************physical Layer****************
//void physicalByteLayer(char* byte, GET_FROM_STATE state);
//void physicalPacketLayer(char* byte, GET_FROM_STATE state);

void *physicalByte(BYTES *_bytes);
void *physicalPacket(BYTES *_bytes);
//************application Layer****************
void applicationLayerPrinter(char byte);
//************protocol Init****************
void protocolInit(BYTES *_bytes, int mode, PHYSICAL_STATE _statePhy,
		TRANSPORT_GET_FROM_STATE transportGetState);
void initPhysicalLayer(BYTES *_bytes, int mode, PHYSICAL_STATE _statePhy);
void initTransportLayer(BYTES *_bytes,
		TRANSPORT_GET_FROM_STATE transportGetState);
void initApplicationLayerByteGenerator(
		BYTES *applicationLayerByteGeneratorBytes,
		TRANSPORT_GET_FROM_STATE transportGetState);
void initGlobalBuffers();
#endif
