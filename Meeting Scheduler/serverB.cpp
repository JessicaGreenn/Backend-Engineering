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


using namespace std;

#define SERVERSIP "127.0.0.1"
#define BPN 22475
#define MPN 23475
#define MAXBUFLEN 1000

string toM = "";
map<string, vector<pair<int, int> > > bFile;
int socketToM;
struct sockaddr_in serverBaddr;
struct sockaddr_in serverMaddr;
char buf[MAXBUFLEN];
string intervalsToM = "";
bool noNames;
string dummy;
vector<string> names;

// convert string to int
int toInt(string num){
    int res = 0;
    for(int i = 0; i < num.size(); i++){
        res = res * 10 + (int) (num[i] - '0');
    }

    return res;
}

// Modified from https://blog.csdn.net/qian2213762498/article/details/81705647
void stripB(string &s){
    if(!s.empty()){
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}

// process input to send to server M and be stored in server A
void fileProcess(ifstream &txt){
    string person;
    // string serverName = "B";
    string dlmt = "]";
    string name = "";   
    string times = "";
    string time = "";
    
    while(getline(txt, person)){

        // construct toM
        int idx = person.find(';');
        // cout << "posiiton of ; " << idx << endl;
        name = person.substr(0, idx);
        stripB(name);
        // cout << name << " " << serverName << endl;
        toM += name + " ";

        // construct aFile
        times = person.substr(idx + 1);
        // erase "[" and "]"
        stripB(times);
        times.erase(times.begin());
        times.pop_back();
        stripB(times);
        times.erase(times.begin());
        times.pop_back();
        // cout << times << endl;
        vector<pair<int, int> > inters;
        int p = 0;
        string left = "";
        string right = "";
        while(((p = times.find(dlmt)) != string::npos)){
            time = times.substr(0, p);
            // cout << "times:" + times << endl;
            left = time.substr(0, time.find(','));
            stripB(left);
            right = time.substr(time.find(',') + 1);
            stripB(right);
            // cout << "time:" + time << ',' << "left:" + left << ',' << "right" + right;
            pair<int, int> tmp = make_pair(toInt(left), toInt(right));
            inters.push_back(tmp);
            // cout << p + dlmt.size() << endl;
            times.erase(0, times.find('[') + 1);
            
        }
        // cout << endl;
        time = times.substr(0, p);
        left = time.substr(0, time.find(','));
        stripB(left);
        right = time.substr(time.find(',') + 1);
        stripB(right);
        pair<int, int> tmp = make_pair(toInt(left), toInt(right));
        inters.push_back(tmp);
        
        bFile[name] = inters;

        

    }
    // toM = "B" + toM;

    
}

// Modified from Beej's code
void createSocket(){
    socketToM = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketToM== -1){
        cout << "B fails to create socket to M." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void setAddrs(){
    memset(&serverBaddr, 0, sizeof(serverBaddr));
    serverBaddr.sin_family = AF_INET;
    serverBaddr.sin_addr.s_addr = inet_addr(SERVERSIP);
    serverBaddr.sin_port = htons(BPN);

    memset(&serverMaddr, 0, sizeof(serverMaddr));
    serverMaddr.sin_family = AF_INET;
    serverMaddr.sin_addr.s_addr = inet_addr(SERVERSIP);
    serverMaddr.sin_port = htons(MPN);
}

// Modified from Beej's code
void bindSocket(){
    if(::bind(socketToM, (struct sockaddr *) &serverBaddr, sizeof(serverBaddr)) == -1){
        cout << "A fails to bind." << endl;
        exit(1);
    }
}

// Modified from Beej's code
void sendNames(){
    // send A's usernames 
    int numbytes;
    if ((numbytes = sendto(socketToM, toM.c_str(), toM.length(), 0, (const struct sockaddr *) &serverMaddr, sizeof(serverMaddr))) == -1) {
		cout << "B fails to send names to M." << endl;
		exit(1);
	}
    
}

// Modified code from https://cloud.tencent.com/developer/article/1787844
vector<pair<int,int> > findOverlap(vector<pair<int,int> > a, vector<pair<int,int> > b){
    int m = a.size();
    int n = b.size();
    int i = 0;
    int j = 0;
    int left;
    int right;
    vector<pair<int,int> > res;
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
            res.push_back(intersec);
    		if(a[i].second >= b[j].second)
    			j++;//a has more time, continue to search b
    		else
    			i++;//b has more time, continue to search a
    	}
    }
    return res;
}

