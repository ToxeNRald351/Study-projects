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
#include <netinet/in.h>
#include <algorithm>
#include <fstream>

#define NETDB_MAX_HOST_NAME_LENGTH 128
#define BUF_SIZE 4096

using namespace std;

//error codes
//1-10
#define BADPATH 1
#define BADOPT 2
#define BADPORT 3
#define NOIP 4
//11-20
#define SOCKET_ERR 11
#define FAILEDOPT 12
#define BINDERR 13
#define LISTENERR 14
#define SENDERR 15
#define CONNECTION 16
#define NOHOST 17
#define RECVERR 18

vector<string> get_args(int argc, char** argv);

void check_args(vector<string> args);

void close_cnn(int& sock, addrinfo* res);

void send_msg(int& sock, string msg);

int recv_msg(int server_sock, char *buf, size_t size = BUF_SIZE);

int exec(const char* cmd, string& str);

int main(int argc, char** argv) {
    vector<string> client_args;
    string input, cmd, filepath, result;
    //0-2 - flags, 3-5 - parameters
    int server_sock = -1, rc, bytesRecv = 0;
    char buf[BUF_SIZE];
    struct in6_addr serveraddr;
    struct addrinfo hint, *res=NULL;
    bool type = false;
    //for incoming file data
    streampos size;
    char * memblock;
    ofstream infile;
    ifstream outfile;

    //Get arguments
    client_args = get_args(argc, argv);
    check_args(client_args);

    //Create a "hint"
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_NUMERICSERV;
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    //Check if given adress is of IPv4 or IPv6 type
    rc = inet_pton(AF_INET, client_args[3].c_str(), &serveraddr);
    if (rc == 1) {
        hint.ai_family = AF_INET;
        hint.ai_flags |= AI_NUMERICHOST;
    }
    else {
        rc = inet_pton(AF_INET6, client_args[3].c_str(), &serveraddr);
        if (rc == 1) {
            hint.ai_family = AF_INET6;
            hint.ai_flags |= AI_NUMERICHOST;
        }
    }

    //Search for specified host
    rc = getaddrinfo(client_args[3].c_str(), client_args[4].c_str(), &hint, &res);
    if (rc != 0) {
        cerr << "Host not found --> " << gai_strerror(rc) << endl;
        if (rc == EAI_SYSTEM)
            cerr << "getaddrinfo() failed" << endl;
        close_cnn(server_sock, res);
        exit(NOHOST);
    }

    //Create socket
    server_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_sock < 0) {
        cerr << "socket() failed" << endl;
        close_cnn(server_sock, res);
        exit(SOCKET_ERR);
    }

    //Connect to server
    rc = connect(server_sock, res->ai_addr, res->ai_addrlen);
    if (rc < 0) {
        perror("connect() failed\n");
        close_cnn(server_sock, res);
        exit(CONNECTION);
    }

    //Reinit to 0
    memset(buf, 0, BUF_SIZE);
    input.erase(0, string::npos);
    //Recieve msg
    bytesRecv = recv_msg(server_sock, buf);
    if (bytesRecv) {
        close_cnn(server_sock, res);
        exit(CONNECTION);
    }
    //Msg recieved successfully
    cout << buf << endl;
    while (1) {
        //Get user input
        getline(cin, input);
        //Make cmd uppercase
        cmd = input.substr(0, 4);
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        input.replace(0, 4, cmd);

        if (input == "DONE") {
            send_msg(server_sock, input);
            recv_msg(server_sock, buf);
            cout << buf << endl;
            break;
        }
        else if (cmd == "TYPE") {
            send_msg(server_sock, input);
            recv_msg(server_sock, buf);
            if (buf[0] == '+') {
                if (input.substr(5, 1) == "B") type = false;
                else type = true;
            }
        }
        else if (cmd == "RETR") {
            filepath = client_args[5] + "/" + input.substr(5, input.length() - 5);
            send_msg(server_sock, input);
            //Reinit to 0
            memset(buf, 0, BUF_SIZE);
            input.erase(0, string::npos);
            recv_msg(server_sock, buf);
            cout << buf << endl;

            //Get user input
            getline(cin, input);
            //Make cmd uppercase
            cmd = input.substr(0, 4);
            transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
            input.replace(0, 4, cmd);

            if (input == "SEND") {
                //Create a memory block for incoming data
                size = stoi(string(buf));
                memblock = new char[size];
                if (!memblock) {
                    cerr << "Not enough space!" << endl;
                    continue;
                }
                //Create a file
                if (exec(("touch " + filepath).c_str(), result)) {
                    cerr << "Unable to create file!" << endl;
                    continue;
                }
                send_msg(server_sock, input);
                //Reinit to 0
                memset(buf, 0, BUF_SIZE);
                input.erase(0, string::npos);
                recv_msg(server_sock, memblock, size);
                if (type) { infile.open(filepath, ios::out); }
                else { infile.open(filepath, ios::out|ios::binary); }
                infile << memblock;
                infile.close();
            }
        }
        else if (cmd == "STOR") {
            filepath = client_args[5] + "/" + input.substr(input.rfind(" ") + 1, input.length() - input.rfind(" ") - 1);
            send_msg(server_sock, input);
            //Reinit to 0
            memset(buf, 0, BUF_SIZE);
            input.erase(0, string::npos);
            recv_msg(server_sock, buf);
            cout << buf << endl;
            if (buf[0] == '-') 
                continue;
            if (type) { outfile.open(filepath, ios::in|ios::ate); }
            else { outfile.open(filepath, ios::in|ios::binary|ios::ate); }
            if (outfile.is_open()) {
                size = outfile.tellg();

                input = "SIZE " + size;
                send_msg(server_sock, input);
                //Reinit to 0
                memset(buf, 0, BUF_SIZE);
                input.erase(0, string::npos);
                recv_msg(server_sock, buf);
                cout << buf << endl;

                if (buf[0] == '-') {
                    continue;
                }

                memblock = new char[size];
                outfile.seekg (0, ios::beg);
                outfile.read (memblock, size);
                outfile.close();

                int rc = send(server_sock, memblock, size, 0);
                if (rc < 0) {
                    perror("send() failed: unable to reply");
                    if (server_sock != -1)
                        close(server_sock);
                    exit(SENDERR);
                }
                delete[] memblock;
                //Reinit to 0
                memset(buf, 0, BUF_SIZE);
                input.erase(0, string::npos);
                recv_msg(server_sock, buf);
                cout << buf << endl;
            }
        }
        else {
            send_msg(server_sock, input);
            //Reinit to 0
            memset(buf, 0, BUF_SIZE);
            input.erase(0, string::npos);
            recv_msg(server_sock, buf);
            cout << buf << endl;
        }
    }

    //Close connection
    close_cnn(server_sock, res);

    return 0;
}

