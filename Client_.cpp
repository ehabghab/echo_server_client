#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <thread>
#include <iostream>

#define RESPONSE_MAX_LENGTH 1000
#define SERVER_IP "127.0.0.1"
#define PORT 7777
int connectToServer() {
  int sock = -1;
  struct sockaddr_in serv_addr;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "[" << __TIME__ << " "
              << "Client_.cpp:" << __LINE__ << "]"
              << "connectToServer(): Socket creation error\n";
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
    std::cout << "[" << __TIME__ << " "
              << "Client_.cpp:" << __LINE__ << "]"
              << "connectToServer(): Invalid address"
                 "Address not supported \n";
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "[Client_.cpp:" << __LINE__ << "]"
              << "connectToServer(): nConnection Failed\n";
    return -1;
  }

  return sock;
}

void receiver(int socketId) {

  std::string msg;

  // this buffer will hold the received bytes at socket.
  char *data = (char *)malloc(sizeof(char) * RESPONSE_MAX_LENGTH);

  while (true) {
    std::cout << "[" << __TIME__ << " "
              << "Client_.cpp:" << __LINE__ << "]"
              << "Please enter your msg: ";
    std::cin >> msg;
    auto msgLen = msg.length();
    if (msgLen == 0) {
      std::cout << "[" << __TIME__ << " "
                << "Client_.cpp:" << __LINE__ << "]"
                << "[Error] message is empty!";

      continue;
    }
    std::cout << "[" << __TIME__ << " "
              << "Client_.cpp:" << __LINE__ << "]" << msg << " --> server @"
              << SERVER_IP << ":" << PORT << "\n";
    send(socketId, msg.c_str(), msgLen, 0);

    // fill the buffer with zeros
    bzero(data, RESPONSE_MAX_LENGTH);
    auto bytesRead = read(socketId, data, RESPONSE_MAX_LENGTH);
    std::cout << "[" << __TIME__ << " "
              << "Client_.cpp:" << __LINE__ << "]" << msg << " <-- server @"
              << SERVER_IP << ":" << PORT << "\n";
  }
}

int main(int argc, char **argv) {
  auto socketId = connectToServer();
  if (socketId == -1) {
    exit(0);
  }
  std::thread recvThread(receiver, socketId);
  recvThread.join();
  return 0;
}