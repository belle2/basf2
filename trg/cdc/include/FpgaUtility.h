#ifndef FPGAUTILITY_H
#define FPGAUTILITY_H

#include <map>
#include <string>
#include <vector>

/// A class that holds FPGA related functions.
class FpgaUtility {

public:

  /// Round double value.
  static double roundInt(double value);
  /// Radix changing functions.
  /// Change string to octal. Possible radix=[2,26].
  static double arbToDouble(std::string in, int inRadix);
  /// Change octal to string.
  static std::string doubleToArb(double in, int outRadix, int numberOfDigits = -1);
  /// Change string number to another string number depending on radix.
  static std::string arbToArb(const std::string& in, int inRadix, int outRadix, int numberOfDigits = -1);
  /// Changes string signed binary(-sign) to two complements.
  static std::string signedToTwosComplement(std::string in, int numberOfDigits);
  /// Changes string two complements to string signed binary(-sign).
  static std::string twosComplementToSigned(std::string in);
  /// COE file functions.
  /// Writes values to a file.
  static void writeSignals(std::string outFilePath, std::map<std::string, std::vector<signed long long> >& data);
  /// Writes multiple signal values to a file in coe format.
  static void multipleWriteCoe(int lutInBitsize, std::map<std::string, std::vector<signed long long> >& data,
                               const std::string& fileDirectory);
  /// Writes a signal's values to a file in coe format.
  static void writeCoe(std::string outFilePath, int lutInBitsize, std::vector<signed long long>& data);
  /// Reads a coe format file and stores the values in vector.
  static void readCoe(std::string inFilePath, std::vector<signed long long>& data, bool twoscomplement = 0);

};

#endif
