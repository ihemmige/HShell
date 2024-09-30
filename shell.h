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

class Shell {
private:
  std::deque<std::string> commandHistory;
  std::unordered_map<int, std::string> modifiedHistory;
  static void interruptSignal(int signum);
  static void childSignal(int signum);
  /*
   * Functions for interacting with user, processing user input
   */
  static void outputPrompt();
  char getch();
  std::vector<std::string> parseInput(std::string &input);
  void populateArgVector(std::vector<char *> &args, std::vector<std::string> &command);

  /*
   * Functions for triggering execution
   */
  void executeCommand(std::vector<std::string> &command);
  int handleBuiltins(std::vector<std::string> &command);
  void printJobs();
  void changeDirectory(std::vector<std::string> &command);
  std::string regenerateCommand(std::vector<std::string> &command);
  void generateChild(std::vector<std::string> &command, int originalStdin,
                     int originalStdout, bool inBackground);

  /*
   * Functions for advanced functionality
   */
  void addJob(pid_t jobNum, std::vector<std::string> &command);
  int handleRedirection(std::vector<std::string> &command);
  void addToHistory(std::string newCommand);
  void printHistory();
  void restoreHistory();
  void tempHistory(int historyIndex, std::string command);

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
void printVector(std::vector<std::string> &vec);
void printString(std::string s);
void printDeque(std::deque<std::string> &d);

#endif