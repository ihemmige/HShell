#ifndef SHELL_H
#define SHELL_H

#include <filesystem>
#include <iostream>
#include <deque>
#include <unordered_map>
#include <set>
#include <vector>
#include <csignal>
#include <fcntl.h>
#include <numeric>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <mutex>
#include <algorithm>

using namespace std;

class Shell {
private:
  deque<string> commandHistory;
  unordered_map<int, string> modifiedHistory;
  static void interruptSignal(int signum);
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
  string regenerateCommand(vector<string> &command);
  void generateChild(vector<string> &command, int originalStdin,
                     int originalStdout, bool inBackground);

  /*
   * Functions for advanced functionality
   */
  void addJob(int jobNum, vector<string> & command);
  int handleRedirection(vector<string> &command);
  void addToHistory(string newCommand);
  void printHistory();
  void restoreHistory();
  void tempHistory(int historyIndex, string command);

public:
  Shell();
  void shellLoop();
};

// functions for handling background job numbers
int createJobNum();
void returnJobNum(int num);

/*
 * Functions for testing and argument visibility
 */
void printVector(vector<string> &vec);
void printString(string s);
void printDeque(deque<string> &d);

#endif