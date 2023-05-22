#ifndef EE450_FAN_YUXIANG_FANYUXIA_SERVERM_H
#define EE450_FAN_YUXIANG_FANYUXIA_SERVERM_H

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <algorithm>


#define TCPPORT "24321" // the port users will be connecting to
#define UDPPORT "23321" // the port users will be connecting to
#define MAXBUFLEN 100
#define SERVERPORT_A "21321"
#define SERVERPORT_B "22321"
#define SEPARATOR "_0x1E_" // Signal to separate string
#define BACKLOG 10 // how many pending connections queue will hold

#define SEARCH_HEADER "SEARCH_"
#define UPDATE_HEADER "UPDATE_"

std::vector <std::vector<int>> findCommonIntervals(const std::vector <std::vector<int>> &intervals1,
                                                   const std::vector <std::vector<int>> &intervals2);

int setupTCP(int &sockfd);

void tcp(int &sockfd, int &udpSockfd);

int setupUDP(int &sockfd);

void udp(int &sockfd);

void dealInputUDP(const char *buf);

void setUpUsernames(std::string server, std::string input);

void stringToArray(const std::string &input, std::string clientName);

int talkServer(const std::string &nameInServer, const std::string &header, const char *const serverName);

std::string timeAvailable(const std::string &nameInServerA, const std::string &nameInServerB);

bool findNameInMap(const char *buf, std::string &nameInServerA, std::string &nameInServerB, std::string &noNames,
                   std::string &validNames);

int setUpChild(int &new_fd, int &sockfd);

int tcpMessage(int &sockfd, int &new_fd, char *receiveMessage);

int findAvailableTIme(int &new_fd, int &udpSockfd, char *receiveMessage);

int updateTime(int &new_fd, int &udpSockfd, char *validMessage);

std::unordered_map <std::string, std::string> usernameMap;
std::vector <std::vector<int>> intervalA;
std::vector <std::vector<int>> intervalB;
bool bootUpA = false;
bool bootUpB = false;

#endif //EE450_FAN_YUXIANG_FANYUXIA_SERVERM_H
