#include "shell.h"
const std::string FAILED_JOB = "exit 1";
const std::string SUCCESS_JOB = "done";

// flag to set when SIGINT is received
volatile sig_atomic_t sig_flag = 0;

// current command, jobs list, and mutex need to be accessed by signal handler
// AND in shell member functions
std::string command;
std::unordered_map<int, std::pair<std::string, int>> jobMap;
std::mutex jobMapMutex;

// variables for job number management
int smallest = 1;
std::set<int> below;

Shell::Shell() {
  // initialize the command history with an empty command
  this->commandHistory.push_back("");
}

// signal handlers for SIGINT and SIGCHLD
void Shell::interruptSignal(int /*signum */) {
  std::cout << std::endl;
  sig_flag = 1;
  command.erase();
  outputPrompt();
}

void Shell::childSignal(int /* signum */) {
  int status;
  // wait for the child process that just exited
  pid_t pid = waitpid(-1, &status, WNOHANG);
  std::lock_guard<std::mutex> lock(jobMapMutex);
  // if the job is a background process
  if (pid > 0 && jobMap.contains(pid)) {
    int exit_code = WEXITSTATUS(status);
    std::string exit_message = ((exit_code == 0) ? SUCCESS_JOB : FAILED_JOB);
    std::string command = jobMap[pid].first;
    int jobNum = jobMap[pid].second;
    // output an update regarding the process that just completed
    std::cout << std::endl
         << "[" << jobNum << "]\t" << exit_message << "\t" << command << std::endl;
    jobMap.erase(pid);    // remove the job
    returnJobNum(jobNum); // return the job number
    outputPrompt();
    sig_flag = 1;
  }
}

void Shell::outputPrompt() {
  std::string curDirectory = std::filesystem::current_path().filename().string();
  std::cout << "HShell " << curDirectory << " <> " << command << std::flush;
}

// Function to get a single character from the terminal without Enter key press
char Shell::getch() {
  char buf = 0;
  struct termios original, temp;
  fflush(
      stdout); // ensure all pending output is written before changing settings
  if (tcgetattr(0, &original) <
      0) // get the current terminal settings into 'original'
    perror("tcsetattr()");
  temp = original; // copy over the original settings
  temp.c_lflag &=
      ~ICANON; // disable canonical mode, so input is not processed line by line
  temp.c_lflag &= ~ECHO; // disable automatic echo of characters (characters not
                         // printed to terminal)
  temp.c_cc[VMIN] = 1;   // minimum characters for a read operation is 1
  temp.c_cc[VTIME] = 0;  // 'read' timeout is 0, 'read' returns immediately when
                         // minimum num of characters (1) is available
  if (tcsetattr(0, TCSANOW, &temp) <
      0) // applies terminal settings, immediately (TCSANOW)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0) // read one char from terminal into buf
    perror("read()");
  if (tcsetattr(0, TCSADRAIN, &original) <
      0) // revert terminal settings to original
    perror("tcsetattr ~ICANON");
  return buf;
}

