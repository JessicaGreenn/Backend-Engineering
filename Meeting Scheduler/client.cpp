#include <iostream>
#include <fstream>
#include <map> 
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
#include <set>

using namespace std;

#define BOTHIP "127.0.0.1"

#define MAXBUFLEN 1000
#define serverTcpPORT 24475

vector<string> names;
int prtSocket;
struct sockaddr_in serverMaddr;
struct sockaddr_in clientAddr;
unsigned short clientPortNum;
char buf[MAXBUFLEN];
string person;
set<string> notDisplay;
char timebuf[MAXBUFLEN];
string meeting;
vector<pair<int,int> > timeSecs;
// string nonExists = "";

// convert string to int
int toInt(string num){
    int res = 0;
    for(int i = 0; i < num.size(); i++){
        res = res * 10 + (int) (num[i] - '0');
    }

    return res;
}

// Modified from https://blog.csdn.net/qian2213762498/article/details/81705647
void stripClient(string &s){
    if(!s.empty()){
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}


void storeNames(string person){
    string tmp = person;
    
    int p1 = 0;

    names.clear();
    while((p1 = tmp.find(" ")) != string::npos){
        names.push_back(tmp.substr(0, p1));
		tmp.erase(0, p1 + 1);

    }

    names.push_back(tmp);

}

void storeNonExists(string nonExists){
    if(nonExists == ""){
        return;
    }
    string tmp = nonExists;
    
    int p1 = 0;
    notDisplay.clear();
    while((p1 = tmp.find(",")) != string::npos){
        notDisplay.insert(tmp.substr(0, p1));
		tmp.erase(0, p1 + 1);

    }

    notDisplay.insert(tmp);
    // cout << "notDisplay size: " << notDisplay.size() << endl;
}

// get sockaddr, IPv4 or IPv6:
// Modified from Beej's code
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Modified from Beej's code
void createParentSocket(){
    prtSocket = socket(AF_INET, SOCK_STREAM, 0);
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(BOTHIP);
	// dinamically assigned ClientPortNum
    clientAddr.sin_port = 0; 

	if(prtSocket == -1){
        cout << "Client fails to create parent socket." << endl;
        exit(1);
    }

}

// Modified from Beej's code
void setAddr(){
    memset(&serverMaddr, '\0', sizeof(serverMaddr));
    serverMaddr.sin_family = AF_INET;
    serverMaddr.sin_addr.s_addr = inet_addr(BOTHIP);
    serverMaddr.sin_port = htons(serverTcpPORT);
}

// Modified from Beej's code
void connectThem(){
	int isConnected;
    isConnected = connect(prtSocket, (struct sockaddr *)&serverMaddr, sizeof(struct sockaddr_in));
	if(isConnected < 0){
        cout << "Client fails to connect." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void sendAndReceive(){
    int sendNames = send(prtSocket, person.c_str(), person.length(), 0);
    if(sendNames == -1){
        cout << "Client fails to send the input usernames. " << endl;
    }else{
        cout << "Client finished sending the usernames to Main Server." << endl;
    }

    int numbytes;
    memset(buf, '\0', sizeof(buf));

	if ((numbytes = recv(prtSocket, buf, MAXBUFLEN-1, 0)) == -1) {
	    cout << "Client fails to receive the non-exist usernames. " << endl;
	    exit(1);
	}

	buf[numbytes] = '\0';
    // nonExists = buf;
    // cout << buf << endl;
}

// Modified from Beej's code
void receiveIntervals(){
    int numbytes;

    memset(timebuf, '\0', sizeof(timebuf));
    if ((numbytes = recv(prtSocket, timebuf, MAXBUFLEN-1, 0)) == -1) {
	    cout << "Client fails to receive the result. " << endl;
	    exit(1);
	}

	timebuf[numbytes] = '\0';

    // if ((numbytes = recv(prtSocket, buf, MAXBUFLEN-1, 0)) == -1) {
	//     cout << "Client fails to receive the result. " << endl;
	//     exit(1);
	// }

	// buf[numbytes] = '\0';
}

bool isValidTime(string &meeting, string avlb){
    stripClient(meeting);
    string leftStr = meeting.substr(meeting.find('[') + 1, meeting.find(',') - 1);
    // cout << "leftStr: " << leftStr << endl;
    stripClient(leftStr);
    int l = toInt(leftStr);
    
    meeting.pop_back();
    string rightStr = meeting.substr(meeting.find(',') + 1);
    // cout << "rightStr: " << rightStr << endl;
    stripClient(rightStr);
    int r = toInt(rightStr);

    meeting = "[" + leftStr + "," + rightStr + "]";

    if(l >= r){
        return false;
    }
    
    string tmp = avlb;
    if(tmp == "[]"){
        return false;
    }
	tmp.erase(tmp.begin());
    tmp.pop_back();
    tmp.erase(tmp.begin());
    tmp.pop_back();
	// cout << "tmp: " << tmp << endl;

	int timeIdx = 0;
    string left = "";
    string right = "";
	string dlmt = "],[";
	string curr = "";
	// timeSecs.clear();
    while((timeIdx = tmp.find(dlmt)) != string::npos){
        curr = tmp.substr(0, timeIdx);
        left = curr.substr(0, curr.find(','));
        right = curr.substr(curr.find(',') + 1);
        pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
        // cout << "(" << tmp1.first << "," << tmp1.second << ")" << endl;
        if(tmp1.second <= l){ 
            // scheduled time goes after this interval
            tmp.erase(0, timeIdx + dlmt.size());
    		continue;
        }else if(tmp1.first >= r){ 
            // scheduled time goes before this interval
    		return false;
        }else{ 
            if(l >= tmp1.first && r <= tmp1.second){
                // scheduled time goes within this interval
                return true;
            }else{
                // scheduled time exceeds this interval
                return false;
            }
    		
    	}
        
            
    }
        
    curr = tmp;
    left = curr.substr(0, curr.find(','));
    right = curr.substr(curr.find(',') + 1);
    pair<int, int> tmp1 = make_pair(toInt(left), toInt(right));
    if(tmp1.second <= l){ 
        // scheduled time goes after this interval
    	return false;
    }else if(tmp1.first >= r){ 
        // scheduled time goes before this interval
    	return false;
    }else{ 
        if(l >= tmp1.first && r <= tmp1.second){
            // scheduled time goes within this interval
            return true;
        }else{
            // scheduled time exceeds this interval
            return false;
        }
    		
    }


}

int main(){

    createParentSocket();
    setAddr();
    connectThem();

    // Modified from Beej's code
	socklen_t addr_len = sizeof(clientAddr);
    getsockname(prtSocket, (struct sockaddr *)&clientAddr, &addr_len);
    // the dynamically assigned port number
    clientPortNum = ntohs(clientAddr.sin_port);

	cout << "Client is up and running." << endl;

    while(1){
    	cout << "Please enter the usernames to check schedule availability:" << endl;
        // store input usernames
    
        getline(cin, person);
        storeNames(person);
        // cout << names.at(2) << endl;

        sendAndReceive();

        string nonExists = buf;
        nonExists.pop_back();
        if(nonExists == ""){
            nonExists = "No one";
        }
	    cout << "Client received the reply from Main Server using TCP over port " << clientPortNum << ": " << nonExists << " do not exist." << endl;
        storeNonExists(nonExists);

        string display = "";
        for(int i = 0; i < names.size(); i++){
            if(notDisplay.count(names[i])){
                continue;
            }
            display += names[i] + ",";
        }
        if(display != ""){
            display.pop_back();
        }
        if(display == ""){

            cout << "-----Start a new request-----" << endl;
            continue;
        }


        receiveIntervals();


        cout << "Client received the reply from Main Server using TCP over port " << clientPortNum << ": " << "Time intervals " << timebuf << " works for " << display << "." << endl;
        // cout << "Client received the reply from Main Server using TCP over port " << clientPortNum << ": " << "Time intervals " << buf << " works for " << display << "." << endl;

        
        //*******************extra points***********************
        cout << "Please enter the final meeting time to register an meeting:" << endl;
        getline(cin, meeting);
        string avlb = timebuf;
        while (!isValidTime(meeting, avlb)){
            cout << "Time interval " << meeting << " is not valid. Please enter again:" << endl;
            getline(cin, meeting);
        }
        
        int sendNames = send(prtSocket, meeting.c_str(), meeting.length(), 0);
        if(sendNames == -1){
            cout << "Client fails to send the scheduled interval. " << endl;
        }else{
            cout << "Sent the request to register " << meeting << " as the meeting time for " << display << "." << endl;
        }

        int numbytes;
        while(1){
            memset(timebuf, '\0', sizeof(timebuf));
            if ((numbytes = recv(prtSocket, timebuf, MAXBUFLEN-1, 0)) == -1) {
	            cout << "Client fails to receive the result. " << endl;
	            exit(1);
	        }
            break;

        }
        string update = timebuf;
        if(update == "U"){
            cout << "Received the notification that registration has finished." << endl;
        }
        
	    buf[numbytes] = '\0';

        //*******************extra points***********************
        


        cout << "-----Start a new request-----" << endl;
        memset(timebuf, '\0', sizeof(timebuf));
        memset(buf, '\0', sizeof(buf));

    }


	return 0;
}