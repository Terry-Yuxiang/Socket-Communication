#include "client.h"


// Line 37 to 72 refer to Beej's socket tutorial
int main(int argc, char *argv[]) {
   std::cout << "Client is up and running." << std::endl;
   while (1) {
      // Set up tcp socket
      int sockfd, numbytes;
      struct addrinfo hints, *servinfo, *p;
      int rv;
      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_INET; // set to AF_INET to use IPv4
      hints.ai_socktype = SOCK_STREAM;

      if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
         return 1;
      }

      // loop through all the results and connect to the first we can
      for (p = servinfo; p != NULL; p = p->ai_next) {
         if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
         }
         if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd); // If failed, close this socket
            perror("client: connect"); // If failed, print error message
            continue;
         }
         break;
      }

      // If something wrong, print error message
      if (p == NULL) {
         fprintf(stderr, "client: failed to connect\n");
         return 2;
      }

      freeaddrinfo(servinfo); // All done with this structure

      // If we do not have time array, means need to input names and find available time.
      // else, we let user choose one time they want, and send to update their available time.
      if(availableTimeArray.size() == 0) {

         namesWaitUpdate.clear();   // Reset the data received last time
         availableTimeArray.clear();  // Reset the data received last time

         char buf[MAXBUFSIZE] = SEARCH_HEADER;  // A character array buf of size MAXBUFSIZE is defined to store data
         std::cout << "Please enter the usernames to check schedule availability:" << std::endl;
         int header_loc = strlen(buf);
         fgets(buf + header_loc, MAXBUFSIZE - header_loc, stdin);
         buf[strcspn(buf, "\n")] = '\0'; // Remove the newline character

         // Refer to Beej's socket tutorial
         if (send(sockfd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
         }
         std::cout << "Client finished sending the usernames to Main Server." << std::endl;


         /**
          * Get port number of this time, stored in clientPort.
          */
         int clientPort = getPortNumber(sockfd);

         // If receiving the data fails, the perror function is used to print an error message.
         if ((numbytes = recv(sockfd, buf, MAXBUFSIZE - 1, 0)) == -1) { // Put received data in buf
            perror("Received Message error");
            close(sockfd);
            continue;   // Go into next connection
         }
         buf[numbytes] = '\0';

         // Process separator, and split it into valid data.
         std::string receive(buf);
         std::size_t pos = receive.find(SEPARATOR);   // Find separator location
         std::string errorNames = receive.substr(0, pos);
         int timeLength = std::stoi( receive.substr(pos + SEPARATOR_LENGTH,RECEIVE_TIME_LENGTH));
         // Get time message
         std::string availableTime = receive.substr(pos + SEPARATOR_LENGTH + RECEIVE_TIME_LENGTH, timeLength);
         // Get name message
         std::string availableNames = receive.substr(pos + SEPARATOR_LENGTH +RECEIVE_TIME_LENGTH + timeLength);

         // Print the received data.
         // Check if the message is valid
         if (errorNames != "IGNORE") {
            // If not valid, print which name not found in server.
            std::cout << "Client received the reply from the Main Server using TCP over port " << clientPort << std::endl
                      << errorNames << std::endl;
         }
         if (availableTime != "IGNORE") {
            // If time valid, print names and their common available time
            std::cout << "Client received the reply from the Main Server using TCP over port " << clientPort << std::endl
                      <<"Time intervals "<< availableTime <<" works for "<<availableNames<<std::endl;

            // Store name in client, waiting for sending to server to update their time.
            namesWaitUpdate = availableNames;
            // Only there are time from serverM, we make string to array.
            stringToArray(availableTime);
         }
      }
      else{
      	bool firstTime = true;
         // Keep while, until user input the valid time.
         while(true) {
            char buf[MAXBUFSIZE] = UPDATE_HEADER;
            if(firstTime) { // First time to show users
            	std::cout << "Please enter the final meeting time to register an meeting:" << std::endl;
            	firstTime = false;
            }
            int header_loc = strlen(buf);
            fgets(buf + header_loc, MAXBUFSIZE - header_loc, stdin); // Get users' input
            buf[strcspn(buf, "\n")] = '\0'; // Remove the newline character

            // Give a choice for user to skip updating.
            if(strcmp(buf, "UPDATE_skip") == 0) {
               availableTimeArray.clear();
               namesWaitUpdate.clear();
               break;
            }

            std::string chooseTime(buf + header_loc);
            int start;
            int end;
            if(getFinalMeetingTime(start, end, chooseTime)) { // If input interval is valid
               bool validTime = false;
               for (const auto &oneInterval : availableTimeArray) { // Judge if time is valid
                  if(start >= oneInterval[0] && end <= oneInterval[1]) {
                     validTime = true;
                     std::string tempString = "_name_" + namesWaitUpdate;
                     const char* namesToUpdate = tempString.c_str();
                     strcat(buf, namesToUpdate);
                     if (strcmp(buf, "quit") == 0) {
                     }
                     std::cout<<"Sent the request to register "<<chooseTime<<" as the meeting time for "<<namesWaitUpdate<<std::endl;
                     if (send(sockfd, buf, strlen(buf), 0) == -1) {
                        perror("send");
                        exit(1);
                     }
                     break; // If find the interval could update, break the loop.
                  }
               }
               // If input time from user is valid send message to serverM else let user input again.
               if(!validTime) {
               	// If input is not valid, let user to input again.
                  std::cout<<"Time interval "<<chooseTime<<" is not valid. Please enter again:"<<std::endl;
               } else {
                  // Receive message from the server.
                  if ((numbytes = recv(sockfd, buf, MAXBUFSIZE - 1, 0)) == -1) {
                     perror("The update failed!!!"); // If failed to get the message from server，print error.
                     exit(1);
                  }
                  buf[numbytes] = '\0';
                  // If successfully, output message to user and break the while.
                  if(strcmp(buf, "The time update successfully!") == 0) {
                     std::cout<<"Received the notification that registration has finished."<<std::endl;
                     availableTimeArray.clear();
                     namesWaitUpdate.clear();
                     break;
                  }
               }
            } else {
            	// If input is not valid, let user to input again.
               std::cout<<"Time interval "<<chooseTime<<" is not valid. Please enter again:"<<std::endl;
            }
         }

      }
      close(sockfd);
   }
   return 0;
}