// Convert a continuous string into vector of strings
std::vector<std::string> Shell::parseInput(std::string &input) {
  std::vector<std::string> tokens;
  std::stringstream iss(input);
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

// execvp requires a C-style array of char pointers (C-style strings)
// so convert vector of std::string to vector of C-style strings
void Shell::populateArgVector(std::vector<char *> &args, std::vector<std::string> &command) {
  for (const std::string &token : command) {
    args.push_back(const_cast<char *>(token.c_str()));
  }
  args.push_back(
      nullptr); // execvp requires the last element in the array to be NULL
}

void Shell::shellLoop() {
  signal(SIGINT, Shell::interruptSignal); // handle Ctrl + C
  signal(SIGCHLD, Shell::childSignal);    // when child processes terminates
  char ch;                                // to read user input into
  outputPrompt();
  int historyIndex = this->commandHistory.size() - 1;
  while (true) {
    ch = getch();

    // handle Ctrl + D (EOF)
    if (ch == 4) {
      std::vector<std::string> temp = {"exit"};
      handleBuiltins(temp);
    }

    if (ch == 27) { // Check for Escape key (start of sequence for arrow keys)
      ch = getch();
      if (ch == '[') { // second character in arrow key sequence
        ch = getch();
        if (ch == 'A') { // up arrow
          // if there is a previous command
          if (historyIndex - 1 >= 0) {
            historyIndex -= 1;
            // need to remove the existing command from terminal
            int curCommandSize = command.size();
            for (int i = 0; i < curCommandSize; i++) {
              std::cout << "\b \b";
            }
            // access the history command and display
            command = this->commandHistory[historyIndex];
            std::cout << command;
          }
        } else if (ch == 'B') { // down arrow
          // if there is a subsequent command
          if (historyIndex + 1 <
              static_cast<int>(this->commandHistory.size())) {
            historyIndex += 1;
            // need to remove the existing command from terminal
            int curCommandSize = command.size();
            for (int i = 0; i < curCommandSize; i++) {
              std::cout << "\b \b";
            }
            // access the history command and display
            command = this->commandHistory[historyIndex];
            std::cout << command;
          }
        } else if (ch == 'C') {
          // TODO handle right arrow
        } else if (ch == 'D') {
          // TODO handle left arrow
        }
      }
    } else if (ch == 10) { // Check for Enter key --> user entered a command
      std::cout << std::endl;
      std::vector<std::string> vals = parseInput(command);
      sig_flag = 0; // reset signal flag
      restoreHistory();
      executeCommand(vals);
      if (command.size()) {
        addToHistory(command); // if the command wasn't empty, add it to history
      }
      command.clear();
      historyIndex =
          this->commandHistory.size() - 1; // reset the history pointer
      if (sig_flag == 0)
        outputPrompt();
    } else if (ch == 127) { // Check for backspace key
      if (!command.empty()) {
        // Remove the last character from the command
        std::cout << "\b \b"; // Move the cursor back and overwrite the character
                         // with a space
        std::cout.flush();
        // store the original history command before it is modified
        tempHistory(historyIndex, command);
        command.pop_back();
        if (historyIndex != static_cast<int>(this->commandHistory.size()) - 1) {
          this->commandHistory[historyIndex] = command;
        }
      }
    } else if (ch == 9) {
      // TODO handle tab key
    } else {
      std::cout << ch;    // Print the character as it is typed
      std::cout.flush();  // Flush the output to make it visible immediately
      command += ch; // update the command
      if (historyIndex != static_cast<int>(this->commandHistory.size()) - 1) {
        this->commandHistory[historyIndex] = command;
      }
    }
  }
}

void Shell::executeCommand(std::vector<std::string> &command) {
  if (command.empty())
    return;
  // only if command is not a built-in, then handle with fork, execvp etc.
  if (handleBuiltins(command)) {
    handleRedirection(command);
  }
}

// catch functions that can be handled without a child process
int Shell::handleBuiltins(std::vector<std::string> &command) {
  if (command[0] == "cd") {
    changeDirectory(command);
    return 0;
  }
  if (command[0] == "pwd") {
    std::cout << std::filesystem::current_path().string() << std::endl;
    return 0;
  }
  if (command[0] == "exit") {
    std::cout << "Exiting shell. Goodbye." << std::endl;
    exit(EXIT_SUCCESS);
  }
  if (command[0] == "jobs") {
    printJobs();
    return 0;
  }
  if (command[0] == "history") {
    printHistory();
    return 0;
  }
  return 1;
}

// for the 'jobs' command
void Shell::printJobs() {
  // Print table header
  std::lock_guard<std::mutex> lock(jobMapMutex);
  if (jobMap.size() > 0) {
    // sort the map into a vector, based on the jobNum (second item in the
    // value)
    std::vector<std::pair<int, std::pair<std::string, int>>> sortedJobs(jobMap.begin(),
                                                    jobMap.end());
    std::sort(sortedJobs.begin(), sortedJobs.end(),
         [](const auto &lhs, const auto &rhs) {
           return lhs.second.second < rhs.second.second;
         });
    // Iterate through the unordered_map and print job entries, from smallest to
    // largest job number
    for (const auto &entry : sortedJobs) {
      std::cout << "[" << entry.second.second << "]"
           << "\t" << entry.second.first << std::endl;
    }
  }
}

// handle cd
void Shell::changeDirectory(std::vector<std::string> &command) {
  // execute cd with no arguments, or with tilda
  if (command.size() == 1 || command[1] == "~") {
    const char *home_directory = getenv("HOME");
    if (home_directory) {
      if (chdir(home_directory) != 0) {
        perror("HShell");
      }
    } else {
      std::cerr << "Error: HOME environment variable not set." << std::endl;
    }
  } else if (command.size() >
             1) { // otherwise, cd has an argument (besides tilda)
    if (chdir(command[1].c_str()) != 0) {
      perror("HShell");
    }
  } else {
    std::cerr << "Usage: cd <directory>" << std::endl;
  }
}

// regenerate command string from tokens
std::string Shell::regenerateCommand(std::vector<std::string> &command) {
  std::string separator = " ";
  return accumulate(next(command.begin()), command.end(), command.front(),
                    [separator](const std::string &acc, const std::string &str) {
                      return acc + separator + str;
                    });
}

// fork and run the user's command; also takes file descriptors for terminal,
// which will be changed if the command involved input/output redirection
void Shell::generateChild(std::vector<std::string> &command, int originalStdin,
                          int originalStdout, bool inBackground) {
  std::vector<char *> args;
  populateArgVector(
      args, command); // execvp requires array of char pointers, not std::vector
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    // Restore standard input and output
    dup2(originalStdin, STDIN_FILENO);
    dup2(originalStdout, STDOUT_FILENO);
  } else if (pid == 0) {
    // Child process
    if (inBackground) { // if command to run in background
      // Close standard input and output
      close(STDIN_FILENO);
      close(STDOUT_FILENO);

      // Create a new session, so the process is disconnected from this terminal
      if (setsid() == -1) {
        perror("setsid");
        exit(EXIT_FAILURE);
      }
    }
    // Execute the command
    execvp(args[0], args.data());
    perror("HShell");
    exit(EXIT_FAILURE);
  } else {
    // Parent process
    if (!inBackground) {
      waitpid(pid, nullptr, 0);
    } else {
      addJob(pid, command);
    }
  }

  // Restore standard input and output
  // close where the STDIN and STDOUT currently point, and point them to default
  dup2(originalStdin, STDIN_FILENO);
  dup2(originalStdout, STDOUT_FILENO);
}

