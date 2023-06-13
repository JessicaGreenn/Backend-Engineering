Developer Name: Danyang Zhang

When scheduling a time from the output time list and updating all meeting members schedule, valid user input can be "[1,2]" or " [  1 ,   2   ]  " (with spaces). You can use this scheduler without the scheduling part by commenting out codes between "//*******************extra points***********************".

Code files:
serverM.cpp: Store names from backend servers through UDP and collect usernames from the client through TCP. Then calculate the final intersections and send them to the client.
serverA.cpp: Store part of the names and the corresponding intervals. Calculate the intersection of users on serverA and send the result to the client through TCP.
serverB.cpp: Store part of the names and the corresponding intervals. Calculate the intersection of users on serverB and send the result to the client through TCP.
client.cpp: Read the input, request with usernames and get the intersection from serverM.

The format of all the messages exchanged:
serverA/B to serverM: usernames are concatenated and delimited by a space.
client to serverM: usernames are concatenated and delimited by a space.
serverM to serverA/B and client: usernames are concatenated and delimited by a comma.
serverA/B to serverM: intervals are formed as "[start,end]" and are encapsulated in a pair of brackets "[]". If it's empty, it's "[]".
serverM to client: intervals are formed as "[start,end]" and are encapsulated in a pair of brackets "[]". If it's empty, it's "[]".

It may show "M fails to bind." message, this is because some zombie processes are created while testing codes, please make sure to kill them and then the codes will work well. (Or wait for some time and restart)
