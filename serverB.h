#ifndef EE450_FAN_YUXIANG_FANYUXIA_SERVERB_H
#define EE450_FAN_YUXIANG_FANYUXIA_SERVERB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <thread>
#include <regex>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstring>

#define UDPPORT "22321" // the port users will be connecting to
#define MAXBUFLEN 100
#define SEARCH_HEADER "SEARCH_"
#define UPDATE_HEADER "UPDATE_"

std::vector<std::vector<int>> findCommonIntervals(const std::vector<std::vector<int>>& intervals1,
                                                  const std::vector<std::vector<int>>& intervals2);

bool splitFromSemi(const std::string &input, std::string &beforeSemi, std::string &afterSemi);

bool checkInputName(const std::string &input, std::string &output);

void removeSpaces(std::string &input);

bool stringToArray(const std::string &input, std::vector <std::vector<int>> &result);

void printVector(const std::vector <std::vector<int>> &outputAfter);

void ifValidInput(const std::string &input);

void getStartEnd(int &start, int &end, const std::string &input);

int setUDP();

int sendUDP(const char* sendtoServerM);

int sendUsernames();

std::unordered_map<std::string, std::vector <std::vector<int>>> hashmap; // Store names and their valid time.

#endif //EE450_FAN_YUXIANG_FANYUXIA_SERVERB_H
