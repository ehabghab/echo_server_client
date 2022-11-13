
#include <linux/sockios.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

#define PORT 7777
#define RESPONSE_MAX_LENGTH 1000

uint8_t initializeSocket() {
  uint8_t socketFileDescriptor;
  struct sockaddr_in address;
  int opt = 1;

  // Creating socket file descriptor
  if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt))) {
    perror("setsockopt - Reuse-Address");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt - Reuse-Port");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(socketFileDescriptor, (struct sockaddr *)&address, sizeof(address)) <
      0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  return socketFileDescriptor;
}

uint8_t listenToSocket(uint8_t socketFileDescriptor) {
  std::cout << "[" << __TIME__ << " "
            << "Server_.cpp:" << __LINE__ << "]"
            << " Listening @ PORT:" << PORT << "\n";
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  uint8_t newSocket;

  if (listen(socketFileDescriptor, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  if ((newSocket = accept(socketFileDescriptor, (struct sockaddr *)&address,
                          (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  std::cout << "[" << __TIME__ << " "
            << "Server_.cpp:" << __LINE__ << "]"
            << "Connected!\n";
  return newSocket;
}

void sender(uint8_t socketId) {
  while (true) {
    // this buffer will hold the received bytes at socket.
    char *data = (char *)malloc(sizeof(char) * RESPONSE_MAX_LENGTH);
    ssize_t bytesRead;
    // fill the buffer with zeros
    bzero(data, RESPONSE_MAX_LENGTH);
    bytesRead = read(socketId, data, RESPONSE_MAX_LENGTH);
    if (bytesRead == 0) {
      continue;
    }
    std::cout << "[" << __TIME__ << " "
              << "Server_.cpp:" << __LINE__ << "]"
              << "Receving " << data << " <-- client\n";

    std::cout << "[" << __TIME__ << " "
              << "Server_.cpp:" << __LINE__ << "]"
              << "Echoing " << data << " --> client\n";

    send(socketId, data, bytesRead, 0);
  }
}

int main(int argc, char **argv) {
  auto socketFileDescriptor = initializeSocket();
  auto socketId = listenToSocket(socketFileDescriptor);
  std::thread sendThread(sender, socketId);
  sendThread.join();
  return 0;
}