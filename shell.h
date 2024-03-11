#ifndef SHELL_H
#define SHELL_H

#include <csignal>
#include <deque>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <numeric>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace std;

class Shell {
private:
  deque<string> commandHistory;
  // unordered_map<int, string> jobMap;
  static void signalHandler(int signum);
  static void childSignal(int signum);
  /*
   * Functions for interacting with user, processing user input
   */
  static void outputPrompt();
  char getch();
  vector<string> parseInput(string &input);
  void populateArgVector(vector<char *> &args, vector<string> &command);

  /*
   * Functions for triggering execution
   */
  void executeCommand(vector<string> &command);
  int handleBuiltins(vector<string> &command);
  void printJobs();
  void changeDirectory(vector<string> &command);
  void generateChild(vector<string> &command, int originalStdin,
                     int originalStdout, bool inBackground);

  /*
   * Functions for advanced functionality
   */
  void addToHistory(string newCommand);
  int handleRedirection(vector<string> &command);

public:
  Shell();
  void shellLoop();
};

/*
 * Functions for testing and argument visibility
 */
void printVector(vector<string> &vec);
void printString(string s);
void printDeque(deque<string> &d);

#endif