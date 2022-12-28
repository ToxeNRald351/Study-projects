#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <string.h>
#include <memory>
#include <bits/stdc++.h>
#include <ifaddrs.h>
#include <algorithm>
#include <fstream>
#include <typeinfo>
#include <sys/wait.h>

#define BUF_SIZE 4096
#define MEMBLOCKSIZE 16384
#define CMDLEN 5

using namespace std;

//error codes
//1-10
#define BADPATH 1
#define BADOPT 2
#define BADPORT 3
#define NOIP 4
#define BADREAD 5
//11-20
#define SOCKET_ERR 11
#define FAILEDOPT 12
#define BINDERR 13
#define LISTENERR 14
#define SENDERR 15

//Replies
#define SUCCESS string("+")
#define ERROR string("-")
#define LOGGEDIN string("!")

//https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
//a function to get arguments from cl
vector<string> get_args(int argc, char** argv);

void check_args(vector<string>& args);

//https://www.cyberithub.com/list-network-interfaces/
//a function to get list of available interfaces
vector<vector<string>> get_if_list();

//https://raymii.org/s/articles/Execute_a_command_and_get_both_output_and_exit_code.html
//a function to get cl execution result. Modified by me. Returns exitcode, result is written into str
int exec(const char* cmd, string& str);

int make_socket_IPv6();

void setopts(int listen_sock);

void make_bind(string IPstr, string port, int listen_sock);

void make_listen(int listen_sock);

int manage_call(int& listen_sock);

void send_msg(int& sock, string msg);

void server_restart(int& listen_sock, int& client_sock, vector<string> server_args);

void greet(int sock);

void ls(int& sock, string mode, string path, bool& list);

void cdir(int& sock, string path, string& workdir, bool logged, bool& dirc);

bool find(int& sock, string file);

void mv(int& sock, string oldname, string newname);

bool retr(int& client_sock, int& listen_sock, string filename, bool& username, bool& password, bool& type, bool& dirc, bool& list, bool& overwrite, char* buf, vector<string>& server_args);

