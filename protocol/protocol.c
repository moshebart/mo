#include "protocol.h"

# define BufferSize 10
int bufferIndexPhyToPhy = 0;
int bufferIndexTransToPhy = 0;
char *BUFFERPtP, *BUFFERTtP;

pthread_cond_t BufferPtP_Not_Full = PTHREAD_COND_INITIALIZER;
pthread_cond_t BufferPtP_Not_Empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t BufferTtP_Not_Full = PTHREAD_COND_INITIALIZER;
pthread_cond_t BufferTtP_Not_Empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t tranprotPhysical = PTHREAD_MUTEX_INITIALIZER,
		physicalM = PTHREAD_MUTEX_INITIALIZER;

//***************initializion***************
void protocolInit(BYTES *_bytes, int mode, PHYSICAL_STATE _statePhy,
		TRANSPORT_GET_FROM_STATE transportGetState) {

	initPhysicalLayer(_bytes, mode, _statePhy);
	initTransportLayer(_bytes, transportGetState);
	initGlobalBuffers();
}

void initPhysicalLayer(BYTES *_bytes, int mode, PHYSICAL_STATE _statePhy) {
	_bytes->mode = mode;
	_bytes->statePhy = _statePhy;
	_bytes->byteArr = (char *) malloc(BufferSize * sizeof(char));
	_bytes->size = 0;
}
void initTransportLayer(BYTES *_bytes,
		TRANSPORT_GET_FROM_STATE transportGetState) {
	_bytes->transportGetState = transportGetState;
}
void initApplicationLayerByteGenerator(
		BYTES *applicationLayerByteGeneratorBytes,
		TRANSPORT_GET_FROM_STATE transportGetState) {

	applicationLayerByteGeneratorBytes->transportGetState = transportGetState;
}
void initGlobalBuffers() {
	BUFFERPtP = (char *) malloc(sizeof(char) * BufferSize);

	BUFFERTtP = (char *) malloc(sizeof(char) * BufferSize);
}
//***************byteGenerator Layer***************
void applicationLayerByteGenerator(BYTES *_bytes) {

	int i;
	size_t rnd;

	while (1) {

		usleep(400 * 1000);

		rnd = rand() % 8;
		_bytes->size = rnd;

		if (rnd > 0) {
			_bytes->byteArr = (char*) malloc((rnd) * sizeof(char));
			for (i = 0; i < rnd; i++) {
				_bytes->byteArr[i] = rand();
			}

		} else {
			_bytes->byteArr = NULL;
		}


		transportLayer(_bytes);//send them out to a transport layer.

		if (rnd > 0) {
			free(_bytes->byteArr);
		}

	}

}
//***************transport Layer***************
void transportLayer(BYTES *_bytes) {
	int i = 0;
	switch (_bytes->transportGetState) {
	case TRANSPORT_GET_FROM_BYTE_PHISICAL:

		applicationLayerPrinter(_bytes->byteArr[0]);

		break;
	case TRANSPORT_GET_FROM_PACKET_PHISICAL:
		_bytes->size--;
		for (i = 0; i < _bytes->size; i++) {
			applicationLayerPrinter(_bytes->byteArr[i]);
		}
		_bytes->size = 0;
		break;
	case TRANSPORT_GET_FROM_BYTE_GENERATOR:
		transportToPhysicalLayer(_bytes);
		break;
	default:
		break;

	}

}

