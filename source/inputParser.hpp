#pragma once

// Base controller class
// by eien86

#include <cstdint>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <string>
#include <sstream>
#include <libretro.h>

namespace jaffar
{

// enum JoypadKey {
//   B       = 1 >> RETRO_DEVICE_ID_JOYPAD_B     ,
//   Y       = 1 >> RETRO_DEVICE_ID_JOYPAD_Y     ,
//   SELECT  = 1 >> RETRO_DEVICE_ID_JOYPAD_SELECT,
//   START   = 1 >> RETRO_DEVICE_ID_JOYPAD_START ,
//   UP      = 1 >> RETRO_DEVICE_ID_JOYPAD_UP    ,
//   DOWN    = 1 >> RETRO_DEVICE_ID_JOYPAD_DOWN  ,
//   LEFT    = 1 >> RETRO_DEVICE_ID_JOYPAD_LEFT  ,
//   RIGHT   = 1 >> RETRO_DEVICE_ID_JOYPAD_RIGHT ,
//   A       = 1 >> RETRO_DEVICE_ID_JOYPAD_A     ,
//   X       = 1 >> RETRO_DEVICE_ID_JOYPAD_X     ,
//   L       = 1 >> RETRO_DEVICE_ID_JOYPAD_L     ,
//   R       = 1 >> RETRO_DEVICE_ID_JOYPAD_R     ,
//   L2      = 1 >> RETRO_DEVICE_ID_JOYPAD_L2    ,
//   R2      = 1 >> RETRO_DEVICE_ID_JOYPAD_R2    ,
//   L3      = 1 >> RETRO_DEVICE_ID_JOYPAD_L3    ,
//   R3      = 1 >> RETRO_DEVICE_ID_JOYPAD_R3    
// };

struct input_t
{
};

class InputParser
{
public:

  enum controller_t { none, joypad };

  InputParser(const nlohmann::json &config)
  {
    // Parsing controller 1 type
    {
      bool isTypeRecognized = false;
      const auto controllerType = jaffarCommon::json::getString(config, "Controller 1 Type");
      if (controllerType == "None")   { _port1ControllerType = controller_t::none; isTypeRecognized = true; }
      // if (controllerType == "Joypad") { _port1ControllerType = controller_t::joypad;  isTypeRecognized = true; }
      if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Controller 1 type not recognized: '%s'\n", controllerType.c_str()); 
    }

    // Parsing controller 2 type
    {
      bool isTypeRecognized = false;
      const auto controllerType = jaffarCommon::json::getString(config, "Controller 2 Type");
      if (controllerType == "None")   { _port2ControllerType = controller_t::none; isTypeRecognized = true; }
      // if (controllerType == "Joypad") { _port2ControllerType = controller_t::joypad;  isTypeRecognized = true; }
      if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Controller 2 type not recognized: '%s'\n", controllerType.c_str()); 
    }
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
    // parseConsoleInput(input.reset, ss, inputString);

    // Input separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);
    
    // Parsing controller 1 inputs
    // if (_port1ControllerType == controller_t::joypad) parseGamepadInput(input.port1, ss, inputString);

    // Input separator
    if (_port2ControllerType != controller_t::none)
    {
      c = ss.get();
      if (c != '|') reportBadInputString(inputString, c);
    }

    // Parsing controller 2 inputs
    // if (_port2ControllerType == controller_t::joypad) parseGamepadInput(input.port2, ss, inputString);

    // End separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // If its not the end of the stream, then extra values remain and its invalid
    c = ss.get();
    if (ss.eof() == false) reportBadInputString(inputString, c);
    
    // Returning input
    return input;
  };

  controller_t getController1Type() const { return _port1ControllerType; }
  controller_t getController2Type() const { return _port2ControllerType; }

  private:

  static void parseConsoleInput(bool& reset, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    // char c;

    // c = ss.get();
    // if (c != '.' && c != 'r') reportBadInputString(inputString, c);
    // if (c == 'r') reset = true;
    // if (c == '.') reset = false;
  }

