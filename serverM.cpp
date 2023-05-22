#include "serverM.h"

int main(void) {
   std::cout << "Main Server is up and running." << std::endl;
   int tcpSockfd, udpSockfd;
   setupUDP(udpSockfd); // Set upt UDP socket.
   setupTCP(tcpSockfd); // Set upt TCP socket.
   // std::cout << "bootup" << std::endl;
   while (!bootUpA || !bootUpB) { // Wait to receive serverA and serverB intialization message.
   	 udp(udpSockfd);
   }
   while (1) {
      intervalA.clear(); // Clear data received from serverA, last time.
      intervalB.clear(); // Clear data received from serverB, last time.
      int new_fd;
      char receiveMessage[1024]; // For storing message from TCP.
      setUpChild(new_fd, tcpSockfd); // Set up TCP child.
      tcpMessage(tcpSockfd, new_fd, receiveMessage); // Get TCP message.
      std::string header(receiveMessage, 0, 7); // Get the header from TCP message.
      char *validMessage = receiveMessage + 7; // Get valid message from TCP message.
      if (header == SEARCH_HEADER) { // Header to search interval time.
         findAvailableTIme(new_fd, udpSockfd, validMessage);
      }
      if (header == UPDATE_HEADER) { // Header to update interval time.
         updateTime(new_fd, udpSockfd, validMessage);
      }
      close(new_fd); // Close this TCP connection.
   }
   close(tcpSockfd);
   close(udpSockfd);
   return 0;
}

/**
 * Connect to serverA and serverB separately, let them update time
 * @param udpSockfd  UDP socket
 * @param validMessage Time and name waiting for update
 * @return
 */
int updateTime(int &new_fd, int &udpSockfd, char *validMessage) {
//   std::cout<<validMessage<<std::endl;
   talkServer(validMessage, UPDATE_HEADER, SERVERPORT_A); // Send message to serverA.
   udp(udpSockfd); // Receive message from serverA.

   talkServer(validMessage, UPDATE_HEADER, SERVERPORT_B); // Send message to serverB.
   udp(udpSockfd); // Receive message from serverB.

   const char *sentToClient = "The time update successfully!";
   if (send(new_fd, sentToClient, strlen(sentToClient), 0) == -1) {
      perror("send");
      exit(1);
   }
   return 0;
}

/**
 * Find the available time from serverA and serverB, combine them and send message back to client
 * @param validMessage
 * @return
 */
int findAvailableTIme(int &new_fd, int &udpSockfd, char *validMessage) {
   std::string toClientMessage = "";
   std::string timeToClient = "006IGNORE"; // Default IGNORE length is six.
   std::string wrongName = "IGNORE";
   std::string nameInServerA = "";
   std::string nameInServerB = "";
   std::string noNames = "";
   std::string validNames = "";

   // If iterate every name, if any name not found in server A & B, send wrong name back to client.
   bool allValid = findNameInMap(validMessage, nameInServerA, nameInServerB, noNames, validNames);
   if (!allValid) {
      std::cout << noNames << " do not exist. Send a reply to the client." << std::endl;
      noNames += " do not exist.";
      wrongName = noNames;
   }
   if (!validNames.empty()) {
      // Find the available time separately in serverA and serverB.
      // Find in A.
      if (nameInServerA.length() > 0) {
         std::cout << "Found " << nameInServerA << " located at Server A. Send to Server A." << std::endl;
         talkServer(nameInServerA, SEARCH_HEADER, SERVERPORT_A);
      }
      // Find in B.
      if (nameInServerB.length() > 0) {
         std::cout << "Found " << nameInServerB << " located at Server B. Send to Server B." << std::endl;
         talkServer(nameInServerB, SEARCH_HEADER, SERVERPORT_B);
      }
      if(nameInServerA.length() > 0) {
         udp(udpSockfd);   // If contact to serverA previous, setup to receive message from A.
      }
      if(nameInServerB.length() > 0) {
         udp(udpSockfd);   // If contact to serverB previous, setup to receive message from B.
      }


      // Combine available time from to server.
      std::string time = timeAvailable(nameInServerA, nameInServerB);
      int length = time.length();
      if (length >= 100) {
         timeToClient = std::to_string(length) + time + validNames;

      } else if (length >= 10) {
         timeToClient = "0" + std::to_string(length) + time + validNames;
      } else {
         timeToClient = "00" + std::to_string(length) + time + validNames;
      }

   }

   // Combine invalid name and valid name with time
   toClientMessage += wrongName + SEPARATOR + timeToClient;
   int str_len = toClientMessage.length();
   char *sentToClient = new char[str_len + 1];
   memcpy(sentToClient, toClientMessage.c_str(), str_len);
   sentToClient[str_len] = '\0';

   std::cout<<"Main Server sent the result to the client."<<std::endl;
   // Send the available time to client
   if (send(new_fd, sentToClient, strlen(sentToClient), 0) == -1) {
      perror("send");
      exit(1);
   }
   return 0;
}

