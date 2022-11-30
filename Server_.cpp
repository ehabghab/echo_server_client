
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <string.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <set>

#define RESPONSE_MAX_LENGTH 1000

using namespace std::chrono_literals;

void sender(uint8_t socketId, std::string clientAdd, std::set<std::string> &activeClients, std::mutex &activeClientMutex)
{
  std::string data(RESPONSE_MAX_LENGTH + 1, 0);
  while (true)
  {
    // this buffer will hold the received bytes at socket.
    ssize_t bytesRead;
    // fill the buffer with zeros
    bzero(&data[0], RESPONSE_MAX_LENGTH);
    bytesRead = read(socketId, &data[0], RESPONSE_MAX_LENGTH);

    // if true, then client sent FIN to close connection.
    if (bytesRead == 0)
    {
      std::cout << "[" << __TIME__ << " "
                << "Server_.cpp:" << __LINE__ << "] "
                << "client " << clientAdd << " closed!\n";
      activeClientMutex.lock();
      activeClients.erase(clientAdd);
      activeClientMutex.unlock();
      return;
    }
    std::cout << "[" << __TIME__ << " "
              << "Server_.cpp:" << __LINE__ << "] "
              << "Receving " << data << " <-- " << clientAdd << "\n ";

    std::cout
        << "[" << __TIME__ << " "
        << "Server_.cpp:" << __LINE__ << "] "
        << "Echoing " << data << " --> " << clientAdd << "\n ";

    send(socketId, &data[0], bytesRead, 0);
  }
}

uint8_t initializeSocket(int port)
{
  uint8_t socketFileDescriptor;
  struct sockaddr_in address;
  int opt = 1;

  // Creating socket file descriptor
  if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt)))
  {
    perror("setsockopt - Reuse-Address");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEPORT, &opt,
                 sizeof(opt)))
  {
    perror("setsockopt - Reuse-Port");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(socketFileDescriptor, (struct sockaddr *)&address, sizeof(address)) <
      0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  return socketFileDescriptor;
}

void listenToSocket(uint8_t socketFileDescriptor, std::set<std::string> &activeClients, std::mutex &activeClientMutex, uint16_t port)
{

  std::cout << "[" << __TIME__ << " "
            << "Server_.cpp:" << __LINE__ << "] "
            << "Listening @ PORT:" << port << "\n";
  while (1)
  {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    uint8_t newSocket;

    if (listen(socketFileDescriptor, 3) < 0)
    {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    if ((newSocket = accept(socketFileDescriptor, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0)
    {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip));
    uint16_t port = htons(address.sin_port);
    std::string clientAdd = std::string(ip) + ":" + std::to_string(port);

    activeClientMutex.lock();
    activeClients.insert(clientAdd);
    activeClientMutex.unlock();

    std::cout << "[" << __TIME__ << " "
              << "Server_.cpp:" << __LINE__ << "] "
              << clientAdd
              << " Connected!\n";

    // spawn separate thread to handle the client.
    // This enables concurrency.
    std::thread send(sender, newSocket, clientAdd, std::ref(activeClients), std::ref(activeClientMutex));
    // Do not on this thread to finish.
    send.detach();
  }
}

void trackActiveClient(std::set<std::string> &activeClients, std::mutex &activeClientMutex)
{
  // this will print the <ip:port> of all active clients every 3 seconds.
  while (true)
  {
    activeClientMutex.lock();
    for (std::string clientAdd : activeClients)
    {
      std::cout << "[" << __TIME__ << " "
                << "Server_.cpp:" << __LINE__ << "] "
                << clientAdd << " is active\n";
    }
    activeClientMutex.unlock();
    std::this_thread::sleep_for(3000ms);
  }
}

int main(int argc, char **argv)
{

  if (argc < 2)
  {
    std::cout << "Usage Error: please select port number\n";
    return -1;
  }
  // this mutex protects reading and writing access to activeClient list.
  std::mutex activeClientMutex;
  std::set<std::string> activeClients;

  // initialize the server listening socket.
  uint8_t socketFileDescriptor = initializeSocket(std::atoi(argv[1]));

  // spawn thread to listen for new connections
  std::thread listenThread(listenToSocket, socketFileDescriptor, std::ref(activeClients), std::ref(activeClientMutex), std::atoi(argv[1]));

  // spawn thread to keep track of active clients.
  std::thread trackerThread(trackActiveClient, std::ref(activeClients), std::ref(activeClientMutex));
  listenThread.join();
  trackerThread.join();
  return 0;
}