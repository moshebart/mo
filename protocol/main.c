#include "protocol.h"
//**************************************************************************************************
//This program gets an arguments and create a protocol according to the argument:
//1. byte generator -> transport -> byte physical -> byte physical -> transport -> byte printer.
//2. byte generator -> transport -> packet physical -> packet physical -> transport -> byte printer.
//3. byte generator -> transport -> byte physical -> packet physical -> transport -> byte printer.
//4. byte generator -> transport -> packet physical -> byte physical -> transport -> byte printer.
//**************************************************************************************************
int main(int argc, char* argv[]) {
	pthread_t _physical0, _physical1;
	BYTES _bytes0, _bytes1, applicationLayerByteGeneratorBytes;
	if (argc < 2) {
		printf("Please enter an argument\n1 for byte physical -> byte physical\n2 for packet physical -> packet physical\n3 for byte physical -> packet physical\n4 for packet physical -> byte physical\n");
		exit(1);
	}
	switch (*argv[1]) {
	//byte physical -> byte physical
	case '1':
		protocolInit(&_bytes0, 0, PHYSICAL_SEND_TO_BYTE_PHYSICAL, DEFAULT);
		pthread_create(&_physical0, NULL, physicalByte, (void*) &_bytes0);
		protocolInit(&_bytes1, 1, PHYSICAL_GET_FROM_BYTE_PHYSICAL,
				TRANSPORT_GET_FROM_BYTE_PHISICAL);
		pthread_create(&_physical1, NULL, physicalByte, (void*) &_bytes1);
		break;
		//packet physical -> packet physical
	case '2':

		protocolInit(&_bytes0, 0, PHYSICAL_SEND_TO_PACKET_PHYSICAL, DEFAULT);
		pthread_create(&_physical0, NULL, physicalPacket, (void*) &_bytes0);
		protocolInit(&_bytes1, 1, PHYSICAL_GET_FROM_PACKET_PHYSICAL,
				TRANSPORT_GET_FROM_PACKET_PHISICAL);
		pthread_create(&_physical1, NULL, physicalPacket, (void*) &_bytes1);
		break;
		//byte physical -> packet physical
	case '3':
		protocolInit(&_bytes0, 0, PHYSICAL_SEND_TO_PACKET_PHYSICAL, DEFAULT);
		pthread_create(&_physical0, NULL, physicalByte, (void*) &_bytes0);
		protocolInit(&_bytes1, 1, PHYSICAL_GET_FROM_PACKET_PHYSICAL,
				TRANSPORT_GET_FROM_PACKET_PHISICAL);
		pthread_create(&_physical1, NULL, physicalPacket, (void*) &_bytes1);
		break;
		//packet physical -> byte physical
	case '4':
	default:
		protocolInit(&_bytes0, 0, PHYSICAL_SEND_TO_BYTE_PHYSICAL,
				TRANSPORT_GET_FROM_BYTE_PHISICAL);
		pthread_create(&_physical0, NULL, physicalPacket, (void*) &_bytes0);

		break;
	}

	initApplicationLayerByteGenerator(&applicationLayerByteGeneratorBytes,
			TRANSPORT_GET_FROM_BYTE_GENERATOR);
	applicationLayerByteGenerator(&applicationLayerByteGeneratorBytes);

	pthread_join(_physical0, NULL);
	if (*argv[1] != '4') {
		pthread_join(_physical1, NULL);
	}

	return 0;
}