  static void parseGamepadInput(uint16_t& code, std::istringstream& ss, const std::string& inputString)
  {
    // // Currently read character
    // char c;

    // // Cleaning code
    // code = 0;

    // // Up
    // c = ss.get();
    // if (c != '.' && c != 'U') reportBadInputString(inputString, c);
    // if (c == 'U') code |= RETRO_DEVICE_ID_JOYPAD_UP;

    // // Down
    // c = ss.get();
    // if (c != '.' && c != 'D') reportBadInputString(inputString, c);
    // if (c == 'D') code |= RETRO_DEVICE_ID_JOYPAD_DOWN;

    // // Left
    // c = ss.get();
    // if (c != '.' && c != 'L') reportBadInputString(inputString, c);
    // if (c == 'L') code |= RETRO_DEVICE_ID_JOYPAD_LEFT;

    // // Right
    // c = ss.get();
    // if (c != '.' && c != 'R') reportBadInputString(inputString, c);
    // if (c == 'R') code |= RETRO_DEVICE_ID_JOYPAD_RIGHT;

    // // Start
    // c = ss.get();
    // if (c != '.' && c != 'S') reportBadInputString(inputString, c);
    // if (c == 'S') code |= RETRO_DEVICE_ID_JOYPAD_START;

    // // Select
    // c = ss.get();
    // if (c != '.' && c != 's') reportBadInputString(inputString, c);
    // if (c == 's') code |= RETRO_DEVICE_ID_JOYPAD_SELECT;

    // // Button B
    // c = ss.get();
    // if (c != '.' && c != 'B') reportBadInputString(inputString, c );
    // if (c == 'B') code |= RETRO_DEVICE_ID_JOYPAD_B;

    // // Button Y
    // c = ss.get();
    // if (c != '.' && c != 'Y') reportBadInputString(inputString, c);
    // if (c == 'Y') code |= RETRO_DEVICE_ID_JOYPAD_Y;

    // // Button A
    // c = ss.get();
    // if (c != '.' && c != 'A') reportBadInputString(inputString, c);
    // if (c == 'A') code |= RETRO_DEVICE_ID_JOYPAD_A;
        
    // // Button X
    // c = ss.get();
    // if (c != '.' && c != 'X') reportBadInputString(inputString, c);
    // if (c == 'X') code |= RETRO_DEVICE_ID_JOYPAD_X;
    
    // // Button L
    // c = ss.get();
    // if (c != '.' && c != 'l') reportBadInputString(inputString, c);
    // if (c == 'l') code |= RETRO_DEVICE_ID_JOYPAD_L;

    // // Button R
    // c = ss.get();
    // if (c != '.' && c != 'r') reportBadInputString(inputString, c);
    // if (c == 'r') code |= RETRO_DEVICE_ID_JOYPAD_R;

    // // Button L2
    // c = ss.get();
    // if (c != '.' && c != 'l') reportBadInputString(inputString, c);
    // if (c == 'l') code |= RETRO_DEVICE_ID_JOYPAD_L2;

    // // Button R2
    // c = ss.get();
    // if (c != '.' && c != 'r') reportBadInputString(inputString, c);
    // if (c == 'r') code |= RETRO_DEVICE_ID_JOYPAD_R2;

    // // Button L3
    // c = ss.get();
    // if (c != '.' && c != 'l') reportBadInputString(inputString, c);
    // if (c == 'l') code |= RETRO_DEVICE_ID_JOYPAD_L3;

    // // Button R3
    // c = ss.get();
    // if (c != '.' && c != 'r') reportBadInputString(inputString, c);
    // if (c == 'r') code |= RETRO_DEVICE_ID_JOYPAD_R3;
  }

  static inline void reportBadInputString(const std::string &inputString, const char c)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s' - Read: '%c'\n", inputString.c_str(), c);
  }

  input_t _input;
  controller_t _port1ControllerType;
  controller_t _port2ControllerType;
}; // class InputParser

} // namespace jaffar