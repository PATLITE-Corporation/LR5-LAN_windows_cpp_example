// LR5-LAN_Sample_CPP.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <conio.h>

#pragma warning(disable:4996)
#pragma comment( lib, "ws2_32.lib" )

WSADATA wsaData;
SOCKET sock = NULL;

// product category
#define PNS_PRODUCT_ID	0x4142

// PNS command identifier
#define PNS_RUN_CONTROL_COMMAND			0x53 	// operation control command
#define PNS_CLEAR_COMMAND	    		0x43 	// clear command
#define PNS_GET_DATA_COMMAND			0x47 	// get status command

// Buffer size from product category to data size
#define PNS_COMMAND_HEADER_LENGTH		6

// response data for PNS command
#define PNS_ACK		0x06 	// normal response
#define PNS_NAK		0x15 	// abnormal response

// LED unit for motion control command
#define PNS_RUN_CONTROL_LED_OFF			0x00 	// light off
#define PNS_RUN_CONTROL_LED_ON			0x01 	// light on
#define PNS_RUN_CONTROL_LED_BLINKING_SLOW	0x02 	// blinking(slow)
#define PNS_RUN_CONTROL_LED_BLINKING_MEDIUM	0x03 	// blinking(medium)
#define PNS_RUN_CONTROL_LED_BLINKING_HIGH	0x04 	// blinking(high)
#define PNS_RUN_CONTROL_LED_FLASHING_SINGLE	0x05 	// flashing single
#define PNS_RUN_CONTROL_LED_FLASHING_DOUBLE	0x06 	// flashing double
#define PNS_RUN_CONTROL_LED_FLASHING_TRIPLE	0x07 	// flashing triple
#define PNS_RUN_CONTROL_LED_NO_CHANGE		0x09    // no change

// buzzer for motion control command
#define PNS_RUN_CONTROL_BUZZER_STOP		0x00 	// stop
#define PNS_RUN_CONTROL_BUZZER_RING		0x01 	// ring
#define PNS_RUN_CONTROL_BUZZER_NO_CHANGE	0x09 	// no change


// operation control data structure
typedef struct
{
	// LED Red pattern
    unsigned char ledRedPattern;

	// LED Amber pattern
    unsigned char ledAmberPattern;

	// LED Green pattern
    unsigned char ledGreenPattern;

	// LED Blue pattern
    unsigned char ledBluePattern;

	// LED White pattern
    unsigned char ledWhitePattern;

	// buzzer mode.
    unsigned char buzzerMode;
}PNS_RUN_CONTROL_DATA;


// status data of operation control
typedef struct
{
    //  LED unit pattern 1 to 5
    unsigned char ledPattern[5];

    // buzzer mode
    unsigned char buzzer;
}PNS_STATUS_DATA;


int SocketOpen(std::string ip, int port);
void SocketClose();
int SendCommand(char* sendData, int sendLength, char* recvData, int recvLength);
int PNS_RunControlCommand(PNS_RUN_CONTROL_DATA runControlData);
int PNS_ClearCommand();
int PNS_GetDataCommand(PNS_STATUS_DATA* statusData);

