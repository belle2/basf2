#ifndef __EXTERNAL__
#include "trg/cdc/FpgaUtility.h"
#else
#include "FpgaUtility.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <cmath>

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::bitset;
using std::ifstream;
using std::ofstream;
using std::ceil;
using std::floor;
using std::fmod;
using std::pow;
using std::log;


// Rounds value to an integer.
double FpgaUtility::roundInt(double value)
{
  if (value > 0) {
    value -= 0.5;
    // If value is very near int, then let value be the int
    if (value - floor(value) < 0.00001) value = floor(value);
    if (floor(value + 1) - value < 0.00001) value = floor(value + 1);
    // Round up
    value = ceil(value);
  } else {
    value += 0.5;
    // If value is very near int, then let value be the int
    if (-value + ceil(value) < 0.00001) value = ceil(value);
    if (-ceil(value - 1) + value < 0.00001) value = ceil(value - 1);
    // Round down
    value = floor(value);
  }
  return value;
}

// Radix changing functions
// Change string to octal. Possible radix=[2,26].
double FpgaUtility::arbToDouble(std::string in, int inRadix)
{
  if (inRadix < 2 || inRadix > 26)  {
    cout << "[Error] arbToDouble() => Radix is out of range [2,26]. Aborting." << endl;
    return 0;
  }
  double result = 0;
  for (unsigned int iChar = 0; iChar < in.size(); iChar++) {
    if (in[iChar] == '-') continue;
    result = result * inRadix;
    if (in[iChar] >= '0' && in[iChar] <= '9') result += in[iChar] - 48;
    else if (in[iChar] >= 'A' && in[iChar] <= 'Z') result += in[iChar] - 55;
    else if (in[iChar] >= 'a' && in[iChar] <= 'z') result += in[iChar] - 87;
    else {
      cout << "[Error] arbToDouble() => Strange character detected. Aborting." << endl;
      return 0;
    }
  } // end of string loop.
  if (in[0] == '-') result *= -1;
  return result;
}
// Change octal to string.
string FpgaUtility::doubleToArb(double in, int outRadix, int numberOfDigits)
{
  string result;
  int sign = 1;
  if (in < 0) {
    sign = -1;
    in *= -1;
  }
  while (1) {
    int rem = fmod(in, outRadix);
    result.insert(result.begin(), char((rem >= 0 && rem <= 9) ? rem + 48 : rem + 55));
    in = (in / outRadix) - (rem / outRadix);
    if (in < outRadix) {
      result.insert(result.begin(), char((in >= 0 && in <= 9) ? in + 48 : in + 55));
      break;
    }
  }
  // Padding for output.
  if (int(result.size()) < numberOfDigits) {
    int nBitsAdd = numberOfDigits - result.size();
    for (int iZero = 0; iZero < nBitsAdd; iZero++) {
      result.insert(0, "0");
    }
  } else if (numberOfDigits == -1) {
  } else if (int(result.size()) > numberOfDigits) {
    cout << "[Error] doubleToArb() => numberOfDigits too small. Aborting." << endl;
    return 0;
  }
  if (sign == -1) result.insert(0, "-");
  return result;
}
// Change number system between strings.
string FpgaUtility::arbToArb(const std::string& in, int inRadix, int outRadix, int numberOfDigits)
{
  return doubleToArb(arbToDouble(in, inRadix), outRadix, numberOfDigits);
}