int main(int argc, char** argv) {
    vector<string> server_args;
    string msg, cmd, np_pair, tmp, filepath, stor_param;
    int listen_sock, client_sock;
    char buf[BUF_SIZE];
    int bytesRecv;
    ifstream db;
    bool username = false;
    bool password = false;
    //type false = B; type true = A
    bool type = false;
    //list false = F; list true = V
    bool list = false;
    bool dirc = false;
    bool overwrite = true;

    streampos size;
    char * memblock;
    ofstream infile;

    //Read arguments from cl
    //0-3 - flags, 4-7 - parameters
    server_args = get_args(argc, argv);
    /*
    for (int i = 0; i < 8; i++)
        cout << server_args[i] << endl;
    */
    //check if ok
    check_args(server_args);
    //ip stored in server_args[4]
    //port in server_args[5]

    //cout << server_args[4] << endl;

    //Create socket
    //IPv6
    listen_sock = make_socket_IPv6();

    //Set options
    setopts(listen_sock);

    //Bind the socket to a IP/port
    //IPv6
    make_bind(server_args[4], server_args[5], listen_sock);

    //Mark the socket for listening in
    make_listen(listen_sock);

    //Process a call
    client_sock = manage_call(listen_sock);

    //Make and send reply - connection established
    greet(client_sock);

    //wait for income
    while (1) {
        //Clear the buffer
        memset(buf, 0, BUF_SIZE);
        msg.erase(0, string::npos);
        tmp.erase(0, string::npos);
        //Wait for msg
        bytesRecv = recv(client_sock, buf, BUF_SIZE, 0);
        if (bytesRecv == -1) {
            perror("recv() failed: there was a connection issue!\n");
            break;
        }
        if (bytesRecv == 0) {
            cout << "The client disconnected!\n";
            username = false;
            list = false;
            password = false;
            dirc = false;
            type = false;
            overwrite = false;
            //Make new listener aka restart server
            server_restart(listen_sock, client_sock, server_args);
            //send greet msg
            greet(client_sock);
            continue;
        }
        //msg received
        //Reply
        msg = string(buf);
        cmd = msg.substr(0, 4);
        if (cmd == "USER" or cmd == "ACCT") {
            if (username and password) {
                msg.erase(0, string::npos);
                msg = ERROR + "Already logged in";
                send_msg(client_sock, msg);
            }
            else if (!password) {
                //Open given file
                db.open(server_args[6].c_str());
                if (!db.is_open()) {
                    cerr << "Unable to open " << server_args[6] << endl;
                    if (client_sock != -1)
                        close(client_sock);
                    exit(BADREAD);
                }
                //Search for username
                while (getline(db, np_pair)) {
                    if (msg.substr(CMDLEN, msg.length() - CMDLEN) == np_pair.substr(0, np_pair.find(":"))) {
                        break;
                    }
                    np_pair = "";
                }
                //If no username found - error
                if (np_pair == "") {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Invalid username, try again";
                    send_msg(client_sock, msg);
                }
                else {
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Username valid, send password";
                    send_msg(client_sock, msg);
                    username = true;
                }
            }
            else {
                //Check if received username is correct
                if (msg.substr(CMDLEN, msg.length() - CMDLEN) == np_pair.substr(0, np_pair.find(":"))) {
                    msg.erase(0, string::npos);
                    if (dirc) msg = LOGGEDIN + "Changed working dir to " + server_args[7];
                    else msg = LOGGEDIN + " Logged in";
                    send_msg(client_sock, msg);
                    username = true;
                }
                else {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Invalid username, try again";
                    send_msg(client_sock, msg);
                }
            }
        }
        else if (cmd == "PASS") {
            if (username and password) {
                msg.erase(0, string::npos);
                msg = ERROR + "Already logged in";
                send_msg(client_sock, msg);
            }
            else if (username) {
                //Check if received password is correct
                if (msg.substr(CMDLEN, msg.length() - CMDLEN) == np_pair.substr(np_pair.find(":") + 1, np_pair.length() - np_pair.find(":"))) {
                    msg.erase(0, string::npos);
                    if (dirc) msg = LOGGEDIN + "Changed working dir to " + server_args[7];
                    else msg = LOGGEDIN + " Logged in";
                    send_msg(client_sock, msg);
                    password = true;
                }
                else {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Wrong password, try again";
                    send_msg(client_sock, msg);
                }
            }
            else {
                //Open given file
                db.open(server_args[6].c_str());
                if (!db.is_open()) {
                    cerr << "Unable to open " << server_args[6] << endl;
                    if (client_sock != -1)
                        close(client_sock);
                    exit(BADREAD);
                }
                //Search for password
                while (getline(db, np_pair)) {
                    if (msg.substr(CMDLEN, msg.length() - CMDLEN) == np_pair.substr(np_pair.find(":") + 1, np_pair.length() - np_pair.find(":"))) {
                        break;
                    }
                    np_pair = "";
                }
                //If no password found - error
                if (np_pair == "") {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Wrong password, try again";
                    send_msg(client_sock, msg);
                }
                else {
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Send username";
                    send_msg(client_sock, msg);
                    password = true;
                }
            }
        }
        else if (cmd == "TYPE") {
            if (username and password) {
                if (msg.substr(CMDLEN, msg.length() - CMDLEN) == "A") {
                    type = true;
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Using Ascii mode";
                    send_msg(client_sock, msg);
                }
                else if (msg.substr(CMDLEN, msg.length() - CMDLEN) == "B") {
                    type = false;
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Using Binary mode";
                    send_msg(client_sock, msg);
                }/*/
                else if (msg.substr(CMDLEN, msg.length() - CMDLEN) == "C") {
                    type = false;
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Using Continuous mode";
                    send_msg(client_sock, msg);
                }//*/
                else {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Type not valid";
                    send_msg(client_sock, msg);
                }
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else if (cmd == "LIST") {
            if (username and password) {
                if (msg.length() < 6) {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Too few arguments";
                    send_msg(client_sock, msg);
                    continue;
                }
                //Checks if we deal with 1 or 2 arguments
                //This is for one, cos there can be only one space between words
                tmp = msg.substr(CMDLEN, msg.length() - CMDLEN);
                if (msg.find(" ") == msg.rfind(" ")) {
                    if (tmp == "F" or tmp == "f") {
                        ls(client_sock, "", server_args[7], list);
                    }
                    else if (tmp == "V" or tmp == "v") {
                        ls(client_sock, "-l ", server_args[7], list);
                    }
                    //Consider given argument as path
                    //Command will be performed on last LIST command basis with its parameter
                    else {
                        if (list)
                            ls(client_sock, "-l ", msg.substr(CMDLEN, msg.length() - CMDLEN), list);
                        else
                            ls(client_sock, "", msg.substr(CMDLEN, msg.length() - CMDLEN), list);
                    }
                }
                else {
                    if (msg.substr(CMDLEN, 1) == "F" or msg.substr(CMDLEN, 1) == "f") {
                        ls(client_sock, "", msg.substr(msg.rfind(" ") + 1, msg.length() - msg.rfind(" ") - 1), list);
                    }
                    else if (msg.substr(CMDLEN, 1) == "V" or msg.substr(CMDLEN, 1) == "v") {
                        ls(client_sock, "-l ", msg.substr(msg.rfind(" ") + 1, msg.length() - msg.rfind(" ") - 1), list);
                    }
                    else {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Invalid LIST parameter specified";
                        send_msg(client_sock, msg);
                    }
                }
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else if (cmd == "CDIR") {
            cdir(client_sock, msg.substr(CMDLEN, msg.length() - CMDLEN), server_args[7], username and password, dirc);
        }
        else if (cmd == "KILL") {
            if (username and password) {
                filepath = server_args[7] + "/" + msg.substr(CMDLEN, msg.length() - CMDLEN);
                if (exec(("rm " + filepath).c_str(), tmp)) {
                    msg.erase(0, string::npos);
                    msg = ERROR + "Not deleted because: No such file or catalog";
                    send_msg(client_sock, msg);
                }
                else {
                    msg.erase(0, string::npos);
                    msg = SUCCESS + filepath + " deleted";
                    send_msg(client_sock, msg);
                }
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else if (cmd == "NAME") {
            if (username and password) {
                filepath = server_args[7] + "/" + msg.substr(CMDLEN, msg.length() - CMDLEN);
                if (find(client_sock, filepath)) {
                    //Clear the buffer
                    memset(buf, 0, BUF_SIZE);
                    msg.erase(0, string::npos);
                    //Wait for msg
                    bytesRecv = recv(client_sock, buf, BUF_SIZE, 0);
                    if (bytesRecv == -1) {
                        perror("recv() failed: there was a connection issue!\n");
                        break;
                    }
                    if (bytesRecv == 0) {
                        cout << "The client disconnected!\n";
                        username = false;
                        list = false;
                        password = false;
                        dirc = false;
                        type = false;
                        overwrite = false;
                        //Make new listener aka restart server
                        server_restart(listen_sock, client_sock, server_args);
                        //send greet msg
                        greet(client_sock);
                        continue;
                    }
                    //msg received
                    //Reply
                    msg = string(buf);
                    cmd = msg.substr(0, 4);
                    if (cmd == "TOBE") {
                        mv(client_sock, filepath, server_args[7] + "/" + msg.substr(CMDLEN, msg.length() - CMDLEN));
                    }
                    else {
                        msg.erase(0, string::npos);
                        msg = ERROR + "File wasn't renamed because: new name unspecified";
                        send_msg(client_sock, msg);
                    }
                }
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else if (cmd == "DONE") {
            username = false;
            password = false;
            dirc = false;
            list = false;
            type = false;
            overwrite = false;
            //Send SUCCESS msg
            msg.erase(0, string::npos);
            msg = SUCCESS;
            send_msg(client_sock, msg);
            cout << "The client disconnected!\n";
            //Make new listener aka restart server
            server_restart(listen_sock, client_sock, server_args);
            //send greet msg
            greet(client_sock);
        }
        else if (cmd == "RETR") {
            if (username and password) {
                filepath = server_args[7] + "/" + msg.substr(CMDLEN, msg.length() - CMDLEN);
                retr(client_sock, listen_sock, filepath, username, password, type, dirc, list, overwrite, buf, server_args);
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else if (cmd == "STOR") {
            if (msg.length() < 10) {
                msg.erase(0, string::npos);
                msg = ERROR + "Too short";
                send_msg(client_sock, msg);
                continue;
            }
            if (username and password) {
                filepath = server_args[7] + "/" + msg.substr(msg.rfind(" ") + 1, msg.length() - msg.rfind(" ") - 1);
                stor_param = msg.substr(CMDLEN, 3);
                transform(stor_param.begin(), stor_param.end(), stor_param.begin(), ::toupper);
                //msg.substr(CMDLEN, 3) -- parameter
                //msg.substr(msg.rfind(" "), msg.length() - msg.rfind(" ")) -- filename
                if (exec(("find " + filepath).c_str(), tmp)) {
                    //file NOT found
                    if (stor_param == "NEW") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "File does not exist, will create new file";
                        send_msg(client_sock, msg);
                    }
                    else if (stor_param == "OLD") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "Will create new file";
                        send_msg(client_sock, msg);
                    }
                    else if (stor_param == "APP") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "Will create file";
                        send_msg(client_sock, msg);
                    }
                    else {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Invalid parameter";
                        send_msg(client_sock, msg);
                    }
                }
                else {
                    //file found
                    if (stor_param == "NEW") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "File exists, will create new generation of file";
                        send_msg(client_sock, msg);
                        filepath = filepath + "_new";
                    }
                    else if (stor_param == "OLD") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "Will write over old file";
                        send_msg(client_sock, msg);
                    }
                    else if (stor_param == "APP") {
                        msg.erase(0, string::npos);
                        msg = SUCCESS + "Will append to file";
                        send_msg(client_sock, msg);
                        overwrite = false;
                    }
                    else {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Invalid parameter";
                        send_msg(client_sock, msg);
                    }
                }
                //#############################################################################################
                //Clear the buffer
                memset(buf, 0, BUF_SIZE);
                msg.erase(0, string::npos);
                //Wait for msg
                bytesRecv = recv(client_sock, buf, BUF_SIZE, 0);
                if (bytesRecv == -1) {
                    perror("recv() failed: there was a connection issue!\n");
                    break;
                }
                if (bytesRecv == 0) {
                    cout << "The client disconnected!\n";
                    username = false;
                    password = false;
                    list = false;
                    dirc = false;
                    type = false;
                    overwrite = false;
                    //Make new listener aka restart server
                    server_restart(listen_sock, client_sock, server_args);
                    //send greet msg
                    greet(client_sock);
                    continue;
                }
                //msg received
                //Reply
                msg = string(buf);
                cmd = msg.substr(0, 4);
                cout << msg << "|\t|" << cmd << endl;
                //#############################################################################################
                if (cmd == "SIZE") {
                    memblock = new char[size];
                    if (!memblock) {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Not enough room, don't send it";
                        send_msg(client_sock, msg);
                        continue;
                    }
                    //Create a file
                    if (exec(("touch " + filepath).c_str(), tmp)) {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Not enough room, don't send it";
                        send_msg(client_sock, msg);
                        continue;
                    }
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "ok, waiting for file";
                    send_msg(client_sock, msg);
                    //#############################################################################################
                    //Wait for FILE DATA
                    bytesRecv = recv(client_sock, memblock, size, 0);
                    if (bytesRecv == -1) {
                        delete[] memblock;
                        perror("recv() failed: there was a connection issue!\n");
                        break;
                    }
                    if (bytesRecv == 0) {
                        delete[] memblock;
                        cout << "The client disconnected!\n";
                        username = false;
                        list = false;
                        password = false;
                        dirc = false;
                        type = false;
                        overwrite = false;
                        //Make new listener aka restart server
                        server_restart(listen_sock, client_sock, server_args);
                        //send greet msg
                        greet(client_sock);
                        continue;
                    }
                    //FILE DATA received
                    if (type) {
                        if (overwrite) { infile.open(filepath, ios::out); }
                        else { infile.open(filepath, ios::out|ios::app); }
                    }
                    else {
                        if (overwrite) { infile.open(filepath, ios::out|ios::binary); }
                        else { infile.open(filepath, ios::out|ios::binary|ios::app); }
                    }
                    if (!infile.is_open()) {
                        msg.erase(0, string::npos);
                        msg = ERROR + "Couldn't save because: can't open file";
                        send_msg(client_sock, msg);
                        continue;
                    }
                    infile << memblock;
                    infile.close();
                    
                    msg.erase(0, string::npos);
                    msg = SUCCESS + "Saved" + filepath;
                    send_msg(client_sock, msg);
                }
            }
            else {
                msg.erase(0, string::npos);
                msg = ERROR + "Access denied. You need to log in first";
                send_msg(client_sock, msg);
            }
        }
        else {
            msg.erase(0, string::npos);
            msg = ERROR + "CMD";
            send_msg(client_sock, msg);
        }
    }

    //Close socket
    if (client_sock != -1)
        close(client_sock);

    return 0;
}

vector<string> get_args(int argc, char** argv) {
    int iflag = 0, pflag = 0, uflag = 0, fflag = 0;
    string def_ip = "::", def_port = "115";
    string ivalue = def_ip, pvalue = def_port, uvalue = "", fvalue = "";
    int index;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, ":i:p:u:f:")) != -1)
        switch (c)
        {
        case 'i':
            if (!iflag and optarg) {
                iflag = 1;
                ivalue = optarg;
            }
            break;
        case 'p':
            if (!pflag and optarg) {
                pflag = 1;
                pvalue = optarg;
            }
            break;
        case 'u':
            if (!uflag and optarg) {
                uflag = 1;
                uvalue = optarg;
            }
            break;
        case 'f':
            if (!fflag and optarg) {
                fflag = 1;
                fvalue = optarg;
            }
            break;
        case ':':
            if (optopt == 'i' || optopt == 'p')
                break;
            cerr << "Option -" << optopt << " requires an argument." << endl;
            exit(BADOPT);
        case '?':
            if (isprint (optopt))
                cerr << "Unknown option '-" << optopt <<"'." << endl;
            else
                cerr << "Unknown option character '\\x" << optopt << "'." << endl;
            exit(BADOPT);
        default:
            abort ();
        }


    /*cout <<"\niflag = " << iflag << ", pflag = " << pflag 
        << ", uflag = " << uflag << ", fflag = " << fflag << endl
        << "ivalue = " << ivalue << ", pvalue = " << pvalue 
        << ", uvalue = " << uvalue << ", fvalue = " << fvalue << endl;//*/

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    vector<string> opt_array;
    opt_array.push_back(to_string(iflag));
    opt_array.push_back(to_string(pflag));
    opt_array.push_back(to_string(uflag));
    opt_array.push_back(to_string(fflag));
    opt_array.push_back(ivalue);
    opt_array.push_back(pvalue);
    opt_array.push_back(uvalue);
    opt_array.push_back(fvalue);

    return opt_array;
}

void check_args(vector<string>& args) {
    //0-3 flags, 4-7 values

    if (stoi(args[0]) == 1) {
        vector<vector<string>> list = get_if_list();
        string np_pair = "";
        //get ipv6 of certain if from the list
        for (unsigned int i = 0; i < list.size(); i++) {
            if (args[4] == list[i][0])
                np_pair = list[i][2];
        }
        //If if has no ipv6 -> error
        if (np_pair == "") {
            cout << "No IPv6 for specified interface!" << endl;
            exit(NOIP);
        }
        args[4] = np_pair;
    }

    if (stoi(args[1]) == 1)
        if (stoi(args[5]) > 65535) {
            cerr << "Port out of range!" << endl;
            exit(BADPORT);
        }

    if (args[6] == "") {
        cerr << "Path to user data is unset!" << endl;
        exit(BADPATH);
    }

    if (args[7] == "") {
        cerr << "Path to work directory is unset!" << endl;
        exit(BADPATH);
    }
}

vector<vector<string>> get_if_list() {

    struct ifaddrs *addresses;
    if (getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        exit(12);
    }

    vector<vector<string>> list;
    vector<string> row;
    struct ifaddrs *address = addresses;
    int family;
    while(address) { 
        family = address->ifa_addr->sa_family;
        //cout << address->ifa_name << "\t" << family << endl;
        if (family == AF_INET6) {
            row.push_back(string(address->ifa_name));
            row.push_back("IPv6");
            char ap[100];
            const int family_size = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            string np_pair = string(ap);
            np_pair = np_pair.substr(0, np_pair.find("%"));
            row.push_back(np_pair);
            list.push_back(row);
            row = {};
        }
        address = address->ifa_next;
    }
    freeifaddrs(addresses);

    return list;
}

int exec(const char* cmd, string& str) {
    char buffer[128];
    int result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) 
            str += buffer;
    }
    catch (...) {
        pclose(pipe);
        throw;
    }
    result = pclose(pipe);
    return result;
}