void check_args(vector<string> args) {
    //0-2 flags, 3-5 values

    if (stoi(args[1]) == 1)
        if (stoi(args[4]) > 65535) {
            cerr << "Port out of range!" << endl;
            exit(BADPORT);
        }

    if (args[3] == "") {
        cerr << "IP-address is unset!" << endl;
        exit(NOIP);
    }

    if (args[5] == "") {
        cerr << "Path to work directory is unset!" << endl;
        exit(BADPATH);
    }
}

vector<string> get_args(int argc, char** argv) {
    int hflag = 0, pflag = 0, fflag = 0;
    string def_port = "115";
    string hvalue = "", pvalue = def_port, fvalue = "";
    int index;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, ":h:p:f:")) != -1)
        switch (c)
        {
        case 'h':
            if (!hflag and optarg) {
                hflag = 1;
                hvalue = optarg;
            }
            break;
        case 'p':
            if (!pflag and optarg) {
                pflag = 1;
                pvalue = optarg;
            }
            break;
        case 'f':
            if (!fflag and optarg) {
                fflag = 1;
                fvalue = optarg;
            }
            break;
        case ':':
            if (optopt == 'p')
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


    /*cout <<"\nhflag = " << hflag << ", pflag = " << pflag 
        << ", fflag = " << fflag << endl
        << "hvalue = " << hvalue << ", pvalue = " << pvalue 
        << ", fvalue = " << fvalue << endl;//*/

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    vector<string> opt_array;
    opt_array.push_back(to_string(hflag));
    opt_array.push_back(to_string(pflag));
    opt_array.push_back(to_string(fflag));
    opt_array.push_back(hvalue);
    opt_array.push_back(pvalue);
    opt_array.push_back(fvalue);

    return opt_array;
}

void close_cnn(int& sock, addrinfo* res) {
    if (sock != -1)
        close(sock);
    if (res != NULL)
        freeaddrinfo(res);
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

int recv_msg(int server_sock, char *buf, size_t size) {
    int bytesRecv = recv(server_sock, buf, size, 0);
    if (bytesRecv == -1) {
        perror("recv() failed: there was a connection issue!\n");
        return -1;
    }
    if (bytesRecv == 0) {
        cout << "Remote server closed the connection" << endl;
        return -1;
    }
    return 0;
}

int exec(const char* cmd, string& str) {
    char buffer[128];
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
    return pclose(pipe);
}