void transportToPhysicalLayer(BYTES *_bytes) {

	size_t rnd = _bytes->size;

	while (--rnd != -1) {

		pthread_mutex_lock(&tranprotPhysical);
		if (bufferIndexTransToPhy == BufferSize) {
			pthread_cond_wait(&BufferTtP_Not_Full, &tranprotPhysical);
		}

		BUFFERTtP[++bufferIndexTransToPhy] = _bytes->byteArr[rnd];

		pthread_mutex_unlock(&tranprotPhysical);
		pthread_cond_signal(&BufferTtP_Not_Empty);

	}


}
//***************Physical Layer*********************************
void *physicalByte(BYTES *_bytes) {
	int i = 0;

	if (_bytes->mode == 0) {

		for (;;) {

			pthread_mutex_lock(&physicalM);
			if (bufferIndexPhyToPhy == BufferSize) {

				pthread_cond_wait(&BufferPtP_Not_Full, &physicalM);
			}
			//wait to transport//////////////////////
			pthread_mutex_lock(&tranprotPhysical);
			if (bufferIndexTransToPhy == -1) {

				pthread_cond_wait(&BufferTtP_Not_Empty, &tranprotPhysical);
			}

			BUFFERPtP[bufferIndexPhyToPhy++]
					= BUFFERTtP[bufferIndexTransToPhy--];

			pthread_mutex_unlock(&tranprotPhysical);
			pthread_cond_signal(&BufferTtP_Not_Full);
			//////////////////////////////////////////

			pthread_mutex_unlock(&physicalM);
			pthread_cond_signal(&BufferPtP_Not_Empty);

		}
	} else {

		switch (_bytes->statePhy) {
		case PHYSICAL_GET_FROM_BYTE_PHYSICAL:
			for (;;) {
				pthread_mutex_lock(&physicalM);
				if (bufferIndexPhyToPhy == -1) {

					pthread_cond_wait(&BufferPtP_Not_Empty, &physicalM);
				}


				_bytes->byteArr[0] = BUFFERPtP[bufferIndexPhyToPhy--];
				transportLayer(_bytes);

				pthread_mutex_unlock(&physicalM);
				pthread_cond_signal(&BufferPtP_Not_Full);
			}

			break;
		case PHYSICAL_GET_FROM_PACKET_PHYSICAL:
			_bytes->size--;

			for (i = 0; i < _bytes->size - 1; i++) {
				_bytes->byteArr[0] = _bytes->byteArr[i];
				transportLayer(_bytes);
			}
			_bytes->size = 0;

			break;
		}
		return 0;

	}
	free(_bytes->byteArr);
	free(_bytes);
	return 0;
}
void *physicalPacket(BYTES *_bytes) {

	time_t lastByte = time(NULL);


	if (_bytes->mode == 0) {

		switch (_bytes->statePhy) {
		case PHYSICAL_SEND_TO_BYTE_PHYSICAL:
			_bytes->mode = 1;
			_bytes->statePhy = PHYSICAL_GET_FROM_PACKET_PHYSICAL;
			for (;;) {

				//wait to transport///////////////////////////
				pthread_mutex_lock(&tranprotPhysical);
				if (bufferIndexTransToPhy == -1) {

					pthread_cond_wait(&BufferTtP_Not_Empty, &tranprotPhysical);
				}
				if (time(NULL) - lastByte > 1) {
					physicalByte(_bytes);
				}

				_bytes->byteArr[_bytes->size++]
						= BUFFERTtP[bufferIndexTransToPhy--];
				lastByte = time(NULL);
				if (_bytes->size == 10) {
					physicalByte(_bytes);
				}
				pthread_mutex_unlock(&tranprotPhysical);
				pthread_cond_signal(&BufferTtP_Not_Full);
				/////////////////////////////////////////////////

			}
			break;
		case PHYSICAL_SEND_TO_PACKET_PHYSICAL:

			for (;;) {

				pthread_mutex_lock(&physicalM);
				if (bufferIndexPhyToPhy == BufferSize) {

					pthread_cond_wait(&BufferPtP_Not_Full, &physicalM);
				}
				///////////wait to transport///////////////////////////
				pthread_mutex_lock(&tranprotPhysical);
				if (bufferIndexTransToPhy == -1) {

					pthread_cond_wait(&BufferTtP_Not_Empty, &tranprotPhysical);
				}

				while (!(time(NULL) - lastByte > 1 || _bytes->size == 10
						|| bufferIndexPhyToPhy > 9 || bufferIndexTransToPhy < 0)) {

					BUFFERPtP[bufferIndexPhyToPhy++]
							= BUFFERTtP[bufferIndexTransToPhy--];
					lastByte = time(NULL);

				}

				pthread_mutex_unlock(&tranprotPhysical);
				pthread_cond_signal(&BufferTtP_Not_Full);
				/////////////////////////////////////////////////////

				pthread_mutex_unlock(&physicalM);
				pthread_cond_signal(&BufferPtP_Not_Empty);

			}
			break;
		}
	} else {

		for (;;) {
			pthread_mutex_lock(&physicalM);
			if (bufferIndexPhyToPhy == -1) {

				pthread_cond_wait(&BufferPtP_Not_Empty, &physicalM);
			}

			if (time(NULL) - lastByte > 1) {
				transportLayer(_bytes);
			}

			while (bufferIndexPhyToPhy > 0) {//taking all the packet
				_bytes->byteArr[_bytes->size++]
						= BUFFERPtP[bufferIndexPhyToPhy--];

				lastByte = time(NULL);
				if (_bytes->size == 10) {



					transportLayer(_bytes);
				}
			}


			pthread_mutex_unlock(&physicalM);
			pthread_cond_signal(&BufferPtP_Not_Full);

		}
	}
	free(_bytes->byteArr);
	free(_bytes);

	return 0;
}

//**************byte Printer Layer****************
void applicationLayerPrinter(char byte) {

	printf("%c \n", byte);

}

