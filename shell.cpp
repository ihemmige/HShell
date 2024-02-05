#include "shell.h"

// Signal handler function
void signalHandler(int signum) {
    cout << "Interrupt signal (" << signum << ") received.\n";
    cout << "hsh> " << flush;
}

vector<string> parse_input(string input) {
  vector<string> tokens;
  stringstream iss(input);
  string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

void print_vector(vector<string>& vec) {
  for (auto v : vec) {
    cout << v << " ";
  }
  cout << endl;
}

void change_directory(vector<string>& command) {
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

int generate_child(vector<char*>& args) {
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

void populate_arg_vector(vector<char*>& args, vector<string>& command) {
    args.reserve(command.size() + 1);
    for (const auto& token : command) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr);
}

void execute_command(vector<string>& command) {
    vector<char*> args;
    populate_arg_vector(args, command);
    
    if (command[0] == "cd") {
      change_directory(command);
      return;
    }
    if (command[0] == "exit") {
      cout << "Exiting shell. Goodbye." << endl;
      exit(EXIT_SUCCESS);
    }
    generate_child(args);
}