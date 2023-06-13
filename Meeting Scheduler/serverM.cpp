#include <iostream>
#include <fstream>
#include <set> 
#include <vector>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>


using namespace std;

#define SERVERSIP "127.0.0.1"
#define MUPN 23475
#define SERVERM_UDP_PORT "23475"
#define MTPN 24475
#define SERVERM_TCP_PORT "24475"
#define APN 21475
#define BPN 22475
#define MAXBUFLEN 50000
#define MAXTCPBUFLEN 1000
#define BACKLOG 10

struct sockaddr_in serverMUdpAddr;
struct sockaddr_in serverMTcpAddr;
struct sockaddr_in serverAaddr;
struct sockaddr_in serverBaddr;
struct sockaddr_in clientAddr;

set<string> namesA;
set<string> namesB;

int socketToAB;
char buf[MAXBUFLEN];
char tcpbuf[MAXTCPBUFLEN];
int prtSocket;
int cldSocket;
string forA = "";
string forB = "";
char bufA[MAXBUFLEN];
char bufB[MAXBUFLEN];
vector<pair<int,int> > intervalsA;
vector<pair<int,int> > intervalsB;
vector<pair<int,int> > totalIntervals;
string toClient = "";

// convert string to int
int toInt(string num){
    int res = 0;
    for(int i = 0; i < num.size(); i++){
        res = res * 10 + (int) (num[i] - '0');
    }

    return res;
}
// Modified from Beej's code
void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// Modified from https://blog.csdn.net/qian2213762498/article/details/81705647
void stripM(string &s){
    if(!s.empty()){
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}

// Modified from Beej's code
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Modified from Beej's code
void createUdpSocket(){
	socketToAB = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketToAB== -1){
        cout << "A fails to create socket to M." << endl;
		exit(1);
    }
}

// Modified from Beej's code
void setAddrs(){
	memset(&serverMUdpAddr, 0, sizeof(serverMUdpAddr));
    serverMUdpAddr.sin_family = AF_INET;
    serverMUdpAddr.sin_addr.s_addr = inet_addr(SERVERSIP);
    serverMUdpAddr.sin_port = htons(MUPN);

	memset(&serverAaddr, 0, sizeof(serverAaddr));
    serverAaddr.sin_family = AF_INET;
    serverAaddr.sin_addr.s_addr = inet_addr(SERVERSIP);
    serverAaddr.sin_port = htons(APN);

	memset(&serverBaddr, 0, sizeof(serverBaddr));
    serverBaddr.sin_family = AF_INET;
    serverBaddr.sin_addr.s_addr = inet_addr(SERVERSIP);
    serverBaddr.sin_port = htons(BPN);
}

// Modified from Beej's code
void bindUdpSocket(){
	if(::bind(socketToAB, (struct sockaddr *) &serverMUdpAddr, sizeof(serverMUdpAddr)) == -1){
        cout << "M fails to bind." << endl;
        exit(1);
    }
}

void recvAnames(){

	// receive A's string
	// Modified from Beej's code
	int numbytes;
	socklen_t addr_len;
	while(1){
		addr_len = sizeof serverAaddr;
		memset(buf, '\0', sizeof(buf));
		if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverAaddr, &addr_len)) == -1) {
			cout << "M fails to receive names from A." << endl;
			exit(1);
		}

		break;

	}
	// cout << buf << endl;
	// store A's usernames
	string tmp = buf;
	int nameIdx = 0;
	while((nameIdx = tmp.find(' ')) != string::npos){
		namesA.insert(tmp.substr(0, nameIdx));
		tmp.erase(0, nameIdx + 1);

	}

	// cout << "Is jameson in namesA? " << namesA.count("jameson") << endl;
    
}

void recvBnames(){

	// receive B's string
	// Modified from Beej's code
	int numbytes;
	socklen_t addr_len;
	while(1){
		addr_len = sizeof serverBaddr;
		memset(buf, '\0', sizeof(buf));
		if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverBaddr, &addr_len)) == -1) {
			cout << "M fails to receive names from B." << endl;
			exit(1);
		}

		break;

	}
	
	// store B's usernames
	string tmp = buf;
	int nameIdx = 0;
	while((nameIdx = tmp.find(' ')) != string::npos){
		namesB.insert(tmp.substr(0, nameIdx));
		tmp.erase(0, nameIdx + 1);

	}

	// cout << "Is silas in namesB? " << namesB.count("silas") << endl;
    
}

