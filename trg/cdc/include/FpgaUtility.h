#ifndef FPGAUTILITY_H
#define FPGAUTILITY_H

#include <map>
#include <string>
#include <vector>

class FpgaUtility{

  public:

    // Round double value.
    static double roundInt(double value);
    // Radix changing functions.
    // Change string to octal. Possible radix=[2,26].
    static double arbToDouble(std::string in, int inRadix);
    // Change octal to string.
    static std::string doubleToArb(double in, int outRadix, int numberOfDigits=-1);
    static std::string arbToArb(std::string in, int inRadix, int outRadix, int numberOfDigits=-1);
    static std::string signedToTwosComplement(std::string in, int numberOfDigits);
    static std::string twosComplementToSigned(std::string in);
    // COE file functions.
    static void writeSignals(std::string outFilePath, std::map<std::string, std::vector<signed long long> > & data);
    static void multipleWriteCoe(int lutInBitsize, std::map<std::string, std::vector<signed long long> > & data, std::string fileDirectory);
    static void writeCoe(std::string outFilePath, int lutInBitsize, std::vector<signed long long> & data);
    static void readCoe(std::string inFilePath, std::vector<signed long long> & data, bool twoscomplement=0);

};

#endif
