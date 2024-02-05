#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <sstream>

using namespace std;

void signalHandler(int signum);

vector<string> parse_input(string input);

void print_vector(vector<string>& vec);

void change_directory(vector<string>& command);

int generate_child(vector<char*>& args);

void execute_command(vector<string>& command);

void populate_arg_vector(vector<char*>& args, vector<string>& command);

#endif
