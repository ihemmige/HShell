#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <filesystem>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

void signalHandler(int signum);

void outputPrompt();

vector<string> parseInput(string input);

void printVector(vector<string>& vec);

void changeDirectory(vector<string>& command);

int handleRedirection(vector<string>& command);

void generateChild(vector<string>& command, int originalStdin, int originalStdout);

void executeCommand(vector<string>& command);

void populateArgVector(vector<char*>& args, vector<string>& command);

int handleBuiltins(vector<string>& command);

#endif