int make_socket_IPv6() {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0){
        cerr << "Can't create socket!\n";
        exit(SOCKET_ERR);
    }

    return sock;
}

void setopts(int listen_sock) {
    //Set an option to reuse address
    int on = 1, off = 0;
    int result = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
    if (result < 0) {
        cerr << "REUSEADDR set to 1 command failed" << endl;
        exit (FAILEDOPT);
    }
    //Set an option to accept both IPv4 and IPv6
    result = setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*)&off, sizeof(off));
    if (result < 0) {
        cerr << "IPV6_V6ONLY set to 1 command failed" << endl;
        exit (FAILEDOPT);
    }
}

void make_bind(string IPstr, string port, int listen_sock) {
    sockaddr_in6 hint;
    memset(&hint, 0, sizeof(hint));
    hint.sin6_family = AF_INET6;
    hint.sin6_port = htons(stoi(port));
    
    inet_pton(AF_INET6, IPstr.c_str(), &hint.sin6_addr);

    if (bind(listen_sock, (sockaddr*)&hint, sizeof(hint)) < 0) {
        cerr << "Can't bind to IP/port!" << endl;
        exit(BINDERR);
    }
}

void make_listen(int listen_sock) {
    if (listen(listen_sock, SOMAXCONN) == -1) {
        cerr << "Can't listen!\n";
        exit(LISTENERR);
    }
}

