#pragma once

// Base controller class
// by eien86

#include <cstdint>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <string>
#include <sstream>

namespace jaffar
{

struct input_t
{
  // Player inputs
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool start = false;
  bool select = false;
  bool square = false;
  bool triangle = false;
  bool circle = false;
  bool cross = false;
  bool ltrigger = false;
  bool rtrigger = false;
  int32_t rightAnalogX = 0;
  int32_t rightAnalogY = 0;
  int32_t leftAnalogX = 0;
  int32_t leftAnalogY = 0;

  // Console inputs
  bool power = false;
  bool home = false;
};

class InputParser
{
public:

  enum controller_t { none, joypad };

  InputParser(const nlohmann::json &config)
  {
  }

  inline input_t parseInputString(const std::string &inputString) const
  {
    // Storage for the input
    input_t input;

    // Currently read character
    char c;

    // Converting input into a stream for parsing
    std::istringstream ss(inputString);

    // Input separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // Parsing console inputs
    parseConsoleInput(input, ss, inputString);

    // Input separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);
    
    // Parsing controller 1 inputs
    parseGamepadInput(input, ss, inputString);

    // End separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // If its not the end of the stream, then extra values remain and its invalid
    c = ss.get();
    if (ss.eof() == false) reportBadInputString(inputString, c);
    
    // Returning input
    return input;
  };

  private:

  static void parseConsoleInput(input_t& input, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    char c;

    c = ss.get();
    if (c != '.' && c != 'h') reportBadInputString(inputString, c);
    if (c == 'h') input.home = true;
    if (c == '.') input.home = false;

    c = ss.get();
    if (c != '.' && c != 'P') reportBadInputString(inputString, c);
    if (c == 'P') input.power = true;
    if (c == '.') input.power = false;
  }

  static void parseGamepadInput(input_t& input, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    char c;

    // Up
    c = ss.get();
    if (c != '.' && c != 'U') reportBadInputString(inputString, c);
    if (c == 'U') input.up = true;

    // Down
    c = ss.get();
    if (c != '.' && c != 'D') reportBadInputString(inputString, c);
    if (c == 'D') input.down = true;

    // Left
    c = ss.get();
    if (c != '.' && c != 'L') reportBadInputString(inputString, c);
    if (c == 'L') input.left = true;

    // Right
    c = ss.get();
    if (c != '.' && c != 'R') reportBadInputString(inputString, c);
    if (c == 'R') input.right = true;

    // Start
    c = ss.get();
    if (c != '.' && c != 'S') reportBadInputString(inputString, c);
    if (c == 'S') input.start = true;

    // Select
    c = ss.get();
    if (c != '.' && c != 's') reportBadInputString(inputString, c);
    if (c == 's') input.select = true;

    // Button Square
    c = ss.get();
    if (c != '.' && c != 'Q') reportBadInputString(inputString, c );
    if (c == 'Q') input.square = true;

    // Button Triangle
    c = ss.get();
    if (c != '.' && c != 'T') reportBadInputString(inputString, c);
    if (c == 'T') input.triangle = true;

    // Button Circle
    c = ss.get();
    if (c != '.' && c != 'C') reportBadInputString(inputString, c);
    if (c == 'C') input.circle = true;
        
    // Button Cross
    c = ss.get();
    if (c != '.' && c != 'X') reportBadInputString(inputString, c);
    if (c == 'X') input.cross = true;
    
    // Button L Trigger
    c = ss.get();
    if (c != '.' && c != 'l') reportBadInputString(inputString, c);
    if (c == 'l') input.ltrigger = true;

    // Button R Trigger
    c = ss.get();
    if (c != '.' && c != 'r') reportBadInputString(inputString, c);
    if (c == 'r') input.rtrigger = true;

    // Parsing Separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // Parsing Right Analog X
    char rightAnalogXString[7];
    rightAnalogXString[0] = ss.get();
    rightAnalogXString[1] = ss.get();
    rightAnalogXString[2] = ss.get();
    rightAnalogXString[3] = ss.get();
    rightAnalogXString[4] = ss.get();
    rightAnalogXString[5] = ss.get();
    rightAnalogXString[6] = '\0';
    input.rightAnalogX = atoi(rightAnalogXString);

    // Parsing comma
    c = ss.get();
    if (c != ',') reportBadInputString(inputString, c);

    // Parsing Right Analog Y
    char rightAnalogYString[7];
    rightAnalogYString[0] = ss.get();
    rightAnalogYString[1] = ss.get();
    rightAnalogYString[2] = ss.get();
    rightAnalogYString[3] = ss.get();
    rightAnalogYString[4] = ss.get();
    rightAnalogYString[5] = ss.get();
    rightAnalogYString[6] = '\0';
    input.rightAnalogY = atoi(rightAnalogYString);

    // Parsing comma
    c = ss.get();
    if (c != ',') reportBadInputString(inputString, c);

    // Parsing Left Analog X
    char leftAnalogXString[7];
    leftAnalogXString[0] = ss.get();
    leftAnalogXString[1] = ss.get();
    leftAnalogXString[2] = ss.get();
    leftAnalogXString[3] = ss.get();
    leftAnalogXString[4] = ss.get();
    leftAnalogXString[5] = ss.get();
    leftAnalogXString[6] = '\0';
    input.leftAnalogX = atoi(leftAnalogXString);

    // Parsing comma
    c = ss.get();
    if (c != ',') reportBadInputString(inputString, c);

    // Parsing Left Analog Y
    char leftAnalogYString[7];
    leftAnalogYString[0] = ss.get();
    leftAnalogYString[1] = ss.get();
    leftAnalogYString[2] = ss.get();
    leftAnalogYString[3] = ss.get();
    leftAnalogYString[4] = ss.get();
    leftAnalogYString[5] = ss.get();
    leftAnalogYString[6] = '\0';
    input.leftAnalogY = atoi(leftAnalogYString);
  }

  static inline void reportBadInputString(const std::string &inputString, const char c)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s' - Read: '%c'\n", inputString.c_str(), c);
  }

}; // class InputParser

} // namespace jaffar