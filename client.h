#ifndef EE450_FAN_YUXIANG_FANYUXIA_CLIENT_H
#define EE450_FAN_YUXIANG_FANYUXIA_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <vector>
#include <cstring>


#define PORT "24321" // the port client will be connecting to
#define SEPARATOR "_0x1E_" // Signal to separate string
#define SEPARATOR_LENGTH 6
#define IGNORE "IGNORE" // Signal to ignore message
#define SEARCH_HEADER "SEARCH_"
#define UPDATE_HEADER "UPDATE_"
#define CHOOSE_INTERVAL false
#define RECEIVE_TIME_LENGTH 3
#define MAXBUFSIZE 1024 // max number of bytes we can get at once

void stringToArray(const std::string &input);
bool getFinalMeetingTime(int &start, int &end, const std::string &input);
int getPortNumber(const int &sockfd);

std::vector<std::vector<int>> availableTimeArray;  // Store the time get from serverM
std::string namesWaitUpdate;  // Store name get from serverM

#endif //EE450_FAN_YUXIANG_FANYUXIA_CLIENT_H