int manage_call(int& listen_sock) {
    //Accept a call
    sockaddr_in6 client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listen_sock, (sockaddr*)&client, &clientSize);
    if (clientSocket == -1) {
        cerr << "Problem with socket!\n";
        exit(SOCKET_ERR);
    }

    //Close the listening socket
    close(listen_sock);
    //Output result
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, 
                sizeof(client), 
                host, 
                NI_MAXHOST, 
                svc, 
                NI_MAXSERV, 
                0);
    
    if (result) {
        cout << host << " connected on " << svc << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin6_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin6_port) << endl;
    }

    return clientSocket;
}

void send_msg(int& sock, string msg) {
    int rc = send(sock, msg.c_str(), BUF_SIZE, 0);
    if (rc < 0) {
        perror("send() failed: unable to reply");
        if (sock != -1)
            close(sock);
        exit(SENDERR);
    }
}

void server_restart(int& listen_sock, int& client_sock, vector<string> server_args) {
    //Make new listener aka restart server
    close(client_sock);
    listen_sock = make_socket_IPv6();
    setopts(listen_sock);
    make_bind(server_args[4], server_args[5], listen_sock);
    make_listen(listen_sock);
    client_sock = manage_call(listen_sock);
}

void greet(int sock) {
    char buf[BUF_SIZE];
    string msg;
    gethostname(buf, BUF_SIZE);
    buf[BUF_SIZE - 1] = '\0';
    msg = SUCCESS + string(buf) + " SFTP Service";
    send_msg(sock, msg);
}

