#ifndef GAME_LOGIC_CLIENT_HPP
#define GAME_LOGIC_CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdio> 
#include <thread>
#include <mutex>
#include <unordered_map>
#include <csignal>

// Cores para terminal
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

void loadCardDrawings();
int getTotalCards();
void showScreenElements();
void * waitStartGameSignal();
char getKeyPress();
std::unordered_map<std::string, std::string> createMap(std::string);
std::vector<int> splitStringIntoInts(std::string &, std::string);
bool updateGameState(std::unordered_map < std::string, std::string > &);
void * listenServer();
void * sendMsg();
void printCardVariationMsg(int);
void handleSignals();
void terminateAll(int);

std::vector<char> cardsSequence = {'A', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'J', 'Q', 'K'};

#endif