/**
 * Get the input start time and end time from user
 * @param start
 * @param end
 * @param input
 * @return
 */
bool getFinalMeetingTime(int &start, int &end, const std::string &input) {

   // The minus integer is not valid, if contains '-' in string, return false.
   if (input.find("-") != std::string::npos || input[0] != '[' || input[input.length() - 1] != ']') {
      return false;
   }

   std::vector<int> numbers;
   std::istringstream iss(input);

   // Skip '['
   iss.ignore();

   // Get number
   int num;
   while (iss >> num) {
      numbers.push_back(num);
      // Skip ',' and ' '
      iss.ignore(1, ']');
   }

   // If not get two number or start equals to end, meaning input is wrong, return false.
   if (numbers.size() != 2 || numbers[0] >= numbers[1]) {
      std::cout << "invalid input" << std::endl;
      return false;
   }
   // Store number in int variable.
   start = numbers[0];
   end = numbers[1];
//   std::cout << "a: " << start << ", b: " << end << std::endl;
   return true;
}

/**
 * Store string data in a vector
 * @param input string which is about time
 * @param result  vector to store time
 * @return  Successfully change type return true, else return false
 */
void stringToArray(const std::string &input) {

   availableTimeArray.clear();
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
      availableTimeArray.push_back(row);
   }
   return;
}

/**
 * Get the port number int his connection.
 * @return port number
 */
int getPortNumber(const int &sockfd) {
   struct sockaddr_in client_addr;
   socklen_t client_addr_len = sizeof(client_addr);
   if (getsockname(sockfd, (struct sockaddr *) &client_addr, &client_addr_len) == -1) {
      perror("Error retrieving local port number");
      exit(1);
   }
   return ntohs(client_addr.sin_port);
}