/**
 * Receive message from client
 * @param receiveMessage The message received from client
 * @return
 */
int tcpMessage(int &sockfd, int &new_fd, char *receiveMessage) {
   // Buffer and variable to store received message

   char buf[1024];
   int numbytes;

   // Receive message from client
   if ((numbytes = recv(new_fd, buf, sizeof(buf) - 1, 0)) == -1) {
      perror("recv");
      exit(1);
   }

   // If recv returns 0, do close this child and do nothing.
   if (numbytes == 0) {
      buf[numbytes] = '\0';
      strcpy(receiveMessage, buf);
      return 0;
//      std::cout << "The connection is closed by client" << std::endl;
   }

   buf[numbytes] = '\0';
//   printf("server: received '%s'\n", buf);
   printf("Main Server received the request from client using TCP over port %s. \n", TCPPORT);

   strcpy(receiveMessage, buf);
   return 0;
}

/**
 * Set up child in TCP
 * @return
 */
// Refer to Beej's socket tutorial
int setUpChild(int &new_fd, int &sockfd) {
   struct sockaddr_storage their_addr;  // connector's address information
   socklen_t sin_size = sizeof their_addr;
   new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
   if (new_fd == -1) {
      perror("serverM: accept()");
      exit(1);
   }
   return 0;
}

/**
 * Use message received from client, judge which name in serverA and which in serverB, if not stored record them.
 * @param buf The message received from client
 * @param nameInServerA The name from client stored in serverA
 * @param nameInServerB The name from client stored in serverB
 * @param noNames The name from client couldn't find in servaerA or serverB
 * @return false if any name not stored in serverA or serverB
 */
bool findNameInMap(const char *buf, std::string &nameInServerA, std::string &nameInServerB, std::string &noNames,
                   std::string &validNames) {
   std::stringstream nameStream(buf);  // char array to string
   std::string oneName;
   bool findAllNames = true;
   while (std::getline(nameStream, oneName, ' ')) {
      auto it = usernameMap.find(oneName);
      if (it == usernameMap.end()) {
//         std::cout<<"This name could not find in map!!!" <<std::endl;
         noNames += oneName + ",";
         findAllNames = false;
      } else if (it->second == "serverA") {
         nameInServerA += it->first + ",";
         validNames += it->first + ",";
      } else {
         nameInServerB += it->first + ",";
         validNames += it->first + ",";
      }
   }
   // Erase the last redundant char ','
   if (nameInServerA.length() > 0) {
      nameInServerA.erase(nameInServerA.size() - 1);
   }
   if (nameInServerB.length() > 0) {
      nameInServerB.erase(nameInServerB.size() - 1);
   }
   if (noNames.length() > 0) {
      noNames.erase(noNames.size() - 1);
   }
   if (validNames.length() > 0) {
      validNames[validNames.size() - 1] = '.';
   }
   return findAllNames;
}

/**
 * Find the available time.
 * @param nameInServerA Names from client stored in serverA
 * @param nameInServerB Names from client stored in serverB
 * @return
 */
std::string timeAvailable(const std::string &nameInServerA, const std::string &nameInServerB) {
   std::vector <std::vector<int>> combine;
   // If you get data both from server A & B, combine two interval, else just use one.
   if (nameInServerA.length() > 0 && nameInServerB.length() > 0) {
      combine = findCommonIntervals(intervalA, intervalB);
   } else {
      if (nameInServerB.length() > 0) {
         combine.assign(intervalB.begin(), intervalB.end());
      } else {
         combine.assign(intervalA.begin(), intervalA.end());
      }
   }
   std::string combineString = "[";
   for (size_t i = 0; i < combine.size(); i++) {

      if (i != combine.size() - 1) {
         combineString += "[" + std::to_string(combine[i][0]) + "," + std::to_string(combine[i][1]) + "]" + ",";
      } else {
         combineString += "[" + std::to_string(combine[i][0]) + "," + std::to_string(combine[i][1]) + "]";
      }
   }
   combineString += "]";

   std::cout << "Found the intersection between the results from server A and B:" << std::endl;
   std::cout << combineString << std::endl;

   return combineString;
}

