#include <thread>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include "Utilities.hpp"

#pragma comment(lib, "ws2_32.lib")

#define localhost "127.0.0.1"
#define port 8080
#define bufferSize 200

void writer()
{
    TaskHandle taskHandle = NULL;
    float64 voltage = 0.0;

    // Create the task
    DAQmxCreateTask("Writer", &taskHandle);

    // Create the analog output voltage channel
    DAQmxCreateAOVoltageChan(taskHandle, "Dev1/ao0", "", -5.0, 5.0, DAQmx_Val_Volts, "");

    // Start the task
    DAQmxStartTask(taskHandle);

    while (true)
    {
        // Set the voltage to 4.0V
        // Write the voltage to the AO port
        DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 4.0, NULL);


        const int timer_duration_ms = 50;
        // Get the current time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Loop until the timer duration is reached
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < timer_duration_ms) {
            continue;
        }
        // Set the voltage back to 0.0V
        // Write the voltage to the AO port
        DAQmxWriteAnalogScalarF64(taskHandle, true, 5.0, 0.0, NULL);

        start_time = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < timer_duration_ms) {
            continue;
        }
    }
}

void reader()
{
    const int size = 5;
    int counter = 0;
    TaskHandle taskHandle = NULL;
    TaskHandle output = NULL;
    float64 data[size];
    int32 read;
    bool hasChanged = false;
    float64 curr = 0;

    // Create a new task
    DAQmxCreateTask("Reader", &taskHandle);
    DAQmxCreateTask("ouput", &output);

    // Add an analog input channel to the task
    DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai11", "", DAQmx_Val_Cfg_Default, -5.0, 5.0, DAQmx_Val_Volts, NULL);
    DAQmxCreateAOVoltageChan(output, "Dev1/ao1", "", -5.0, 5.0, DAQmx_Val_Volts, "");

    // Start the task
    DAQmxStartTask(taskHandle);
    DAQmxStartTask(output);

    ConcreteObserver* obs = new ConcreteObserver(output, 10 ,30);
    Subject sub;
    sub.attach(obs);

    // Read a single voltage value from the channel
    auto start_time = std::chrono::high_resolution_clock::now();
    while (true) {
        DAQmxReadAnalogF64(taskHandle, size, 5.0, DAQmx_Val_GroupByScanNumber, data, size, &read, NULL);
        counter += size;
        if (!hasChanged && data[0]>3.5) {
            sub.notify();
            hasChanged = true;
        }
        else if (hasChanged && data[0] < 3.5) {
            hasChanged = false;
        }
    }

    // Stop and clear the task
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
}

void startSession() {
    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();
}

int main()
{
    startSession();
    /*WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        return -1;
    }
    SOCKET serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, _T(localhost), &service.sin_addr.s_addr);
    service.sin_port = htons(port);
    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    SOCKET acceptSocket;
    acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    char receiveBuffer[bufferSize] = "";
    while (true) {
        int byteCount = recv(acceptSocket, receiveBuffer, bufferSize, 0);
        if (byteCount == SOCKET_ERROR) {
            break;
        }
        if (byteCount > 0) {
            if (receiveBuffer[0] == 'a') {
                break;
            }
            else {
                std::cout << receiveBuffer << std::endl;
            }
        }
    }
    closesocket(serverSocket);
    WSACleanup();*/
}