// Modified from Beej's code
void createParentSocket(){
	// create parent socket
	prtSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(prtSocket == -1){
        cout << "M fails to create parent socket." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void setClientAddr(){
	// set addr
	memset(&serverMTcpAddr, '\0', sizeof(serverMTcpAddr));
    serverMTcpAddr.sin_family = AF_INET;
    serverMTcpAddr.sin_addr.s_addr = INADDR_ANY;
    serverMTcpAddr.sin_port = htons(MTPN);
}

// Modified from Beej's code
void bindParentSocket(){
	// bind()
	if(::bind(prtSocket, (struct sockaddr *)&serverMTcpAddr, sizeof(serverMTcpAddr)) == -1){
        cout << "M fails to bind." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void listenTo(){
	// listen()
	if(listen(prtSocket, BACKLOG) == -1){
        cout << "M fails to listen." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void createChildSocketAndAccept(){
	// create child socket for the client
	socklen_t addr_len = sizeof clientAddr;
    // accept()
    // cite Beej code: accept function
	cldSocket = accept(prtSocket, (struct sockaddr *)&clientAddr, &addr_len);
	if (cldSocket == -1) {
		cout << "M fails to accept." << endl;
		exit(1);
	}
}

void receiveAndSendNames(){
	// receive through child socket
	// Modified from Beej's code
    // memset(tcpbuf, '\0', MAXTCPBUFLEN);
    int rec;
	// memset(tcpbuf, '\0', sizeof(tcpbuf));
    rec = recv(cldSocket, tcpbuf, MAXTCPBUFLEN, 0);
    if(rec <= 0){
        cout << "M fails to receive input names from client." << endl;
        exit(1);
    }
	tcpbuf[rec] = '\0';

	cout << "Main Server received the request from client using TCP over port " << MTPN << "." << endl;        

    string tmp = "";
	tmp = tcpbuf;
	string forClient = "";
	string dlmt1 = ",";
	string curr;
	int p1 = 0;
	forA = "";
	forB = "";
	forClient = "";
    while((p1 = tmp.find(" ")) != string::npos){
        curr = tmp.substr(0, p1);
		if(namesA.count(curr) == 1){
			forA += curr + dlmt1;
		}else if(namesB.count(curr) == 1){
			forB += curr + dlmt1;
		}else{
			forClient += curr + dlmt1;
		}
		tmp.erase(0, p1 + 1);
		


    }

    curr = tmp;
    // cout << tmp <<endl;

	if(namesA.count(curr) == 1){
		forA += curr + dlmt1;
		// cout << forA << endl;
	}else if(namesB.count(curr) == 1){
		forB += curr + dlmt1;
		// cout << forB << endl;
	}else{
		forClient += curr + dlmt1;
		// cout << forClient << endl;
	}
	if(forA == ""){
		forA = ",";
	}
	if(forB == ""){
		forB = ",";
	}
	if(forClient == ""){
		forClient = ",";
	}

	// cout <<"forClient:"<< forClient << endl;
	// cout << "forA:" << forA << endl;
	// cout << "forB:" << forB << endl;


	forA.pop_back();
	forB.pop_back();
	forClient.pop_back();
	forA += "Y";
	forB += "Y";
	forClient = forClient + "Y";
	
	// cout << "forClient: " << forClient << endl;
	// cout << "forA: " << forA << endl;
	// cout << "forB: " << forB << endl;
	
	// send non-existed names to the client
	if (send(cldSocket, forClient.c_str(), forClient.length(), 0) == -1){
		cout << "M fails to send non-existed names to the client." << endl;
		exit(1);
	}else{
		forClient.pop_back();
		if(forClient == ""){
			cout << "No one do not exist. Send a reply to the client." << endl;
		}else{
			cout << forClient << " do not exist. Send a reply to the client." << endl;

		}
	}


	// send A's usernames 
	// Modified from Beej's code
    int numbytes;
    if ((numbytes = sendto(socketToAB, forA.c_str(), forA.length(), 0, (const struct sockaddr *) &serverAaddr, sizeof(serverAaddr))) == -1) {
		cout << "M fails to send names to A." << endl;
		exit(1);
	}
	forA.pop_back();
	if(forA != ""){
		cout << "Found " << forA << " located at Server A. Send to Server A." << endl;
	}
	
	// send B's usernames 
	// Modified from Beej's code
    if ((numbytes = sendto(socketToAB, forB.c_str(), forB.length(), 0, (const struct sockaddr *) &serverBaddr, sizeof(serverBaddr))) == -1) {
		cout << "M fails to send names to B." << endl;
		exit(1);
	}
	forB.pop_back();
	if(forB != ""){
		cout << "Found " << forB << " located at Server B. Send to Server B." << endl;

	}
}

// Modified code from https://cloud.tencent.com/developer/article/1787844
void findOverlap(vector<pair<int,int> > a, vector<pair<int,int> > b){
    int m = a.size();
    int n = b.size();
    int i = 0;
    int j = 0;
    int left;
    int right;
    totalIntervals.clear();
	// cout << "total size after clear: " << totalIntervals.size() << endl;
    pair<int, int> intersec;
    while(i < m && j < n){
    	if(a[i].second <= b[j].first){ 
            //a's time is earlier
    		i++;
        }else if(a[i].first >= b[j].second){ 
            //b's time is earlier
    		j++;
        }else{ 
    		//a and b have intersection
    		left = max(a[i].first, b[j].first);
    		right = min(a[i].second, b[j].second);
    		intersec = make_pair(left, right);
            totalIntervals.push_back(intersec);
    		if(a[i].second >= b[j].second)
    			j++;//a has more time, continue to search b
    		else
    			i++;//b has more time, continue to search a
    	}
    }

	// cout << "total size: " << totalIntervals.size() << endl;

}

void receiveIntervalsA(int numbytes, socklen_t addr_len){
	while(1){
		// Modified from Beej's code
		addr_len = sizeof serverAaddr;
		// memset(bufA, '\0', sizeof(bufA));
		if ((numbytes = recvfrom(socketToAB, bufA, MAXBUFLEN-1 , 0, 
		(struct sockaddr *) &serverAaddr, &addr_len)) == -1) {
			cout << "M fails to receive intervals from A." << endl;
			exit(1);
		}
		bufA[numbytes] = '\0';
		break;

	}
	cout << "Main Server received from server A the intersection result using UDP over port "<< MUPN << ": " << bufA << "." << endl;
	// store A's intervals

	string tmpA = bufA;
	if(tmpA == "[]"){
		return;
	}
	tmpA.erase(tmpA.begin());
    tmpA.pop_back();
    tmpA.erase(tmpA.begin());
    tmpA.pop_back();
	// cout << "tmpA: " << tmpA << endl;

	int timeIdx = 0;
    string left = "";
    string right = "";
	string dlmt = "],[";
	string curr = "";
	intervalsA.clear();
    while((timeIdx = tmpA.find(dlmt)) != string::npos){
        curr = tmpA.substr(0, timeIdx);
        left = curr.substr(0, curr.find(','));
        right = curr.substr(curr.find(',') + 1);
        pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
        intervalsA.push_back(tmp1);
        tmpA.erase(0, timeIdx + dlmt.size());
            
    }
        
    curr = tmpA;
    left = curr.substr(0, curr.find(','));
    right = curr.substr(curr.find(',') + 1);
    pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
    intervalsA.push_back(tmp1);

	// cout << "intervalsA size: " << intervalsA.size() << endl;

	// if(intervalsA.size() == 0){
	// 	cout << "No intervals in A." << endl;
	// }else{
	// 	for(int i = 0; i < intervalsA.size(); i++){
	// 		cout << "(" << intervalsA[i].first << "," << intervalsA[i].second << ") "; 
	// 	}
	// 	cout << endl;
	// }
}

void receiveIntervalsB(int numbytes, socklen_t addr_len){
	while(1){
		// Modified from Beej's code
		addr_len = sizeof serverAaddr;
		// memset(bufB, '\0', sizeof(bufB));
		if ((numbytes = recvfrom(socketToAB, bufB, MAXBUFLEN-1 , 0, 
		(struct sockaddr *) &serverBaddr, &addr_len)) == -1) {
			cout << "M fails to receive intervals from B." << endl;
			exit(1);
		}
		bufB[numbytes] = '\0';
		break;

	}
	
	cout << "Main Server received from server B the intersection result using UDP over port "<< MUPN << ": " << bufB << "." << endl;

	// store B's intervals
	string tmpB = bufB;
	if(tmpB == "[]"){
		return;
	}
	tmpB.erase(tmpB.begin());
    tmpB.pop_back();
    tmpB.erase(tmpB.begin());
    tmpB.pop_back();
	// cout << "tmpB: " << tmpB << endl;

	int timeIdx = 0;
	string dlmt = "],[";
    string left = "";
    string right = "";
	string curr = "";
	intervalsB.clear();
    while((timeIdx = tmpB.find(dlmt)) != string::npos){
        curr = tmpB.substr(0, timeIdx);
        left = curr.substr(0, curr.find(','));
        right = curr.substr(curr.find(',') + 1);
        pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
        intervalsB.push_back(tmp1);
        tmpB.erase(0, timeIdx + dlmt.size());
            
    }
        
    curr = tmpB;
    left = curr.substr(0, curr.find(','));
    right = curr.substr(curr.find(',') + 1);
    pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
    intervalsB.push_back(tmp1);

	// cout << "intervalsB size: " << intervalsB.size() << endl;

	// if(intervalsB.size() == 0){
	// 	cout << "No intervals in B." << endl;
	// }else{
	// 	for(int i = 0; i < intervalsB.size(); i++){
	// 		cout << "(" << intervalsB[i].first << "," << intervalsB[i].second << ") "; 
	// 	}
	// 	cout << endl;
	// }
}

int main() {


	createUdpSocket();
    setAddrs();
	bindUdpSocket();
	cout << "Main Server is up and running." << endl;
	recvAnames();
	cout << "Main Server received the username list from serverA using UDP over port " << MUPN << '.' << endl;
	recvBnames();
	cout << "Main Server received the username list from serverB using UDP over port " << MUPN << '.' << endl;

	createParentSocket();
	setClientAddr();
	bindParentSocket();
	listenTo();
	createChildSocketAndAccept();

	while(1){

		// createChildSocketAndAccept();
		receiveAndSendNames();

		// receive A's intervals
		int numbytes;
		socklen_t addr_len;
		
		if(forA == "" && forB == ""){
			// string dum = "[]";
			// if (send(cldSocket, dum.c_str(), dum.length(), 0) == -1){
			// 	cout << "M fails to send intervals to the client." << endl;
			// 	exit(1);
			// }else{
			// cout << "No valid username is provided. Please start a new request." << endl;
			// }
			continue;
		}

		if(forA == ""){
			receiveIntervalsB(numbytes, addr_len);
			cout << "Found the intersection between the results from server A and B: " << bufB << endl;

			// send intervals to the client
			usleep(50000);
			string tmpbufB = bufB;
			if (send(cldSocket, tmpbufB.c_str(), tmpbufB.length(), 0) == -1){
				cout << "M fails to send intervals to the client." << endl;
				exit(1);
			}else{
				cout << "Main Server sent the result to the client." << endl;

			}

			//*******************extra points***********************
			// receive through child socket
			// Modified from Beej's code
    		int rec;
			memset(tcpbuf, '\0', sizeof(tcpbuf));
    		rec = recv(cldSocket, tcpbuf, MAXTCPBUFLEN, 0);
    		if(rec <= 0){
        		cout << "M fails to receive the scheduled interval from client." << endl;
        		exit(1);
    		}

			cout << "Main Server received the scheduled interval from client using TCP over port " << MTPN << "." << endl;

			string clientInterval = tcpbuf;

    		if ((numbytes = sendto(socketToAB, clientInterval.c_str(), clientInterval.length(), 0, (const struct sockaddr *) &serverBaddr, sizeof(serverBaddr))) == -1) {
				cout << "M fails to send the scheduled interval to B." << endl;
				exit(1);
			}

			//recv the update message from B
			addr_len = sizeof serverBaddr;
			memset(buf, '\0', sizeof(buf));
			if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverBaddr, &addr_len)) == -1) {
				cout << "M fails to receive update message from B." << endl;
				exit(1);
			}

			// string resB = buf;

			// if(resB == "U"){

			// 	cout << "Server B finished updating. Main server notified the client to start a new request." << endl;
			// }

			// send update message to the client
			string update = "U";
			if (send(cldSocket, update.c_str(), update.length(), 0) == -1){
				cout << "M fails to send intervals to the client." << endl;
				exit(1);
			}else{
				cout << "Notified client that registration has finished." << endl;
			}
			//*******************extra points***********************

			continue;

		}

		if(forB == ""){
			receiveIntervalsA(numbytes, addr_len);
			cout << "Found the intersection between the results from server A and B: " << bufA << endl;

			// send intervals to the client
			string tmpbufA = bufA;
			usleep(50000);
			if (send(cldSocket, tmpbufA.c_str(), tmpbufA.length(), 0) == -1){
				cout << "M fails to send intervals to the client." << endl;
				exit(1);
			}else{
				cout << "Main Server sent the result to the client." << endl;
			}

			//*******************extra points***********************
			// receive through child socket
			// Modified from Beej's code
    		int rec;
			memset(tcpbuf, '\0', sizeof(tcpbuf));
    		rec = recv(cldSocket, tcpbuf, MAXTCPBUFLEN, 0);
    		if(rec <= 0){
        		cout << "M fails to receive the scheduled interval from client." << endl;
        		exit(1);
    		}

			cout << "Main Server received the scheduled interval from client using TCP over port " << MTPN << "." << endl;

			string clientInterval = tcpbuf;

    		if ((numbytes = sendto(socketToAB, clientInterval.c_str(), clientInterval.length(), 0, (const struct sockaddr *) &serverAaddr, sizeof(serverAaddr))) == -1) {
				cout << "M fails to send the scheduled interval to A." << endl;
				exit(1);
			}

			//recv the update message from B
			addr_len = sizeof serverAaddr;
			memset(buf, '\0', sizeof(buf));
			if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverAaddr, &addr_len)) == -1) {
				cout << "M fails to receive update message from A." << endl;
				exit(1);
			}

			// string resA = buf;

			// if(resA == "U"){

			// 	cout << "Server A finished updating. Main server notified the client to start a new request." << endl;
			// }

			// send update message to the client
			string update = "U";
			if (send(cldSocket, update.c_str(), update.length(), 0) == -1){
				cout << "M fails to send intervals to the client." << endl;
				exit(1);
			}else{
				cout << "Notified client that registration has finished." << endl;
			}
			//*******************extra points***********************
			continue;

		}

		
		receiveIntervalsA(numbytes, addr_len);

		// receive B's intervals
	
		receiveIntervalsB(numbytes, addr_len);

		// find the final intersection result
		findOverlap(intervalsA, intervalsB);
		// cout << "intervals size: " << totalIntervals.size() << endl;
		toClient = "";
		if(totalIntervals.size() == 0){
			toClient = ",";
			
        	// cout << forA << "," << forB << " do not have intersections." << endl;
    	}else{
			
        	for(int i = 0; i < totalIntervals.size(); i++){
            	toClient += "[" + to_string(totalIntervals.at(i).first) + "," + to_string(totalIntervals.at(i).second) + "],";
        	}
    	}
    	toClient.pop_back();
    	toClient = "[" + toClient + "]";

    	cout << "Found the intersection between the results from server A and B: " << toClient << "." << endl;

		// send intervals to the client
		usleep(50000);
		if (send(cldSocket, toClient.c_str(), toClient.length(), 0) == -1){
			cout << "M fails to send intervals to the client." << endl;
			exit(1);
		}else{
			cout << "Main Server sent the result to the client." << endl;
		}

		//*******************extra points***********************
		// receive through child socket
		// Modified from Beej's code
    	int rec;
		memset(tcpbuf, '\0', sizeof(tcpbuf));
    	rec = recv(cldSocket, tcpbuf, MAXTCPBUFLEN, 0);
    	if(rec <= 0){
        	cout << "M fails to receive registration from client." << endl;
        	exit(1);
    	}

		cout << "Main Server received registration from client using TCP over port " << MTPN << "." << endl;

		string clientInterval = tcpbuf;

		// stripM(clientInterval);
		

    	if ((numbytes = sendto(socketToAB, clientInterval.c_str(), clientInterval.length(), 0, (const struct sockaddr *) &serverAaddr, sizeof(serverAaddr))) == -1) {
			cout << "M fails to send registration to A." << endl;
			exit(1);
		}else{
			cout << "M sent registration to A." << endl;
		}

		if ((numbytes = sendto(socketToAB, clientInterval.c_str(), clientInterval.length(), 0, (const struct sockaddr *) &serverBaddr, sizeof(serverBaddr))) == -1) {
			cout << "M fails to send registration to B." << endl;
			exit(1);
		}else{
			cout << "M sent registration to B." << endl;
		}

		addr_len = sizeof serverAaddr;
		memset(buf, '\0', sizeof(buf));
		if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverAaddr, &addr_len)) == -1) {
			cout << "M fails to receive update message from A." << endl;
			exit(1);
		}
		string resA = buf;

		addr_len = sizeof serverBaddr;
		memset(buf, '\0', sizeof(buf));
		if ((numbytes = recvfrom(socketToAB, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverBaddr, &addr_len)) == -1) {
			cout << "M fails to receive update message from B." << endl;
			exit(1);
		}

		string resB = buf;

		// if(resA == "U" && resB == "U"){

		// 	cout << "Server A and B finished updating. Main server notified the client to start a new request." << endl;
		// }

		// send update message to the client
		string update = "U";
		if (send(cldSocket, update.c_str(), update.length(), 0) == -1){
			cout << "M fails to send intervals to the client." << endl;
			exit(1);
		}else{
			cout << "Notified the client that registration has finished." << endl;
		}

		//*******************extra points***********************



		

	}
	
    return 0;
}