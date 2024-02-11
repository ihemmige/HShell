#include "shell.h"

void signalHandler(int signum) {
    cout << "Signal (" << signum << ") received.\n";
    cout << "hsh> " << flush;
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

int generateChild(vector<char*>& args) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args.data());
        perror("hsh");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(nullptr);
    }
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
    vector<char*> args;
    populateArgVector(args, command);
    if (command.empty()) return;
    if (handleBuiltins(command)) {
      generateChild(args);
    }
}