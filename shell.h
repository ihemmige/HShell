#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <deque> 
#include <sstream>
#include <filesystem>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

using namespace std;

// Functions for interacting with user, processing user input
void signalHandler(int signum);
void outputPrompt();
char getch();
vector<string> parseInput(string& input);
void populateArgVector(vector<char*>& args, vector<string>& command);

// Functions for triggering execution
void shellLoop();
void executeCommand(vector<string>& command);
int handleBuiltins(vector<string>& command);
void changeDirectory(vector<string>& command);
void generateChild(vector<string>& command, int originalStdin, int originalStdout);

// Functions for advanced Functionality
void addToHistory(deque<string>& commandHistory, string newCommand);
int handleRedirection(vector<string>& command);

// Functions for testing and argument visibility
void printVector(vector<string>& vec);
void printString(string s);
void printDeque(deque<string>& d);

#endif
