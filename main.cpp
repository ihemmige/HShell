#include "shell.h"
#include <filesystem>

using namespace std;

int main() {
  string buffer;
  signal(SIGINT, signalHandler);
  while (1) {
    string curDirectory = filesystem::current_path().filename().string();
    cout << "HShell " << curDirectory << " > ";
    if (!getline(cin, buffer)) {
      // If user entered Ctrl+D
      if (cin.eof()) {
        cout << "Exiting shell. Goodbye." << endl;
      }
      // Handle other errors
      else {
        cerr << "Error reading input. Exiting." << endl;
      }
      break;
    } else {
      vector<string> vals = parse_input(buffer);
      execute_command(vals);
    }
  }
	return 0;
}