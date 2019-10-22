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

int checkdup2(int &status, string &fileName, vector<char*> cmd) {
    int pos = -1;
    for (int i = 0; i < cmd.size(); i++) {
        if (cmd[i] == nullptr) {
            break;
        }
        if (status == 2) {
            if (!strcmp(cmd[i], "<")) {
                status = 0;
                pos = i;
            } else if (!strcmp(cmd[i], ">")) {
                status = 1;
                pos = i;
            }
        }
        fileName = cmd[i];
    }
    return pos;
}

void openFile(int status, string fileName) {
    int file;
    if (status != 2) {
        if (status == 0) {
            file = open(fileName.c_str(), O_RDONLY);
        } else if (status == 1) {
            file = open(fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }
        dup2(file, status);
    }
}

void customCmd(vector<string> &cmd) {
    if (cmd[0] == "ll") {
        cmd[0] = "ls";
        cmd.insert(cmd.begin() + 1, "-l");
    }
    if (cmd[0] == "la") {
        cmd[0] = "ls";
        cmd.insert(cmd.begin() + 1, "-a");
    }
}

bool inCustomColor(string cmd) {
    return (cmd == "ls" || cmd == "grep");
}

void exec(bool waitPid, vector<string> cmd) {
    if (!cmd.empty() && cmd[0] == "exit") {
        should_run = false;
    }
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        if (cmd.front() == "!!") {
            history.pop_back();
            printHistory(true);
            if (history.empty()) {
                exit(0);
            }
            cmd = parseString(history.back(), " |\t\r\n\a");
        }
        // add color & custom ll, la, ...
        customCmd(cmd);
        if (inCustomColor(cmd[0])) {
            cmd.insert(cmd.begin() + 1, "--color=auto");
        }
        
        // convert vector string -> vector char*
        vector<char*> charVec;
        for (int i = 0; i < cmd.size(); i++) {
            charVec.push_back(const_cast<char*>(cmd[i].c_str()));
        }
        charVec.push_back(nullptr);

        int posNull = checkdup2(statusFile, fileName, charVec);
        if (posNull > 0) {
            charVec[posNull] = nullptr;
        }
        openFile(statusFile, fileName);
        
        if (statusPipe == "22") {
            // do nothing
        }
        if (statusPipe == "01") {
            close(pipefd[0]);
            dup2(pipefd[1], 1);
        }
        if (statusPipe == "10") {
            dup2(pipefd[0], 0);
            close(pipefd[1]);
        }
        if (statusPipe == "12") {
            dup2(pipefd[0], 0);
        }

        char **args = &charVec[0];
        if (!strcmp(*args, "history")) {
            history.pop_back();
            printHistory(false);
            exit(0);
        } else if (!strcmp(*args, "exit")) {
            exit(0);
        } else if (execvp(*args, args)) {
            printf("Command '%s' not found\n", *args);
            exit(0);
        } 
        
    } else if (pid > 0) {
        // parent process
        if (statusPipe != "22") {
            if (statusPipe == "10") {
                waitpid(pid, nullptr, 1);
            }
        } else {
            waitpid(pid, nullptr, !waitPid);
        }
    } else {
        // error
        puts("The fork system call failed to create a new process");
        exit(1);
    }
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
                exec(waitPid, parseString(cmdPipe[i], " |\t\r\n\a"));
            }
            
        }
    }
}
int main() {
    start("osh");
    return 0;
}