/// <summary>
/// Main Function
/// </summary>
int main(int argc, char* argv[])
{
    int ret;

    // Connect to LR5-LAN
    ret = SocketOpen("192.168.10.1", 10000);
    if (ret == -1) {
        return 0;
    }

    // Get the command identifier specified by the command line argument
    char* commandId = NULL;
    if (argc > 1) {
        commandId = argv[1];

        switch (*commandId)
        {
            case 'S':
            {
                // operation control command
                if (argc >= 8)
                {
                    PNS_RUN_CONTROL_DATA runControlData;
                    runControlData.ledRedPattern = atoi(argv[2]);
                    runControlData.ledAmberPattern = atoi(argv[3]);
                    runControlData.ledGreenPattern = atoi(argv[4]);
                    runControlData.ledBluePattern = atoi(argv[5]);
                    runControlData.ledWhitePattern = atoi(argv[6]);
                    runControlData.buzzerMode = atoi(argv[7]);
                    PNS_RunControlCommand(runControlData);
                }

                break;
            }

            case 'C':
            {
                // clear command
                PNS_ClearCommand();
                break;
            }

            case 'G':
            {
                // get status command
                PNS_STATUS_DATA statusData;
                ret = PNS_GetDataCommand(&statusData);
                if (ret == 0)
                {
                    // Display acquired data
                    printf("Response data for status acquisition command\n");
                    // LED Red pattern
                    printf("LED Red pattern : %d\n", statusData.ledPattern[0]);
                    // LED Amber pattern
                    printf("LED Amber pattern : %d\n", statusData.ledPattern[1]);
                    // LED Green pattern
                    printf("LED Green pattern : %d\n", statusData.ledPattern[2]);
                    // LED Blue pattern
                    printf("LED Blue pattern : %d\n", statusData.ledPattern[3]);
                    // LED White pattern
                    printf("LED White pattern : %d\n", statusData.ledPattern[4]);
                    // buzzer mode
                    printf("buzzer mode : %d\n", statusData.buzzer);
                }

                break;
            }
        }

    }

    // Close the socket
    SocketClose();

    return 0;
}

/// <summary>
/// Connect to LR5-LAN
/// </summary>
/// <param name="ip">IP address</param>
/// <param name="port">port number</param>
/// <returns>success: 0, failure: non-zero</returns>
int SocketOpen(std::string ip, int port)
{
	// Initialize winsock2
    if (WSAStartup(MAKEWORD(2, 0), &wsaData))
    {
        std::cout << "reset winsock failed" << std::endl;
        return -1;
    }

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "make socket failed" << std::endl;
        return -1;
    }

    // Set the IP address and port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

    // Connect to LR5-LAN
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)))
    {
        std::cout << "connect failed" << std::endl;
        return -1;
    }

    return 0;
}

/// <summary>
/// Close the socket.
/// </summary>
void SocketClose()
{
	// Close the socket.
    closesocket(sock);

	// Exit process of winsock2
    WSACleanup();
}

/// <summary>
/// Send command
/// </summary>
/// <param name="sendData">send data</param>
/// <param name="recvData">received data</param>
/// <returns>success: 0, failure: non-zero</returns>
int SendCommand(char* sendData, int sendLength, char* recvData, int recvLength)
{
    int ret;

    if (sock == NULL)
    {
        std::cout << "socket is not" << std::endl;
        return -1;
    }

	// Send
    ret = send(sock, sendData, sendLength, 0);
    if (ret < 0)
    {
        std::cout << "failed to send" << std::endl;
        return -1;
    }

	// Receive response data
    std::memset(recvData, 0, recvLength);
    ret = recv(sock, recvData, recvLength, 0);
    if (ret < 0)
    {
        std::cout << "failed to recv" << std::endl;
        return -1;
    }

    return 0;
}


/// <summary>
/// Send operation control command for PNS command
/// Each color of the LED unit and the buzzer can be controlled by the pattern specified in the data area
/// Operates with the color and buzzer set in the signal light mode
/// </summary>
/// <param name="runControlData">
/// Red/amber/green/blue/white LED unit operation patterns, buzzer mode
/// Pattern of LED unit (off: 0, on: 1, blinking(slow): 2, blinking(medium): 3, blinking(high): 4, flashing single: 5, flashing double: 6, flashing triple: 7, no change: 9)
/// Pattern of buzzer (stop: 0, ring: 1, no change: 9)
/// </param>
/// <returns>success: 0, failure: non-zero</returns>
int PNS_RunControlCommand(PNS_RUN_CONTROL_DATA runControlData)
{
    int ret;
    char sendData[PNS_COMMAND_HEADER_LENGTH + sizeof(runControlData)];
    char recvData[1];
    std::memset(sendData, 0, sizeof(sendData));
    std::memset(recvData, 0, sizeof(recvData));

    // Product Category (AB)
    sendData[0] = PNS_PRODUCT_ID >> 8;
    sendData[1] = (char)(PNS_PRODUCT_ID | 0xFF00);

    // Command identifier (S)
    sendData[2] = PNS_RUN_CONTROL_COMMAND;

    // Empty (0)
    sendData[3] = 0;

    // Data size
    sendData[4] = sizeof(runControlData) >> 8;
    sendData[5] = (char)(sizeof(runControlData) | 0xFF00);

    // Data area
    std::memcpy(&sendData[6], &runControlData, sizeof(runControlData));

    // Send PNS command
    ret = SendCommand(sendData, PNS_COMMAND_HEADER_LENGTH + sizeof(runControlData), recvData, sizeof(recvData));
    if (ret != 0)
    {
        std::cout << "failed to send data" << std::endl;
        return -1;
    }

    // check the response data
    if (recvData[0] == PNS_NAK)
    {
        // receive abnormal response
        std::cout << "negative acknowledge" << std::endl;
        return -1;
    }

    return 0;
}


