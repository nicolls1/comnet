#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>

using namespace std;

vector<std::string> split(const string &s, char delim);

int main(int argc, char* argv[]) {
  
  string input;

  cout << "Enter numbers" << endl;
  getline(cin, input);
  vector<string> split_line;
  split_line = split(input, ' ');

  unsigned long sum = 0;
  vector<string>::iterator it = split_line.begin();
  for(; it != split_line.end(); ++it) {
    sum += (unsigned short) atoi(it->c_str());
  }

  sum = (sum & 0xffff) + (sum >> 16);

  cout << "Checksum as integer: " << (unsigned short) ~sum << endl;

  return 0;
}

std::vector<std::string>& split(const std::string &s, 
    char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