// Changes signed binary string to 2s complement binary string.
string FpgaUtility::signedToTwosComplement(string in, int numberOfDigits)
{
  int sign = 1;
  if (in[0] == '-') {
    in.erase(0, 1);
    sign = -1;
  }
  // Padding for output.
  if (int(in.size()) < numberOfDigits) {
    unsigned nBits = numberOfDigits - in.size();
    for (unsigned iZero = 0; iZero < nBits; iZero++) {
      in.insert(0, "0");
    }
  } else if (int(in.size()) >= numberOfDigits) {
    cout << "[Error] signedToTwosComplement() => numberOfDigits too small. Aborting." << endl;
    return "";
  }
  if (sign == 1) {
    // For positive values.
  } else {
    // For negative values.
    // Flip values.
    for (unsigned int iChar = 0; iChar < in.size(); iChar++) {
      if (in[iChar] == '1') in[iChar] = '0';
      else if (in[iChar] == '0') in[iChar] = '1';
      else cout << "[Error] signedToTwosComplement() => Strange character in string. Aborting." << endl;
    }
    //cout<<"Flip: "<<in<<endl;
    // Add 1 to result.
    for (int iDigit = in.size() - 1; iDigit >= 0; iDigit--) {
      if (in[unsigned(iDigit)] == '0') {
        in[unsigned(iDigit)] = '1';
        break;
      } else {
        in[unsigned(iDigit)] = '0';
      }
    } // End of adding 1.
    //cout<<"Adding: "<<in<<endl;
  }
  return in;
}

// Changes 2s complement binary string to signed binary string.
std::string FpgaUtility::twosComplementToSigned(std::string in)
{
  // If value is positive.
  if (in[0] == '0') {
  } else {
    // Subtract 1.
    for (int iDigit = in.size() - 1; iDigit >= 0; iDigit--) {
      if (in[unsigned(iDigit)] == '1') {
        in[unsigned(iDigit)] = '0';
        break;
      } else {
        in[unsigned(iDigit)] = '1';
      }
    } // End of subtract 1.
    //cout<<"Subtract 1: "<<in<<endl;
    // Flip values.
    for (unsigned int iChar = 0; iChar < in.size(); iChar++) {
      if (in[iChar] == '1') in[iChar] = '0';
      else if (in[iChar] == '0') in[iChar] = '1';
      else cout << "[Error] twosComplementToSigned() => Strange character in string. Aborting." << endl;
    }
    //cout<<"Flip: "<<in<<endl;;
    // Remove padding '0's
    for (int iDigit = 0; iDigit < int(in.size()); iDigit++) {
      if (in[unsigned(iDigit)] == '0') {
        if (in.size() == 1) break;
        in.erase(iDigit, 1);
        iDigit--;
      } else {
        break;
      }
    }
    // Add minus sign.
    in.insert(0, "-");
  }
  return in;
}

// Write values to a file.
void FpgaUtility::writeSignals(std::string outFilePath, std::map<std::string, std::vector<signed long long> >& data)
{
  // Create file.
  ofstream outFile;
  outFile.open(outFilePath.c_str());
  // Fill content of file.
  for (auto it = data.begin(); it != data.end(); it++) {
    outFile << (*it).first << " ";
    for (unsigned iVector = 0; iVector < (*it).second.size(); iVector++) {
      outFile << (*it).second[iVector] << " ";
    }
    outFile << endl;
  }
  // Close file.
  outFile.close();
}

// COE file functions. [TODO] Should  limit number of entries for write functions.
void FpgaUtility::multipleWriteCoe(int lutInBitsize, std::map<std::string, std::vector<signed long long> >& data,
                                   const std::string& fileDirectory)
{
  // Loop over all data.
  for (auto it = data.begin(); it != data.end(); it++) {
    FpgaUtility::writeCoe(fileDirectory + "/" + (*it).first + ".coe", lutInBitsize, (*it).second);
  }
}

void FpgaUtility::writeCoe(std::string outFilePath, int lutInBitsize, std::vector<signed long long>& data)
{

  // Create file.
  ofstream coeFile;
  coeFile.open(outFilePath.c_str());
  // Meta data for file.
  coeFile << "memory_initialization_radix=10;" << endl;
  coeFile << "memory_initialization_vector=" << endl;
  unsigned nEntries = pow(2, lutInBitsize);
  // Fill content of file.
  unsigned nDataSize = data.size();
  for (unsigned index = 0; index < (nEntries - 1); index++) {
    if (index < nDataSize) coeFile << data[index] << "," << endl;
    else coeFile << 0 << "," << endl;
  }
  if (nEntries <= nDataSize) coeFile << data[nEntries - 1] << ";" << endl;
  else coeFile << 0 << ";" << endl;
  // Close file.
  coeFile.close();
}

