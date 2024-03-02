#include "shell.h"

volatile sig_atomic_t flag = 0;

/* Functions for interacting with user, processing user input */

void signalHandler(int /*signum */) {
    cout << endl;
    flag = 1;
    outputPrompt();
}

void outputPrompt() {
  string curDirectory = filesystem::current_path().filename().string();
  cout << "HShell " << curDirectory << " > " << flush;
}

// Function to get a single character from the terminal without Enter key press
char getch() {
  char buf = 0;
  struct termios original, temp;
  fflush(stdout); // ensure all pending output is written before changing settings
  if (tcgetattr(0, &original) < 0) // get the current terminal settings into 'original'
    perror("tcsetattr()");
  temp = original; // copy over the original settings
  temp.c_lflag &= ~ICANON; // disable canonical mode, so input is not processed line by line
  temp.c_lflag &= ~ECHO; // disable automatic echo of characters (characters not printed to terminal)
  temp.c_cc[VMIN] = 1; // minimum characters for a read operation is 1
  temp.c_cc[VTIME] = 0; // 'read' timeout is 0, 'read' returns immediately when minimum num of characters (1) is available
  if (tcsetattr(0, TCSANOW, &temp) < 0) // applies terminal settings, immediately (TCSANOW)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0) // read one char from terminal into buf
    perror("read()");
  if (tcsetattr(0, TCSADRAIN, &original) < 0) // revert terminal settings to original
    perror("tcsetattr ~ICANON");
  return buf;
}

// Convert a continuous string into vector of strings
vector<string> parseInput(string& input) {
  vector<string> tokens;
  stringstream iss(input);
  string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

// execvp requires a C-style array of char pointers (C-style strings)
// so convert vector of std::string to vector of C-style strings
void populateArgVector(vector<char*>& args, vector<string>& command) {
    for (const auto& token : command) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr); // execvp requires the last element in the array to be NULL
}

/* Functions for triggering execution */
void shellLoop() {
  signal(SIGINT, signalHandler); // handle Ctrl + C
  string command;
  char ch; // to read user input into
  outputPrompt();
  deque<string> commandHistory = {""}; // initialize w/ empty string, allow empty command as option when using up-down arrows
  int historyIndex = commandHistory.size() - 1;
  while (true) {
    ch = getch();
    
    // handle Ctrl + D (EOF)
    if (ch == 4) {
      vector<string> temp = {"exit"};
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
                cout << "\b \b";
            }
            // access the history command and display
            command = commandHistory[historyIndex];
            cout << command;
          }
        }
        else if (ch == 'B') { // down arrow
          // if there is a subsequent command
          if (historyIndex + 1 < static_cast<int>(commandHistory.size())) {
            historyIndex += 1;
            // need to remove the existing command from terminal
            int curCommandSize = command.size();
            for (int i = 0; i < curCommandSize; i++) {
                cout << "\b \b";
            }
            // access the history command and display
            command = commandHistory[historyIndex];
            cout << command;
          }
        } else if (ch == 'C') {
          //TODO handle right arrow
        } else if (ch == 'D') {
          //TODO handle left arrow
        }
      }
    }
    else if (ch == 10) { // Check for Enter key --> user entered a command
      cout << endl;
      vector<string> vals = parseInput(command);
      flag = 0; // reset signal flag
      executeCommand(vals);
      if (command.size()) addToHistory(commandHistory, command); // if the command wasn't empty, add it to history
      command.clear();
      historyIndex = commandHistory.size() - 1; // reset the history pointer
      if (flag == 0) outputPrompt();
    } else if (ch == 127) { // Check for backspace key
      if (!command.empty()) {
        // Remove the last character from the command
        cout << "\b \b"; // Move the cursor back and overwrite the character with a space
        command.pop_back();
        cout.flush();
      }
    } else if (ch == 9) {
      //TODO handle tab key
    } else {
      cout << ch;   // Print the character as it is typed
      cout.flush(); // Flush the output to make it visible immediately
      command += ch; // update the command
    }
  }
}

