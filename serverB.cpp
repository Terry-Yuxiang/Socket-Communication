#include "serverB.h"

int main() {
   std::ifstream infile("./b.txt");

   if (!infile) {
      std::cerr << "Error: Could not open input file." << std::endl;
      return 0;
   }
   std::string input;

   //Store input line in key:value pair
   while (std::getline(infile, input)) {
      ifValidInput(input);
   }

   // Start print out.
   std::cout<<"ServerB is up and running using UDP on port "<<UDPPORT<<"."<<std::endl;
   sendUsernames();
   std::cout<<"ServerB finished sending a list of usernames to Main Server."<<std::endl;
   setUDP();
   return 0;
}

/**
 * Find the common available time from two users
 * @param intervals1 Available time from user1
 * @param intervals2 Available time from user2
 * @return return their common available time
 */
std::vector<std::vector<int>> findCommonIntervals(const std::vector<std::vector<int>>& intervals1,
                                                  const std::vector<std::vector<int>>& intervals2) {
   std::vector<std::vector<int>> commonIntervals;
   size_t loc1 = 0;
   size_t loc2 = 0;
   while (loc1 < intervals1.size() && loc2 < intervals2.size()) {
      int start1 = intervals1[loc1][0];
      int end1 = intervals1[loc1][1];
      int start2 = intervals2[loc2][0];
      int end2 = intervals2[loc2][1];
      if(end2 <= start1) {
         loc2++;
         continue;
      }
      if(start2 >= end1) {
         loc1++;
         continue;
      }
      std::vector<int> nums = {start1, end1, start2, end2};
      std::sort(nums.begin(), nums.end());
      std::vector<int> row = {nums[1], nums[2]};
      commonIntervals.push_back(row);
      if(nums[3] == end2) {
         loc1++;
      } else {
         loc2++;
      }
   }
   return commonIntervals;
}

