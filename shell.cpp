#include "shell.h"

void signalHandler(int signum) {
    cout << "Signal (" << signum << ") received.\n";
    outputPrompt();
}

vector<string> parseInput(string input) {
  vector<string> tokens;
  stringstream iss(input);
  string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

void printVector(vector<string>& vec) {
  for (auto v : vec) {
    cout << v << " ";
  }
  cout << endl;
}

void changeDirectory(vector<string>& command) {
  if (command.size() == 1 || command[1] == "~") {
    const char* home_directory = getenv("HOME");
    if (home_directory) {
      if (chdir(home_directory) != 0) {
        perror("hsh");
      }
    } else {
      cerr << "Error: HOME environment variable not set." << std::endl;
    }
  } else if (command.size() > 1) {
    if (chdir(command[1].c_str()) != 0) {
      perror("hsh");
    }
  } else {
      cerr << "Usage: cd <directory>" << endl;
  }
}

void generateChild(vector<string>& command, int originalStdin, int originalStdout) {
  vector<char*> args;
  populateArgVector(args, command);
  pid_t pid = fork();
  if (pid == -1) {
      perror("fork");
      dup2(originalStdin, STDIN_FILENO);
      dup2(originalStdout, STDOUT_FILENO);
  } else if (pid == 0) {
      // Child process
      execvp(args[0], args.data());
      perror("hsh");
      exit(EXIT_FAILURE);
  } else {
      // Parent process
      wait(nullptr);
  }
    // restore standard input and output
    dup2(originalStdin, STDIN_FILENO);
    dup2(originalStdout, STDOUT_FILENO);
}

int handleRedirection(vector<string>& command) {
    string inputFile;
    string outputFile;
    size_t ptr = 0;
    while (ptr < command.size()) {
      if (command[ptr] == ">" && ptr + 1 < command.size()) {
          outputFile = command[ptr + 1];
          command.erase(command.begin() + ptr, command.begin() + ptr + 2);
      }
      else {
        ptr++;
      }
    }

    int originalStdin = dup(STDIN_FILENO);
    int originalStdout = dup(STDOUT_FILENO);

    // Set up file descriptors for output redirection
    if (!outputFile.empty()) {
        int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("open");
            return 1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    generateChild(command, originalStdin, originalStdout); // the command is run, and file descriptors reset if necessary
    return 0;
}

void populateArgVector(vector<char*>& args, vector<string>& command) {
    // args.reserve(command.size() + 1);
    for (const auto& token : command) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr);
}

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

void executeCommand(vector<string>& command) {
    if (command.empty()) return;
    if (handleBuiltins(command)) {
      handleRedirection(command);
    }
}

void outputPrompt() {
  string curDirectory = filesystem::current_path().filename().string();
  cout << "HShell " << curDirectory << " > ";
  cout << flush;
}