void ls(int& sock, string mode, string path, bool& list) {
    string result, msg;
    if (exec(("ls " + mode + path).c_str(), result)) {
        msg.erase(0, string::npos);
        msg = ERROR + "Unable to gain access to '" + path + "': No such file or catalog";
        send_msg(sock, msg);
    }
    else {
        msg.erase(0, string::npos);
        msg = SUCCESS + path + "\n" + result;
        send_msg(sock, msg);
        if (mode == "") list = false;
        else list = true;
    }
}

void cdir(int& sock, string path, string& workdir, bool logged, bool& dirc) {
    string result, msg;
    if (exec(("cd " + path).c_str(), result)) {
        msg.erase(0, string::npos);
        msg = ERROR + "Can't connect to directory because: can't cd to '" + path + "': No such catalog";
        send_msg(sock, msg);
    }
    else {
        workdir = path;
        if (logged) {
            msg.erase(0, string::npos);
            msg = LOGGEDIN + "Changed working dir to " + path;
            send_msg(sock, msg);
        }
        else {
            dirc = true;
            msg.erase(0, string::npos);
            msg = SUCCESS + "directory ok, send account/password";
            send_msg(sock, msg);
        }
    }
}

bool find(int& sock, string file) {
    string result, msg;
    if (exec(("find " + file).c_str(), result)) {
        msg.erase(0, string::npos);
        msg = ERROR + "Can't find " + file;
        send_msg(sock, msg);
        return false;
    }
    msg.erase(0, string::npos);
    msg = SUCCESS + "File exists";
    send_msg(sock, msg);
    return true;
}

