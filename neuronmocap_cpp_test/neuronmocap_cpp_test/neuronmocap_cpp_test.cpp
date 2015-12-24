#include <stdint.h>
#include <iostream>
#include <Windows.h>
#include <NeuronDataReader.h>
#include <conio.h>
#include <assert.h>

#pragma pack(push, 1)
typedef struct DataBlock_DR{
	float x;
	float y;
	float z;
	float alpha;
	float beta;
	float gamma;
} DataBlock_DR;

typedef struct DataBlock_R{
	float alpha;
	float beta;
	float gamma;
} DataBlock_R;
#pragma pack(pop)

typedef DataBlock_DR RefPosition;

void CALLBACK Callback_socketStatusChanged(void* customedObj, SOCKET_REF sender, SocketStatus status, char* message)
{
	switch (status){
	case CS_Running:
		std::cerr << "CS_Running" << std::endl;
		break;
	case CS_Starting:
		std::cerr << "CS_Starting" << std::endl;
		break;
	case CS_OffWork:
		std::cerr << "CS_OffWork" << std::endl;
		break;
	default:
		std::cerr << "Undefined Socket Status" << std::endl;
	}
	if (message){
		std::cerr << "Message : " << message << std::endl;
	}
}

std::ostream& operator <<(std::ostream& os, const DATA_VER& ver){
	os << "Data Version : " << static_cast<int>(ver.BuildNumb) << "."
		<< static_cast<int>(ver.Revision) << "."
		<< static_cast<int>(ver.Major) << "."
		<< static_cast<int>(ver.Minor);
	return os;
}

void CALLBACK Callback_frameDataReceived(void * customedObj, SOCKET_REF sender, BvhDataHeaderEx* header, float* data)
{
	static int count = 0;
	if (count == 0){
		std::cerr << header->DataVersion << std::endl;
	}

	UINT32 data_count = header->DataCount;
	BOOL with_displacement = header->WithDisp;
	BOOL with_reference = header->WithReference;

	RefPosition refPos;

	const float* pData = data;
	if (with_reference){
		::memcpy(&refPos, pData, sizeof(RefPosition));
		pData += 6;
		data_count -= 6;	//subtract size of Reference Positionn
	}

	//printf("%6u\r", data_count); fflush(stdout);
	if (with_displacement){
		UINT32 num_blocks = data_count / 6;
		const DataBlock_DR* pBlock = reinterpret_cast<const DataBlock_DR*>(pData);
		for (int i = 1; i <= num_blocks; i++, pBlock++){
			if (i == 15){
				fprintf(stderr, "%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f\r",
					pBlock->x, pBlock->y, pBlock->z,
					pBlock->alpha, pBlock->beta, pBlock->gamma);
			}
		}
	}
	else{
		UINT32 num_blocks = data_count / 3;
		const DataBlock_R* pBlock = reinterpret_cast<const DataBlock_R*>(pData);
		for (int i = 1; i <= num_blocks; i++, pBlock++){
			if (i == 15){
				fprintf(stderr, "%6.2f %6.2f %6.2f\r",
					pBlock->alpha, pBlock->beta, pBlock->gamma);
			}
		}
	}
	//printf("%6d\r", count); fflush(stdout);

	count++;
}

int main(int argc, char** argv)
{
	BRRegisterSocketStatusCallback(NULL, Callback_socketStatusChanged);
	BRRegisterFrameDataCallback(NULL, Callback_frameDataReceived);

	std::cerr << "Press \'q\' to quit." << std::endl;

	SOCKET_REF sock = BRConnectTo("127.0.0.1", 7001);

	if (!sock){
		std::cerr << "Socket connect error" << std::endl;
		return 0;
	}

	SocketStatus status = BRGetSocketStatus(sock);

	while (true)
	{
		if (_kbhit()){
			int key = getch();
			if (key == 'q')
				break;
		}
		::Sleep(50);
	}

	BRCloseSocket(sock);
	return 0;
}

