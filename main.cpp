#include "shell.h"
#include <termios.h>

using namespace std;

// Function to get a single character from the terminal without Enter key press
char getch() {
  char buf = 0;
  struct termios old = {0};
  fflush(stdout);
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return buf;
}

void shellLoop() {
  string command;
  char ch;
  outputPrompt();

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
        if (ch == 'A') {
          // TODO implement command history
          cout << "Up arrow key pressed" << endl;
          outputPrompt();
          continue; // Skip the character handling below
        }
        else if (ch == 'B') {
          // TODO implement command history
          cout << "Down arrow key pressed" << endl;
          outputPrompt();
          continue; // Skip the character handling below
        }
      }
    }
    else if (ch == 10) { // Check for Enter key
      cout << endl;
      vector<string> vals = parseInput(command);
      executeCommand(vals);
      command.clear();
      outputPrompt();
      continue; // Skip the character handling below
    }
    else if (ch == 127) { // Check for backspace key
      if (!command.empty()) {
        // Remove the last character from the command
        cout << "\b \b"; // Move the cursor back and overwrite the character with a space
        command.pop_back();
        cout.flush();
      }
      continue; // Skip the character handling below
    }
    cout << ch;   // Print the character as it is typed
    cout.flush(); // Flush the output to make it visible immediately
    command += ch;
  }
}

int main() {
  signal(SIGINT, signalHandler);
  shellLoop();
  return 0;
}