void Shell::addJob(pid_t pid, std::vector<std::string> & command) {
    // initialize mask for blocking SIGCHLD
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    // Block SIGCHLD while starting a new job
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
      perror("sigprocmask");
    }
    {
      std::lock_guard<std::mutex> lock(jobMapMutex);
      // create an entry in the jobMap table with PID, jobNum, and command
      jobMap[pid] = {regenerateCommand(command), createJobNum()};
      std::cout << "[" << jobMap[pid].second << "] " << pid << std::endl;
    }
    if (sigprocmask(SIG_UNBLOCK, &mask, nullptr) == -1) {
      perror("sigprocmask");
    }
}

int Shell::handleRedirection(std::vector<std::string> &command) {
  std::string inputFile;
  std::string outputFile;
  size_t ptr = 0;
  // detect the ">" operator for output redirection
  // if multiple such operators are found, the file pointed to by the last one
  // will be used
  while (ptr < command.size()) {
    // if ">" is found, and a file follows
    if (command[ptr] == ">" && ptr + 1 < command.size()) {
      // store the file; remove the arrow and file from command
      outputFile = command[ptr + 1];
      command.erase(command.begin() + ptr, command.begin() + ptr + 2);
    } else {
      ptr++;
    }
  }

  // check if user requested for process to run in the background, and modify
  // the command accordingly
  bool inBackground = false;
  if (command.size()) {
    if (command.back() == "&") {
      inBackground = true;
      command.pop_back();
    } else if (command.back().back() == '&') {
      inBackground = true;
      command.back().pop_back();
    }
  }

  // store the file descriptors for default stdout and stdin
  int originalStdin = dup(STDIN_FILENO);
  int originalStdout = dup(STDOUT_FILENO);

  // Set up file descriptor for output redirection
  if (!outputFile.empty()) {
    int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
      perror("open");
      return 1;
    }
    dup2(fd, STDOUT_FILENO); // stdout now routes to the newly opened file
    close(fd);
  }

  // the command is run, and file descriptors subsequently reset if necessary
  generateChild(command, originalStdin, originalStdout, inBackground);
  return 0;
}

void Shell::addToHistory(std::string newCommand) {
  const int MAXSIZE =
      51; // small number used for testing, but can increase if desired
  // remove the command used earliest if at maximum
  if (this->commandHistory.size() == MAXSIZE) {
    this->commandHistory.pop_front();
  }
  this->commandHistory.pop_back();            // pop off the "" command
  this->commandHistory.push_back(newCommand); // add the new command
  this->commandHistory.push_back("");         // add back the "" command
}

void Shell::printHistory() {
  int const MAX_COMMANDS = 15; // only print the last 15 commands
  for (int i = std::max(0, static_cast<int>(this->commandHistory.size()) -
                          MAX_COMMANDS - 1);
       i < static_cast<int>(this->commandHistory.size()) - 1; i++) {
    std::cout << i + 1 << " " << this->commandHistory[i] << std::endl;
  }
}

// when history commands are edited, restore them after a command is entered
void Shell::restoreHistory() {
  // restore history
  for (auto pair : this->modifiedHistory) {
    this->commandHistory[pair.first] = pair.second;
  }
  this->modifiedHistory.clear();
}

// when history commands are edited, want to store the original command stored
// in history, to restore later
void Shell::tempHistory(int historyIndex, std::string command) {
  if (!this->modifiedHistory.contains(historyIndex) &&
      historyIndex != static_cast<int>(this->commandHistory.size()) - 1) {
    this->modifiedHistory[historyIndex] = command;
  }
}

// generate the next smallest job number (positive integer)
int Shell::createJobNum() {
  if (below.size()) {
    int temp = *below.begin();
    if (temp < smallest) {
      below.erase(temp);
      return temp;
    }
  }
  smallest++;
  return smallest - 1;
}

// for when job completes, return that number to the pool
void Shell::returnJobNum(int num) {
  if (num >= smallest) {
    return;
  }
  if (num + 1 == smallest) {
    smallest -= 1;
  } else {
    below.insert(num);
  }
}

void printVector(std::vector<std::string> &vec) {
  for (std::string v : vec) {
    std::cout << v << " ";
  }
  std::cout << std::endl;
}

void printDeque(std::deque<std::string> &d) {
  for (std::string v : d) {
    std::cout << v << " ";
  }
  std::cout << std::endl;
}

void printString(std::string s) {
  for (char c : s) {
    std::cout << c << " ";
  }
  std::cout << std::endl;
}