void recvAndFindIntervals(){
    int numbytes;
	socklen_t addr_len;
    while(1){
        addr_len = sizeof serverMaddr;
        memset(buf, '\0', sizeof(buf));
        // Modified from Beej's code
		if ((numbytes = recvfrom(socketToM, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverMaddr, &addr_len)) == -1) {
			cout << "B fails to receive names from M." << endl;
			exit(1);
		}

		break;
    }
    // names received from M
    // cout << buf << endl;

    string tmp = buf;
    tmp.pop_back();
    dummy = tmp;
    vector<pair<int,int> > intersecs;
    if(tmp == ""){
        // cout << "No names are sent to B." << endl;
        intervalsToM = "[]";
        memset(buf, '\0', sizeof(buf));
        noNames = true;
        return;
    }

    cout << "Server B received the usernames from Main Server using UDP over port " << BPN << "." << endl;

	string dlmt1 = ",";
	string curr;
	int p1 = tmp.find(",");
    names.clear();
    // Only one name is sent to B
    if(p1 == string::npos){
        names.push_back(tmp);
        intersecs = bFile[tmp];
    }else{
        string pre = tmp.substr(0, p1);
        intersecs = bFile[pre];
        names.push_back(pre);
        tmp.erase(0, p1 + 1);
        while((p1 = tmp.find(",")) != string::npos){
            
            curr = tmp.substr(0, p1);
            names.push_back(curr);
            intersecs = findOverlap(intersecs, bFile[curr]);
		    tmp.erase(0, p1 + 1);

        }
        names.push_back(tmp);
        intersecs = findOverlap(intersecs, bFile[tmp]);


    }
    if(intersecs.size() == 0){
        cout << dummy << " do not have intersections." << endl;
        intervalsToM = "[]";
        return;
    }else{
        for(int i = 0; i < intersecs.size(); i++){
            intervalsToM += "[" + to_string(intersecs.at(i).first) + "," + to_string(intersecs.at(i).second) + "],";
        }
    }

    intervalsToM.pop_back();
    intervalsToM = "[" + intervalsToM + "]";
    
}

// Modified from Beej's code
void sendIntervals(){
    // send B's users' intervals 
    int numbytes;
    if ((numbytes = sendto(socketToM, intervalsToM.c_str(), intervalsToM.length(), 0, (const struct sockaddr *) &serverMaddr, sizeof(serverMaddr))) == -1) {
		cout << "B fails to send intervals to M." << endl;
		exit(1);
	}
}

string itoString(vector<pair<int, int> > vals){
    string res = "";
	if(vals.size() == 0){
		res = ",";
			
    }else{
			
        for(int i = 0; i < vals.size(); i++){
            res += "[" + to_string(vals.at(i).first) + "," + to_string(vals.at(i).second) + "],";
        }
    }
    res.pop_back();
    res = "[" + res + "]";
    return res;
}

void deleteTime(string name, int l, int r){
    // vector<pair<int, int> > vals = bFile[name];
    for(int i = 0; i < bFile[name].size(); i++){
        if(bFile[name][i].second <= l){
            continue;
        }else if(bFile[name][i].first >= r){
            break;
        }else{
            if(l == bFile[name][i].first && r == bFile[name][i].second){
                bFile[name].erase(bFile[name].begin() + i);
            }else if(l == bFile[name][i].first){
                bFile[name][i].first = r;
            }else if(r == bFile[name][i].second){
                bFile[name][i].second = l;
            }else{
                pair<int, int> newone = make_pair(bFile[name][i].first, l);
                bFile[name][i].first = r;
                bFile[name].insert(bFile[name].begin() + i, newone);
            }
        }
    }

}

int main() {
    cout << "Server B is up and running using UDP on port " << BPN << '.' << endl;

    ifstream file("b.txt"); 
    fileProcess(file);
    createSocket();
    setAddrs();
    bindSocket();
    sendNames();
    cout << "Server B finished sending a list of usernames to Main Server." << endl;
    while(1){
        intervalsToM = "";
        noNames = false;
        recvAndFindIntervals();
        if(noNames){
            continue;
        }
        cout << "Found the intersection result: " << intervalsToM << " for " << dummy << "." << endl;
        sendIntervals();
        cout << "Server B finished sending the response to Main Server." << endl;

        //*******************extra points***********************
        socklen_t addr_len = sizeof serverMaddr;
        // memset(buf, '\0', sizeof(buf));
        int numbytes;
        // Modified from Beej's code
	    if ((numbytes = recvfrom(socketToM, buf, MAXBUFLEN-1 , 0, (struct sockaddr *) &serverMaddr, &addr_len)) == -1) {
		    cout << "B fails to receive the scheduled interval from M." << endl;
		    exit(1);
	    }
        buf[numbytes] = '\0';
        // cout << buf << endl;
        cout << "Register a meeting at "<< buf << " and update the availability for the following users:" << endl;
        string meeting = buf;
        string leftStr = meeting.substr(meeting.find('[') + 1, meeting.find(',') - 1);
        // stripB(leftStr);
        int l = toInt(leftStr);
    
        meeting.pop_back();
        string rightStr = meeting.substr(meeting.find(',') + 1);
        // stripB(rightStr);
        int r = toInt(rightStr);
        string orig;
        string trans;
        string name;
        for(int i = 0; i < names.size(); i++){
            name = names[i];
            orig =  itoString(bFile[name]);
            deleteTime(name, l, r);
            trans = itoString(bFile[name]);
            cout << name << ": updated from " << orig << " to " << trans << endl;
        }

        // send B's update message 
        int bbytes;
        string m = "U";
        if ((bbytes = sendto(socketToM, m.c_str(), m.length(), 0, (const struct sockaddr *) &serverMaddr, sizeof(serverMaddr))) == -1) {
		    cout << "B fails to send update message to M." << endl;
		    exit(1);
	    }
        cout << "Notified Main Server that registration has finished." << endl;

        //*******************extra points***********************

    }

    
    
    return 0;
}




