#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>

using namespace std;

/**
 * check & handle dup2
 * 0 : strin
 * 1 : strout
 * 2 : strerr -> default
 **/
int statusFile = 2;
string fileName = "";

bool should_run = true;

int pipefd[2];
string statusPipe = "33";

vector<string> history;

void addHistory(string cmd) {
    if (history.size() >= 1000) {
        history.erase(history.begin());
    }
    history.push_back(cmd);
}

void printHistory(bool last) {
    if (history.empty()) {
        puts("No commands in history");
        return;
    }
    if (last) {
        cout << history.back();
        return;
    }
    for (int i = 0; i < history.size(); i++) {
        cout << i << " " << history[i];
    }
}

string getCmd() {
    string cmd;
    getline(cin, cmd);
    return cmd + '\n';
}

vector<string> parseString(string inputString, string delimiter) {
    vector<string> result;
    char *token = strtok(const_cast<char*>(inputString.c_str()), const_cast<char*>(delimiter.c_str()));
    while (token) {
        result.push_back(token);
        token = strtok(nullptr, delimiter.c_str());
    }
    return result;
}

bool checkWait(string cmd) {
    return strtok(const_cast<char*>(cmd.c_str()), " |\t\r\n\a");
}

void start(string hostname) {
    while (should_run) {
        cout << hostname << "> ";
        // split &
        vector<string> cmdLink = parseString(getCmd(), "&");
        bool waitPid = checkWait(cmdLink.back());
        for (auto iPipe : cmdLink) {
            pipe(pipefd);
            // split pipe
            vector<string> cmdPipe = parseString(iPipe, "|");
            for (int i = 0; i < cmdPipe.size(); i++) {
                // handle
                if (cmdPipe.size() == 1) {
                    statusPipe = "22";
                } else {
                    if (i == 0) {
                        statusPipe = "01";
                    } else if (i == cmdPipe.size() - 1) {
                        statusPipe = "10";
                    } else {
                        statusPipe = "12";
                    }
                }
                // add cmd to history
                if (cmdPipe[i] != "\n") {
                    addHistory(cmdPipe[i]);
                }
            }
            
        }
    }
}
int main() {
    start("osh");
    return 0;
}