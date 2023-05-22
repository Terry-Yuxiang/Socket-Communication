# Socket Communication Projection

Part Done:

1. Basic part (As same as the project description)
   Start the server and client in order as serverM, serverA, serverB, and client. The serverA and serverB would separately read a.txt and b.txt and initialize names and their available time. When serverA and serverB finish initialization, they send a message through UDP to serverM. ServerM would get the name and remember which name belongs to which server. Users' input should be in the client process when they input multiple names, the client would send a message to serverM. When serverM receives the message, it judges if the names are valid and send the invalid names back to the client. ServerM would print out which names belong to which server and send requests to servers to get their available time. After serverA and serverB receive the message from serverM, they would find the time, combine them, and send them back to serverM. After serverM receives messages from serverA and sereverB, it combines the interval time and send them to the client. When the client receives the message from serverM, print out which names are invalid and print out valid names' common interval time.

2. Suffix(Optional) part

   If valid names' common interval time is not null, the client could let users choose one interval time which is valid. (Users' input should be like this "[start, end]" otherwise, it would be invalid input.) If the input is invalid, the client would print out the wrong message and let users input again. If users want to skip this step, users could input "skip" to skip the update step. After the client detect the valid interval time, it will send a message to serverM. When serverM receives the message it will print out one line that it receives the message through TCP and send the update messages to serverA and serverB. When serverA and serverB finish updating, send a finished message through UDP to serverM, which would tell the client the update is finished,


File Description:
---serverA.cpp: Read the input file and store the data. Receive messages from serverM, according to different header messages, find interval time, and update interval time operation.
---serverA.h: The header file of the serverA.cpp

---serverB.cpp: Read the input file and store the data. Receive messages from serverM, according to different header messages, find interval time, and update interval time operation.
---sereverB.h: The header file of the sereverB.cpp

---serverM.cpp: The main server remembers which names locate at which server. Receive messages from users according to different requests and ask serverA and serverB to do different things.
---serverM.h: The header file of the sereverM.cpp

---client.cpp: The client-side lets users input the names they want to check common interval time and allows users to choose one interval time.
---client.h: The header file of the client.cpp

---Makefile: Makefile of this project

---a.txt: Sample data read by the serverA
---b.txt: Sample data read by the serverB

Format of Message
client ---> serverM
Search interval time: 		SEARCH_name1,name2,name3,...
Update interval time: 		UPDATE_[start time, end]_name_name1,name2,name3,...
Description:
Usernames are concatenated and delimited by a comma. Headers are SEARCH_ and UPDATE_.
In the update message, _name_ is the separator signal for choosing time and names.

serverM ---> Client:
Search interval time: wrongName_0x1E_[xxx]timeWithNames
Update interval time: The time update successfully!
When messages are invalid, wrongName or time would be "IGNORE". "_0x1E" is the separator.
xxx is three digital to represent the time end loc in timeWithNames string. In this string, 
0-xxx is time, xxx-end is names.
If update successfully, the serverM would send "The time update successfully!" to client.

sereverM ---> serverA & serverB
Find interval time: 		SEARCH_name1,name2,name3,...
Update interval time: 		UPDATE_[start time, end]_name_name1,name2,name3,...
Description:
Usernames are concatenated and delimited by a comma. Headers are SEARCH_ and UPDATE_.
In the update message, _name_ is the separator signal for choosing time and names.

serverA ---> serverM
After initialization: 		serverA;U;name1,name2,name3,...
Find interval time result: 	serverA;I;[[0,1],[2,3],...]
After finishing the update: 	update_finished
Description:
Different flags are separated by semicolons. The first part is the server's name.
The second part represents the message type. U means to initialize serverM. It means
the message of interval-time results. In the update, only send the message update_finished.

serverB ---> serverM
After initialization: 		serverB;U;name1,name2,name3,...
Find Interval time result: 	serverB;I;[[0,1],[2,3],...]
After finishing the update: 	update_finished
Description:
Different flags are separated by semicolons. The first part is the server's name.
The second part represents the message type. U means to initialize serverM. It means
the message of interval-time results. In the update, only send the message update_finished.

Idiosyncrasy:
Not found yet.

Reused Code
The part of sending and receiving messages through UDP and TCP is a copy of Beej's Guide.