/**
 * Receive message from serverA and serverB
 * @param sockfd
 */
// Refer to Beej's socket tutorial
void udp(int &sockfd) {
   socklen_t addr_len;
   struct sockaddr_storage their_addr;
   int numbytes;
   char buf[MAXBUFLEN];
   addr_len = sizeof their_addr;
   if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                            (struct sockaddr *) &their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
   }

   buf[numbytes] = '\0';

   if (strcmp(buf, "update_finished") != 0) {
      dealInputUDP(buf);
   }

}

/**
 * Set up TCP socket
 * @param sockfd
 * @return
 */
// Refer to Beej's socket tutorial
int setupTCP(int &sockfd) {
   struct addrinfo hints, *servinfo, *p;
   int yes = 1;
   int rv;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET; // set to AF_INET to use IPv4
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE; // use my IP

   if ((rv = getaddrinfo("localhost", TCPPORT, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1) {
         perror("server: socket");
         continue;
      }

      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                     sizeof(int)) == -1) {
         perror("setsockopt");
         exit(1);
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
         close(sockfd);
         perror("server: bind");
         continue;
      }
      break;
   }

   freeaddrinfo(servinfo); // all done with this structure

   if (p == NULL) {
      fprintf(stderr, "server: failed to bind\n");
      exit(1);
   }

   if (listen(sockfd, BACKLOG) == -1) {
      perror("listen");
      exit(1);
   }
   return 0;
}

/**
 * Set up UDP socket
 * @param sockfd
 * @return
 */
