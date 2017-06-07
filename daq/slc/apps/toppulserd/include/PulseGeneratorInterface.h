#ifndef _PULSEGENERATORINTERFACE_H
#define _PULSEGENERATORINTERFACE_H
#include "daq/slc/apps/toppulserd/vxi11_user.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <sstream>


#define BUF_LEN 10000

/**
 *Convert integer to string
 */
inline std::string IntToString(const int value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/**
 *Convert float to string
 */
inline std::string FloatToString(const float value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/**
 *convert string from pulser GPIB interface to float value
 */
inline float StringToFloat(const std::string& s)
{
  std::istringstream i(s);
  float x;
  if (!(i >> x)) return 0;
  else return x;
}

/**
 *@brief Pulse Generator Interface Class
 *This class uses predefined command strings to set/get parameters from
 *the Tektronix AFG3251 pulse generator.
 *@author Tobias Weber
 */
class PulseGeneratorInterface {

public:

  /**
   *@brief standard constructor
   */
  PulseGeneratorInterface()
  {
    pulserLink = NULL;
    ipAddress = "";
    pulserInterfaceStatus = -1;
    debugMode = false;
    std::string initList[4] = {"SIN", "SQU", "PULS", "GAUS"};
    for (int i = 0; i < 4; ++i) validPulseShapes.push_back(initList[i]);

    outputChannelQuery = "OUTP";
    outputVoltageQuery = "SOUR:VOLT:AMPL";
    outputFrequencyQuery = "SOUR:FREQ";
    outputDutyCycleQuery = "SOUR:PULS:DCYC";
    outputPulseShapeQuery = "SOUR:FUNC:SHAP";
  }

  /**
   *@brief destructor takes care to close interface
   */
  ~PulseGeneratorInterface()
  {
    if (pulserLink) CloseConnection();
  }

  /**
   *@brief enable debugging mode
   *command string is written to stdout and not the pulser interface
   */
  void EnableDebugMode(bool enable = true)
  {
    debugMode = enable;
  }

  /**
   *@brief Open connection to pulse generator GPIB interface
   *@param ip address of pulse generator
   */
  int OpenConnection(const std::string& fipAddress)
  {
    pulserLink = new CLINK;
    ipAddress = fipAddress;
    pulserInterfaceStatus = vxi11_open_device(ipAddress.c_str(), pulserLink);
    return pulserInterfaceStatus;
  }

  /**
   *@brief close connection
   */
  int CloseConnection()
  {
    if (pulserLink) {
      pulserInterfaceStatus = vxi11_close_device(ipAddress.c_str(), pulserLink);
      delete pulserLink;
      pulserLink = NULL;
    }
    return pulserInterfaceStatus;
  }

  int GetPulserInterfaceStatus()
  {
    return pulserInterfaceStatus;
  }

  bool GetChannelIsOn()
  {
    std::string query;
    query = outputChannelQuery + "?";
    if (debugMode) {
      std::cout << query << std::endl;
      return true;
    }
    if (pulserLink != NULL) {
      int status;
      std::string response = ReadPulserParameter(query, pulserLink, status);
      if (status > 0) {
        int val = atoi(response.c_str());
        return val;
      } else return 0;
    }
    return 0;
  }

  float GetFrequency()
  {
    std::string query;
    query = outputFrequencyQuery + "?";
    if (debugMode) {
      std::cout << query << std::endl;
      return 0;
    }
    if (pulserLink != NULL) {
      int status;
      std::string response = ReadPulserParameter(query, pulserLink, status);
      if (status > 0) {
        float frequency = StringToFloat(response);
        return frequency;
      } else return -1;
    }
    return -1;
  }

  float GetDutyCycle()
  {
    std::string query;
    query = outputDutyCycleQuery + "?";
    if (debugMode) {
      std::cout << query << std::endl;
      return 0;
    }
    if (pulserLink != NULL) {
      int status;
      std::string response = ReadPulserParameter(query, pulserLink, status);
      if (status > 0) {
        float dutycycle = StringToFloat(response);
        return dutycycle;
      } else return -1;
    }
    return -1;
  }

  float GetAmplitude()
  {
    std::string query;
    query = outputVoltageQuery + "?";
    if (debugMode) {
      std::cout << query << std::endl;
      return 0;
    }
    if (pulserLink != NULL) {
      int status;
      std::string response = ReadPulserParameter(query, pulserLink, status);
      if (status > 0) {
        float amplitude = StringToFloat(response);
        return amplitude;
      } else return -1;
    }
    return -1;
  }

  std::string GetPulseShape()
  {
    std::string query;
    query = outputPulseShapeQuery + "?";
    if (debugMode) {
      std::cout << query << std::endl;
      return "";
    }
    if (pulserLink != NULL) {
      int status;
      std::string response = ReadPulserParameter(query, pulserLink, status);
      if (status > 0) {
        return response;
      } else return "Unknown";
    }
    return "Unknown";
  }

  void SetChannelOutput(bool enable)
  {
    std::string query;
    if (enable) query = outputChannelQuery + " 1";
    else query = outputChannelQuery + " 0";
    if (debugMode) {
      std::cout << query << std::endl;
      return ;
    }
    if (pulserLink != NULL) {
      pulserInterfaceStatus = SendPulserParameter(query, pulserLink);
    }
  }

  void SetFrequency(float frequency)
  {
    std::string query;
    query = outputFrequencyQuery + " " + FloatToString(frequency) + "Hz";
    if (debugMode) {
      std::cout << query << std::endl;
      return ;
    }
    if (pulserLink != NULL) {
      pulserInterfaceStatus = SendPulserParameter(query, pulserLink);
    }
  }

  void SetDutyCycle(float dutycycle)
  {
    std::string query;
    if (dutycycle > 0. && dutycycle < 100.) {
      query = outputDutyCycleQuery + " " + FloatToString(dutycycle);
    } else {
      query = outputDutyCycleQuery + " " + FloatToString(50.);
    }
    if (debugMode) {
      std::cout << query << std::endl;
      return ;
    }
    if (pulserLink != NULL) {
      pulserInterfaceStatus = SendPulserParameter(query, pulserLink);
    }
  }

  void SetAmplitude(float amplitude)
  {
    std::string query;
    if (amplitude > 5) amplitude = 5;
    query = outputVoltageQuery + " " + FloatToString(amplitude) + "VPP";
    if (debugMode) {
      std::cout << query << std::endl;
      return ;
    }
    if (pulserLink != NULL) {
      pulserInterfaceStatus = SendPulserParameter(query, pulserLink);
    }
  }

  void SetPulseShape(std::string shape)
  {
    std::string query;
    query = outputPulseShapeQuery + " " + shape;
    if (debugMode) {
      std::cout << query << std::endl;
      return ;
    }
    std::list<std::string>::iterator it = std::find(validPulseShapes.begin(), validPulseShapes.end(),
                                                    shape);
    if (it == validPulseShapes.end()) {
      return;
    }
    if (pulserLink != NULL) {
      pulserInterfaceStatus = SendPulserParameter(query, pulserLink);
    }
  }

private:

  /**
   *@brief send a command to the pulse generator to change an internal parameter
   *@param query string, not tested for correctness
   *@pram pulse generator link interface
   */
  int SendPulserParameter(const std::string& query, CLINK* const clink)
  {
    int status = vxi11_send(clink, query.c_str());
    return status;
  }

  /**
   *@brief read the value of parameter from the pulser
   *@param query string, not tested for correctness
   *@param pulse generator link interface
   *@param status response from interface transaction
   */
  std::string ReadPulserParameter(const std::string& query, CLINK* const clink, int& status)
  {
    char buf[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    std::string queryResponse = "";

    vxi11_send(clink, query.c_str());
    status = vxi11_receive(clink, buf, BUF_LEN);
    if (status > 0) {
      buf[strlen(buf) - 1] = 0;
      queryResponse = buf;
    }
    return queryResponse;
  }

  /**
   *query strings for tektronix pulser
   */
  std::string outputChannelQuery;
  std::string outputVoltageQuery;
  std::string outputFrequencyQuery;
  std::string outputDutyCycleQuery;
  std::string outputPulseShapeQuery;
  std::list<std::string> validPulseShapes;

  int pulserInterfaceStatus;
  std::string ipAddress;
  CLINK* pulserLink;
  bool debugMode;


};

#endif
