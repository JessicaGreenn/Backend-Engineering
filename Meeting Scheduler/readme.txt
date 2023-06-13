a. Full Name: Danyang Zhang

b. StudentID: 4565957475

c. I have completed the optional part (extra credits). The valid input should be like format "[1,2]" or " [  1 ,   2   ]  " (with spaces). If the available time interval is "[]", then please terminate it and restart to avoid being asked for user input over and over again.(Because in this situation any input is invalid.) You can grade project without the extra part by commenting out codes between "//*******************extra points***********************", thanks!

d. Code files:
serverM.cpp: Store names from backend servers through UDP and collect usernames from the client through TCP. Then calculate the final intersections and send them to the client.
serverA.cpp: Store part of the names and the corresponding intervals. Calculate the intersection of users on serverA and send the result to the client through TCP.
serverB.cpp: Store part of the names and the corresponding intervals. Calculate the intersection of users on serverB and send the result to the client through TCP.
client.cpp: Read the input, request with usernames and get the intersection from serverM.

e. The format of all the messages exchanged:
serverA/B to serverM: usernames are concatenated and delimited by a space.
client to serverM: usernames are concatenated and delimited by a space.
serverM to serverA/B and client: usernames are concatenated and delimited by a comma.
serverA/B to serverM: intervals are formed as "[start,end]" and are encapsulated in a pair of brackets "[]". If it's empty, it's "[]".
serverM to client: intervals are formed as "[start,end]" and are encapsulated in a pair of brackets "[]". If it's empty, it's "[]".

g. It may show "M fails to bind." message, this is because some zombie processes are created while testing codes, please make sure to kill them and then the codes will work well. (Or wait for some time and restart) Thanks!

h. Reused Code: 
1. Modified Beej's code for TCP/UDP socket creation, binding, receiving and sending.
2. Modified code from https://cloud.tencent.com/developer/article/1787844 to find overlaps between two schedules.
Modified from https://blog.csdn.net/qian2213762498/article/details/81705647 to strip spaces off a string.