void executeCommand(vector<string>& command) {
    if (command.empty()) return;
    // only if command is not a built-in, then handle with fork, execvp etc.
    if (handleBuiltins(command)) {
      handleRedirection(command);
    }
}

// catch functions that can be handled without a child process
int handleBuiltins(vector<string>& command) {
    if (command[0] == "cd") {
      changeDirectory(command);
      return 0;
    }
    if (command[0] == "pwd") {
      cout << filesystem::current_path().string() << endl;
      return 0;
    }
    if (command[0] == "exit") {
      cout << "Exiting shell. Goodbye." << endl;
      exit(EXIT_SUCCESS);
    }
    return 1;
}

// handle cd
void changeDirectory(vector<string>& command) {
  // execute cd with no arguments, or with tilda
  if (command.size() == 1 || command[1] == "~") {
    const char* home_directory = getenv("HOME");
    if (home_directory) {
      if (chdir(home_directory) != 0) {
        perror("HShell");
      }
    } else {
      cerr << "Error: HOME environment variable not set." << std::endl;
    }
  } else if (command.size() > 1) { // otherwise, cd has an argument (besides tilda)
    if (chdir(command[1].c_str()) != 0) {
      perror("HShell");
    }
  } else {
      cerr << "Usage: cd <directory>" << endl;
  }
}

// fork and run the user's command; also takes file descriptors for terminal, which will be changed
// if the command involved input/output redirection
void generateChild(vector<string>& command, int originalStdin, int originalStdout) {
  vector<char*> args;
  populateArgVector(args, command); // execvp requires array of char pointers, not std::vector
  pid_t pid = fork();
  if (pid == -1) {
      perror("fork");
      // restore standard input and output
      dup2(originalStdin, STDIN_FILENO);
      dup2(originalStdout, STDOUT_FILENO);
  } else if (pid == 0) {
      // Child process
      execvp(args[0], args.data()); // execute the command
      perror("HShell");
      exit(EXIT_FAILURE);
  } else {
      // Parent process
      wait(nullptr);
  }
    // restore standard input and output
    // close where the STDIN and STDOUT currently point, and point them to default
    dup2(originalStdin, STDIN_FILENO);
    dup2(originalStdout, STDOUT_FILENO);
}

/* Functions for advanced functionality */
int handleRedirection(vector<string>& command) {
    string inputFile;
    string outputFile;
    size_t ptr = 0;
    // detect the ">" operator for output redirection
    // if multiple such operators are found, the file pointed to by the last one will be used
    while (ptr < command.size()) {
      // if ">" is found, and a file follows
      if (command[ptr] == ">" && ptr + 1 < command.size()) {
          // store the file; remove the arrow and file from command
          outputFile = command[ptr + 1];
          command.erase(command.begin() + ptr, command.begin() + ptr + 2);
      }
      else {
        ptr++;
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
    generateChild(command, originalStdin, originalStdout); 
    return 0;
}

void addToHistory(deque<string>& commandHistory, string newCommand) {
  const int MAXSIZE = 51; // small number used for testing, but can increase if desired
  // remove the command used earliest if at maximum
  if (commandHistory.size() == MAXSIZE) {
    commandHistory.pop_front();
  }
  commandHistory.pop_back(); // pop off the "" command
  commandHistory.push_back(newCommand); // add the new command
  commandHistory.push_back(""); // add back the "" command
}

/* Functions for testing and argument visibility */
void printVector(vector<string>& vec) {
  for (string v : vec) {
    cout << v << " ";
  }
  cout << endl;
}

void printDeque(deque<string>& d) {
  for (string v : d) {
    cout << v << " ";
  }
  cout << endl;
}

void printString(string s) {
  for (char c : s) {
    cout << c << " ";
  }
  cout << endl;
}