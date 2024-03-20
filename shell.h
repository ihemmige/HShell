#ifndef SHELL_H
#define SHELL_H

#include <algorithm>
#include <csignal>
#include <deque>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <numeric>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

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
  void addJob(int jobNum, vector<string> &command);
  int handleRedirection(vector<string> &command);
  void addToHistory(string newCommand);
  void printHistory();
  void restoreHistory();
  void tempHistory(int historyIndex, string command);

  /*
   * Functions for handling background job numbers
   */
  static int createJobNum();
  static void returnJobNum(int num);

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