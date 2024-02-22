#include "shell.h"
using namespace std;

void shellLoop() {
  string command;
  char ch;
  outputPrompt();
  deque<string> commandHistory;
  commandHistory.push_back("");
  size_t historyIndex = commandHistory.size()-1;
  while (true) {
    ch = getch();
    // handle Ctrl + D (EOF)
    if (ch == 4) {
      vector<string> temp = {"exit"};
      handleBuiltins(temp);
    }
    
    if (ch == 27) { // Check for Escape key (arrow keys)
      ch = getch(); // Read the next character to determine the specific arrow key
      if (ch == '[') {
        ch = getch(); // Read the actual arrow key
        if (ch == 'A') { // up arrow
          if (historyIndex - 1 < commandHistory.size()) {
            historyIndex -= 1;
            int curCommandSize = command.size();
            for (int i = 0; i < curCommandSize; i++) {
                cout << "\b \b";
            }
            command = commandHistory[historyIndex];
            cout << command;
          }
        }
        else if (ch == 'B') { // down arrow
          if (historyIndex + 1 < commandHistory.size()) {
            historyIndex += 1;
            int curCommandSize = command.size();
            for (int i = 0; i < curCommandSize; i++) {
                cout << "\b \b";
            }
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
    else if (ch == 10) { // Check for Enter key
      cout << endl;
      vector<string> vals = parseInput(command);
      executeCommand(vals);
      if (command.size()) addToHistory(commandHistory, command);
      command.clear();
      historyIndex = commandHistory.size() - 1;
      outputPrompt();
    }
    else if (ch == 127) { // Check for backspace key
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
      command += ch;
    }
  }
}

int main() {
  signal(SIGINT, signalHandler);
  shellLoop();
  return 0;
}