void mv(int& sock, string oldname, string newname) {
    string result, msg;
    if (exec(("mv " + oldname + " " + newname).c_str(), result)) {
        msg.erase(0, string::npos);
        msg = ERROR + "File wasn't renamed because: something went wrong";
        send_msg(sock, msg);
    }
    else {
        msg.erase(0, string::npos);
        msg = SUCCESS + oldname + " renamed to " + newname;
        send_msg(sock, msg);
    }
}

bool retr(int& client_sock, int& listen_sock, string filename, bool& username, bool& password, bool& type, bool& dirc, bool& list, bool& overwrite, char* buf, vector<string>& server_args) {
    string result, msg, cmd;
    streampos size;
    char * memblock;
    int bytesRecv;
    if (exec(("find " + filename).c_str(), result)) {
        msg.erase(0, string::npos);
        msg = ERROR + "File doesn't exist";
        send_msg(client_sock, msg);
        return false;
    }

    ifstream file; 
    if (!type) file.open(filename, ios::in|ios::binary|ios::ate);
    else file.open(filename, ios::in|ios::ate);

    if (file.is_open()) {
        size = file.tellg();
        msg.erase(0, string::npos);
        msg = to_string(size);
        send_msg(client_sock, msg);

        //Clear the buffer
        memset(buf, 0, BUF_SIZE);
        msg.erase(0, string::npos);
        //Wait for msg
        bytesRecv = recv(client_sock, buf, BUF_SIZE, 0);
        if (bytesRecv == -1) {
            perror("recv() failed: there was a connection issue!\n");
            return false;
        }
        if (bytesRecv == 0) {
            cout << "The client disconnected!\n";
            username = false;
            password = false;
            dirc = false;
            type = false;
            list = false;
            overwrite = false;
            //Make new listener aka restart server
            server_restart(listen_sock, client_sock, server_args);
            //send greet msg
            greet(client_sock);
            return false;
        }
        //msg received
        //Reply
        msg = string(buf);
        cmd = msg.substr(0, 4);
        if (cmd == "SEND") {
            memblock = new char[size];
            file.seekg(0, ios::beg);
            file.read(memblock, size);
            file.close();
            int rc = send(client_sock, memblock, size, 0);
            if (rc < 0) {
                perror("send() failed: unable to reply");
                if (client_sock != -1)
                    close(client_sock);
                exit(SENDERR);
            }
            delete[] memblock;
            return true;
        }
        else if (cmd == "STOP") {
            file.close();
            msg.erase(0, string::npos);
            msg = SUCCESS + "ok, RETR aborted";
            send_msg(client_sock, msg);
            return true;
        }
        else {
            file.close();
            msg.erase(0, string::npos);
            msg = ERROR + "Bad reply, RETR aborted";
            send_msg(client_sock, msg);
            return false;
        }
    }
    else {
        msg.erase(0, string::npos);
        msg = ERROR + "Unable to open file";
        send_msg(client_sock, msg);
        return false;
    }
}