// The part of send message through UDP refer to Beej's socket tutorial
int setUDP () {
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytes;
   struct sockaddr_storage their_addr;
   socklen_t addr_len;

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

   while (1) {
      char buf[MAXBUFLEN];
      addr_len = sizeof their_addr;
      if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                               (struct sockaddr *) &their_addr, &addr_len)) == -1) {
         perror("recvfrom");
         exit(1);
      }
      buf[numbytes] = '\0';

      std::string header(buf, 0, 7);
      char * validMessage = buf + 7;

      // Header to search the names and find their common available time.
      if(header == SEARCH_HEADER) {
         std::cout <<"Server B received the usernames from Main Server using UDP over port "<< UDPPORT << "."
         <<std::endl;

         // Put string in string stream to iterate.
         std::stringstream names(validMessage);
         std::vector<std::string> keyNames;  // Store names
         std::string oneName;
         while (std::getline(names, oneName, ',')) {  // Push name in vector
            keyNames.push_back(oneName);  // Push name in vector
         }

         std::vector<std::vector<int>> combine = hashmap[keyNames[0]];
         for(size_t i = 1; i < keyNames.size(); i++) {
            combine = findCommonIntervals(combine, hashmap[keyNames[i]]);
         }

         // Print interval vector
         std::string combineString = "[";
         for (size_t i = 0; i < combine.size(); i++) {
            if (i != combine.size() - 1) {
               combineString += "[" + std::to_string(combine[i][0]) + "," + std::to_string(combine[i][1]) + "]" + ",";
            } else {
               combineString += "[" + std::to_string(combine[i][0]) + "," + std::to_string(combine[i][1]) + "]";
            }
         }
         combineString += "]";

         // Set up ServerB's message header and join with true message
         std::string withHeader = "serverB;I;" + combineString;
         // Change string to char[]
         int str_len = withHeader.length();
         char* sendtoServerM = new char[str_len + 1];
         memcpy(sendtoServerM, withHeader.c_str(), str_len);
         sendtoServerM[str_len] = '\0';

         std::cout<<"Found the intersection result: "<<combineString<<" for "<<validMessage<<std::endl;
         sendUDP(sendtoServerM); // send message to serverM
         std::cout<<"Server B finished sending the response to Main Server."<<std::endl;
      }

      // Header to update the time for names received.
      if (header == UPDATE_HEADER) {
         std::string delimiter = "_name_";
         std::string time, names;
         std::string validMessageString(validMessage);
         size_t delimiter_position = validMessageString.find(delimiter);
         if (delimiter_position != std::string::npos) {
            time = validMessageString.substr(0, delimiter_position);
            names = validMessageString.substr(delimiter_position + delimiter.length());
         }
         std::cout << "Register a meeting at " << time << " and update the availability for the following users:"
                   << std::endl;

         int start, end;
         getStartEnd(start, end, time);
         // Iterate all names
         names.resize(names.size() - 1); // Delete the last char '.'
         std::stringstream nameStream(names);  // char array to string
         std::string oneName;
         // Iterator all names may need to be updated.
         while (std::getline(nameStream, oneName, ',')) {
            auto it = hashmap.find(oneName);

            if (it == hashmap.end()) {
               continue;
            }

            // Iterator the user's available time and update it.
            std::vector <std::vector<int>> array = it->second;
            // Print out the`vector before update
            std::cout << it->first << ": update from " << std::flush;
            printVector(array);
            std::cout << " to " << std::flush;

            for (auto array_it = array.begin(); array_it != array.end(); ) {
               int originStart = (*array_it)[0];
               int originEnd = (*array_it)[1];
               if (start >= originStart && end <= originEnd) {
                  // Add new vector
                  if(start != originStart) {
                     std::vector<int> newVector1 = {originStart, start};
                     array_it = array.insert(array_it, newVector1); // Insert vector at this position
                     ++array_it; // move to next, making it still point to old vector
                  }
                  if(end != originEnd) {
                     std::vector<int> newVector2 = {end, originEnd};
                     array_it = array.insert(array_it, newVector2); // Insert vector at this position
                     ++array_it; // move to next, making it still point to old vector
                  }
                  // Delete original vector
                  array_it = array.erase(array_it);
                  break;
               } else {
                  ++array_it;
               }
            }
            it->second = array;
            // Print out the`vector after update
            printVector(array);
            std::cout<<std::endl;
         }
         std::cout<<"Notified Main Server that registration has finished."<<std::endl;
         sendUDP("update_finished");
      }
   }
   close(sockfd);
   return 0;
}

/**
 * Get the start and end time from the string which comes from client
 * @param start The start time from client
 * @param end The end time from client
 * @param input client input time
 */
void getStartEnd(int &start, int &end, const std::string &input) {
   std::vector<int> numbers;
   std::istringstream iss(input);

   // skip '['
   iss.ignore();

   // get the number
   int num;
   while (iss >> num) {
      numbers.push_back(num);
      iss.ignore(1, ']');
   }
   start = numbers[0];
   end = numbers[1];
   return;
}

/**
 * Check if the message from the initializing file is valid
 * @param input Message from initializing file
 */
void ifValidInput(const std::string &input) {
   std::string beforeSemi, afterSemi;
   std::string outputBefore = "";
   std::vector <std::vector<int>> outputAfter;

   splitFromSemi(input, beforeSemi, afterSemi);
   checkInputName(beforeSemi, outputBefore);
   removeSpaces(afterSemi);
   stringToArray(afterSemi, outputAfter);


   hashmap[outputBefore]=outputAfter;
}

/**
 * Print one name's leisure time
 * @param outputAfter
 */
void printVector(const std::vector <std::vector<int>> &outputAfter) {
   std::cout<<"[";
   for (auto row_it = outputAfter.begin(); row_it != outputAfter.end(); ++row_it) {
      std::cout << "[";
      for (size_t i = 0; i < row_it->size(); ++i) {
         std::cout << (*row_it)[i];
         if (i + 1 < row_it->size()) {
            std::cout << ",";
         }
      }
      std::cout << "]";

      // The last do not need ','
      if (std::next(row_it) != outputAfter.end()) {
         std::cout << ",";
      }
   }
   std::cout<<"]";
}

