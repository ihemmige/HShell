#include <iostream>
#include <csignal>

using namespace std;

// Signal handler function
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    cout << "hsh> " << flush;
}

int main() {
  string buffer;
  signal(SIGINT, signalHandler);
  while (1) {
    cout << "hsh> ";
    if (!getline(std::cin, buffer)) {

      // If user entered Ctrl+D
      if (cin.eof()) {
        cout << "Ctrl+D detected. Closing shell." << endl;
      }
      // Handle other errors
      else {
        cerr << "Error reading input. Exiting." << endl;
      }
      break;
    } else {
      cout << buffer << endl;
    }
  }
  return 0;
}