/// <summary>
/// Send clear command for PNS command
/// Turn off the LED unit and stop the buzzer
/// </summary>
/// <returns>success: 0, failure: non-zero</returns>
int PNS_ClearCommand()
{
    int ret;
    char sendData[PNS_COMMAND_HEADER_LENGTH];
    char recvData[1];
    std::memset(sendData, 0, sizeof(sendData));
    std::memset(recvData, 0, sizeof(recvData));

    // Product Category (AB)
    sendData[0] = PNS_PRODUCT_ID >> 8;
    sendData[1] = (char)(PNS_PRODUCT_ID | 0xFF00);

    // Command identifier (C)
    sendData[2] = PNS_CLEAR_COMMAND;

    // Empty (0)
    sendData[3] = 0;

    // Data size
    sendData[4] = 0;
    sendData[5] = 0;

    // Send PNS command
    ret = SendCommand(sendData, PNS_COMMAND_HEADER_LENGTH, recvData, sizeof(recvData));
    if (ret != 0)
    {
        std::cout << "failed to send data" << std::endl;
        return -1;
    }

    // check the response data
    if (recvData[0] == PNS_NAK)
    {
        // receive abnormal response
        std::cout << "negative acknowledge" << std::endl;
        return -1;
    }

    return 0;
}


/// <summary>
/// Send status acquisition command for PNS command
/// LED unit and buzzer status can be acquired
/// </summary>
/// <param name="statusData">Received data of status acquisition command (status of LED unit and buzzer)</param>
/// <returns>Success: 0, failure: non-zero</returns>
int PNS_GetDataCommand(PNS_STATUS_DATA* statusData)
{
    int ret;
    char sendData[PNS_COMMAND_HEADER_LENGTH];
    char recvData[sizeof(PNS_STATUS_DATA)];
    std::memset(sendData, 0, sizeof(sendData));
    std::memset(recvData, 0, sizeof(recvData));
    std::memset(statusData, 0, sizeof(PNS_STATUS_DATA));

    // Product Category (AB)
    sendData[0] = PNS_PRODUCT_ID >> 8;
    sendData[1] = (char)(PNS_PRODUCT_ID | 0xFF00);

    // Command identifier (G)
    sendData[2] = PNS_GET_DATA_COMMAND;

    // Empty (0)
    sendData[3] = 0;

    // Data size
    sendData[4] = 0;
    sendData[5] = 0;

    // Send PNS command
    ret = SendCommand(sendData, PNS_COMMAND_HEADER_LENGTH, recvData, sizeof(recvData));
    if (ret != 0)
    {
        std::cout << "failed to send data" << std::endl;
        return -1;
    }

    // check the response data
    if (recvData[0] == PNS_NAK)
    {
        // receive abnormal response
        std::cout << "negative acknowledge" << std::endl;
        return -1;
    }

    // LED unit R pattern 1 to 5
    std::memcpy(statusData->ledPattern, &recvData[0], sizeof(statusData->ledPattern));

    // Buzzer Mode
    statusData->buzzer = recvData[5];

    return 0;
}