/**
 * Process input string
 * @param input
 * @param beforeSemi
 * @param afterSemi
 * @return If success return true, else return false
 */
bool splitFromSemi(const std::string &input, std::string &beforeSemi, std::string &afterSemi) {
   std::regex pattern(R"((.*)\s*;\s*(.*))");

   std::smatch result;
   if (std::regex_search(input, result, pattern)) {
      if (result.size() == 3) {
         beforeSemi = result[1].str();
         afterSemi = result[2].str();
         return true;
      }
   }
   return false;
}

/**
 * Check if the initializing name is valid
 * @param input Original name
 * @param output Remove front and end space
 * @return If all name is valid return true, else return false
 */
bool checkInputName(const std::string &input, std::string &output) {
   std::string trim;

   // Remove leading and trailing spaces
   auto start = input.find_first_not_of(" ");
   auto end = input.find_last_not_of(" ");

   // Judge if the string is null
   if (start == std::string::npos || end == std::string::npos) {
      return false;
   }

   trim = input.substr(start, end - start + 1);

   for (char c: trim) {
      if (!std::islower(c) || c == ' ') {
         return false;
      }
   }

   output = trim;
   return true;
}

/**
 * Remove surplus spaces from string
 * @param input
 */
void removeSpaces(std::string &input) {
   input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
}

/**
 * Store string data in a vector
 * @param input string which is about time
 * @param result  vector to store time
 * @return  Successfully change type return true, else return false
 */
bool stringToArray(const std::string &input, std::vector <std::vector<int>> &result) {
   if (input.size() < 2) {
      return false;
   } else {
      if (input[0] != '[' || input[input.size() - 1] != ']') {
         return false;
      }
      if (input.size() < 7 || (input[1] != '[' || input[input.size() - 2] != ']')) {
         return false;
      }
   }

   std::stringstream noOutBrackets(input.substr(1, input.size() - 2));
   std::string item;

   while (std::getline(noOutBrackets, item, ']')) {
      if (item.empty()) {
         return false;
      }

      if (item[0] == '[') {
         item = item.substr(1); // Remove opening bracket
      } else if (item[0] == ',' && item[1] == '[') {
         item = item.substr(2);
      } else {
         return false;
      }
      std::stringstream interval(item);
      std::string number;
      std::vector<int> row;

      while (std::getline(interval, number, ',')) {
         row.push_back(std::stoi(number));
      }

      result.push_back(row);
   }

   return true;
}

/**
 * Send message to serverM through UDP
 * @param sendtoServerM The message sent to serverM
 * @return
 */
// Refer to Beej's socket tutorial
int sendUDP(const char* sendtoServerM) {
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET; // set to AF_INET to use IPv4
   hints.ai_socktype = SOCK_DGRAM;

   if ((rv = getaddrinfo("localhost", "23321", &hints, &servinfo)) != 0) {
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

   if (sendto(sockfd, sendtoServerM, strlen(sendtoServerM), 0 ,p->ai_addr, p->ai_addrlen) == -1) {
      perror("talker: sendto");
      exit(1);
   }

   freeaddrinfo(servinfo);

   close(sockfd);
   return 0;
}

/**
 * After initialization, send names store in this server to serverM
 * @return
 */
int sendUsernames() {
   int totalLength = 0;
   for (auto it = hashmap.begin(); it != hashmap.end(); ++it) {
      totalLength += it->first.length() + 1;
   }
   // Create new char array.
   std::string head = "serverB;U;";
   char* usernames = new char[totalLength + head.length()];
   head.copy(usernames, head.length(), 0);
   int index = head.length();
   // Put all key in array.
   for (auto it = hashmap.begin(); it != hashmap.end(); ++it) {
      std::string key = it->first;
      key.copy(usernames + index, key.length(), 0);
      usernames[index + key.length()] = ',';
      index += key.length() + 1;
   }
   usernames[totalLength + head.length() - 1] = '\0';

   sendUDP(usernames);

   return 0;
}