// Refer to Beej's socket tutorial
int setupUDP(int &sockfd) {
   struct addrinfo hints, *servinfo, *p;
   int rv;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET; // set to AF_INET to use IPv4
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE; // use my IP

   if ((rv = getaddrinfo("localhost", UDPPORT, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
         perror("listener: socket");
         continue;
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
         close(sockfd);
         perror("listener: bind");
         continue;
      }

      break;
   }

   if (p == NULL) {
      fprintf(stderr, "listener: failed to bind socket\n");
      return 2;
   }

   freeaddrinfo(servinfo);

   return 0;
}

/**
 * Process data receive from serverA and serverB
 * @param buf Message received from serverA and serverB
 */
void dealInputUDP(const char *buf) {
   std::string clientName(buf, 7);
   if (clientName == "serverA") {
      bootUpA = true;
      std::string dataType(buf + 8, 1);
      if (dataType == "U") {  // If dataType is U, means it is initialization
         std::string usernames(buf + 10);
         setUpUsernames(clientName, usernames);
         std::cout << "Main Server received the username list from server A using UDP over port " << UDPPORT << "."
                   << std::endl;
      } else {
         // Else, it is message of available time
         std::cout << "Main Server received from server A the intersection result using UDP over port " << UDPPORT
         << ":"<<std::endl;
         std::string intervalFromA(buf + 10);
         if (intervalFromA != "[]") {
            stringToArray(intervalFromA, clientName);
            // Print interval A
            std::string combineString = "[";
            for (size_t i = 0; i < intervalA.size(); i++) {

               if (i != intervalA.size() - 1) {
                  combineString +=
                        "[" + std::to_string(intervalA[i][0]) + "," + std::to_string(intervalA[i][1]) + "]" + ",";
               } else {
                  combineString += "[" + std::to_string(intervalA[i][0]) + "," + std::to_string(intervalA[i][1]) + "]";
               }
            }
            combineString += "]";
            std::cout << combineString << std::endl;
         }
      }
   } else if (clientName == "serverB") {
      bootUpB = true;
      std::string dataType(buf + 8, 1);
      if (dataType == "U") {
         std::string usernames(buf + 10);
         setUpUsernames(clientName, usernames);
         std::cout << "Main Server received the username list from server B using UDP over port " << UDPPORT << "."
                   << std::endl;
      } else {
         std::cout << "Main Server received from server B the intersection result using UDP over port " << UDPPORT
                   << ":"<<std::endl;
         std::string intervalFromB(buf + 10);
         if (intervalFromB != "[]") {
            stringToArray(intervalFromB, clientName);
            // Print interval B
            std::string combineString = "[";
            for (size_t i = 0; i < intervalB.size(); i++) {

               if (i != intervalB.size() - 1) {
                  combineString +=
                        "[" + std::to_string(intervalB[i][0]) + "," + std::to_string(intervalB[i][1]) + "]" + ",";
               } else {
                  combineString += "[" + std::to_string(intervalB[i][0]) + "," + std::to_string(intervalB[i][1]) + "]";
               }
            }
            combineString += "]";
            std::cout << combineString << std::endl;
         }
      }
   }

}

/**
 * Store the message of which name in which server
 * @param server
 * @param input
 */
void setUpUsernames(std::string server, std::string input) {
   std::stringstream names(input);
   std::string oneName;
   while (std::getline(names, oneName, ',')) {  // Split the string with a comma delimiter
      usernameMap.insert(std::make_pair(oneName, server)); // Store the split string in a vector
   }
}

/**
 * Convert string to data in vector
 * @param input string
 * @param clientName Message from which server
 */
void stringToArray(const std::string &input, std::string clientName) {
   std::stringstream noOutBrackets(input.substr(1, input.size() - 2));
   std::string item;
   while (std::getline(noOutBrackets, item, ']')) {
      if (item[0] == '[') {
         item = item.substr(1); // Remove opening bracket
      } else if (item[0] == ',' && item[1] == '[') {
         item = item.substr(2);
      }
      std::stringstream interval(item);
      std::string number;
      std::vector<int> row;

      while (std::getline(interval, number, ',')) {
         row.push_back(std::stoi(number));
      }

      if (clientName == "serverA") {
         intervalA.push_back(row);  // Store the time in intervalA
      } else {
         intervalB.push_back(row);  // Store the time in intervalB
      }

   }
   return;
}

/**
 * send message to server from udp
 * @param nameInServer names in this server
 * @param header tell server update or search
 * @return
 */
// Refer to Beej's socket tutorial
int talkServer(const std::string &nameInServer, const std::string &header, const char *const serverName) {
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytes;


   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET; // set to AF_INET to use IPv4
   hints.ai_socktype = SOCK_DGRAM;

   if ((rv = getaddrinfo("localhost", serverName, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and make a socket
   for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1) {
         perror("talker: socket");
         continue;
      }

      break;
   }

   if (p == NULL) {
      fprintf(stderr, "talker: failed to create socket\n");
      return 2;
   }

   std::string message = header + nameInServer;
   size_t messageLength = message.length() + 1; // +1 是为了容纳空字符 '\0'
   char *messageToServer = new char[messageLength];
   strcpy(messageToServer, message.c_str());

   if ((numbytes = sendto(sockfd, messageToServer, strlen(messageToServer), 0,
                          p->ai_addr, p->ai_addrlen)) == -1) {
      perror("talker: sendto");
      exit(1);
   }

   freeaddrinfo(servinfo);

//   printf("talker: sent %d bytes to %s\n", numbytes, "localhost");
   close(sockfd);

   return 0;
}

/**
 * Find the common available time from two users
 * @param intervals1 Available time from user1
 * @param intervals2 Available time from user2
 * @return return their common available time
 */
std::vector <std::vector<int>> findCommonIntervals(const std::vector <std::vector<int>> &intervals1,
                                                   const std::vector <std::vector<int>> &intervals2) {
   std::vector <std::vector<int>> commonIntervals;
   size_t loc1 = 0;
   size_t loc2 = 0;
   while (loc1 < intervals1.size() && loc2 < intervals2.size()) {
      int start1 = intervals1[loc1][0];
      int end1 = intervals1[loc1][1];
      int start2 = intervals2[loc2][0];
      int end2 = intervals2[loc2][1];
      if (end2 <= start1) {
         loc2++;
         continue;
      }
      if (start2 >= end1) {
         loc1++;
         continue;
      }
      std::vector<int> nums = {start1, end1, start2, end2};
      std::sort(nums.begin(), nums.end());
      std::vector<int> row = {nums[1], nums[2]};
      commonIntervals.push_back(row);
      if (nums[3] == end2) {
         loc1++;
      } else {
         loc2++;
      }
   }
   return commonIntervals;
}