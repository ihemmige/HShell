#include "shell.h"

using namespace std;

int main() {
  string buffer;
  signal(SIGINT, signalHandler);
  while (1) {
    cout << "hsh> ";
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
      // print_vector(vals);
      execute_command(vals);
    }
  }
	return 0;
}