void FpgaUtility::readCoe(std::string inFilePath, std::vector<signed long long>& data, bool twosComplement)
{

  ifstream coeFile;
  coeFile.open(inFilePath.c_str());
  if (coeFile.fail()) {
    cout << "[Error] FpgaUtility::readCoe() => Can not open file: " << inFilePath << endl;
    return;
  } else {
    // Read file
    string t_line;
    stringstream reformatCoeFile;
    // Reformat file.
    while (getline(coeFile, t_line)) {
      // Remove all spaces
      t_line.erase(remove_if(t_line.begin(), t_line.end(), ::isspace), t_line.end());
      // Remove blank lines
      if (t_line.size() == 0) continue;
      // Remove all characters after ';'
      size_t t_iFind = t_line.find(";");
      if (t_iFind != string::npos) {
        t_line.erase(t_iFind + 1, string::npos);
      }
      // Remove lines for comments
      if (t_line == ";") continue;
      // Combine lines until ';'
      if (t_line.find(";") == string::npos) reformatCoeFile << t_line;
      // Replace ';' with endl
      else reformatCoeFile << t_line.substr(0, t_line.size() - 1) << endl;
    }
    // Process reformatted file.
    //vector<string> keywords = {"memory_initialization_radix", "memory_initialization_vector"};
    int t_radix = 0;
    vector<string> t_rawData;
    while (getline(reformatCoeFile, t_line)) {
      //cout<<t_line<<endl;
      // Find keywords
      size_t t_iFind = t_line.find("=");
      if (t_iFind != string::npos) {
        // Find values for keywords
        // Find radix
        if (t_line.substr(0, t_iFind) == "memory_initialization_radix") {
          //t_radix = t_line.substr(t_iFind+1, string::npos);
          t_radix = atoi(t_line.substr(t_iFind + 1, string::npos).c_str());
        } else if (t_line.substr(0, t_iFind) == "memory_initialization_vector") {
          // Find data values
          string t_dataLine = t_line.substr(t_iFind + 1, string::npos);
          while (1) {
            t_iFind = t_dataLine.find(",");
            if (t_iFind != string::npos) {
              t_rawData.push_back(t_dataLine.substr(0, t_iFind));
              t_dataLine.erase(0, t_iFind + 1);
            } else {
              // last entry
              t_rawData.push_back(t_dataLine);
              break;
            }
          } // Finding data values loop
        } else {
          cout << "[Error] FpgaUtility::readCoe() => .coe format keyword is wrong. Aborting" << endl;
          break;
        } // End of finding values for keywords
      } else {
        cout << "[Error] FpgaUtility::readCoe() => .coe format is wrong. Needs keywords. Aborting" << endl;
        break;
      } // End of finding keywords
    }
    //// Print rawData
    //for(auto it=t_rawData.begin(); it!=t_rawData.end(); it++){
    //  cout<<(*it)<<endl;
    //}

    // Clear data and set data size.
    data.clear();
    data.resize(t_rawData.size());

    if (!twosComplement) {
      for (int iData = 0; iData < int(t_rawData.size()); iData++) {
        // Change string to float octal.
        data[iData] = FpgaUtility::arbToDouble(t_rawData[iData], t_radix);
      }
    } else {
      // Find number of bits in binary for data.
      int nBits = t_rawData[0].size();
      // Calculate max value.
      int t_max = pow(t_radix, nBits) - 1;
      // Calculate number of bits
      nBits = floor(log(t_max) / log(2)) + 1;
      for (int iData = 0; iData < int(t_rawData.size()); iData++) {
        // Change to binary.
        string t_binary = FpgaUtility::arbToArb(t_rawData[iData], t_radix, 2, nBits);
        // Change to signed.
        string t_signedBinary = FpgaUtility::twosComplementToSigned(t_binary);
        // Change to float octal.
        data[iData] = FpgaUtility::arbToDouble(t_signedBinary, 2);
      }
    } // End filling data.
    //// Print all values of LUT
    //cout<<"LUT("<<inFilePath<<") data"<<endl;
    //for(int iData=0; iData<int(data.size()); iData++){
    //  cout<<"["<<iData<<"] "<<data[iData]<<endl;
    //}
  }
}
