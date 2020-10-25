#ifndef TRGBIT_h
#define TRGBIT_h

#include <string>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/TRGSummary.h>

const int N_INPUT_ARRAY = 5;
const int N_OUTPUT_ARRAY = 5;
const int N_PSNM_ARRAY = 10;
const int N_BITS_RESERVED = 200;

namespace Belle2 {

/// A general interface to get a trigger response.
  class TrgBit {

  public:

    /// Constructor
    TrgBit();

    /// Destructor
    virtual ~TrgBit();

  public:

#include <trg/gdl/TrgBitInput.h>
#include <trg/gdl/TrgBitOutput.h>

  public:
    /// Input bit pattern class.
    class InputBitPattern {

    public:
      /// Constructor
      InputBitPattern() : _pattern{}   {}; //TODO is it good default?

      /// Destructor
      virtual ~InputBitPattern() {};

    public:

      /// reads/sets a bit.
      void set(unsigned pattern, unsigned wordPosition);

      /// get fired or not
      bool isFired(unsigned bit) const;

    public:// Operators
      /// Comparison
      int operator == (InputBitPattern&);

      /// Bit
      int operator [](unsigned);

    private:

      /// hit pattern
      unsigned _pattern[6];

    };

    /// Output bit pattern class.
    class OutputBitPattern {

    public:
      /// Constructor
      OutputBitPattern() : _pattern{}  {}; //TODO is it good default?

      /// Destructor
      virtual ~OutputBitPattern() {};

    public:

      /// reads/sets a bit.
      void set(unsigned pattern, unsigned wordPosition);

      /// get fired or not
      bool isFired(unsigned bit) const;

    public:// Operators

      /// Comparison
      int operator == (OutputBitPattern&);

      /// Bit
      int operator [](unsigned);

    private:

      /// hit pattern
      unsigned _pattern[6];

    };

  private:

    unsigned _exp;
    unsigned _run;

    // configuration numbers
    unsigned nconf_psnm;
    unsigned nconf_ftdl;
    unsigned nconf_input;

    // number of using bits
    unsigned n_input;
    unsigned n_output;

    // Whether hit information is filled or not.
    // This is true when TRGSummary class is available.
    bool _isFiredFilled;

  public:// Selectors

    /// returns TrgBit version;
    std::string version(void) const;

    /// returns pre-scale value.
    unsigned preScaleValue(output A) const;
    unsigned preScaleValue(unsigned i) const;

    /// print prescale values
    void printPreScaleValues(void) const;
    /// print configuration values
    void printConf(void) const;

    /// returns true if the psnm bit is fired.
    bool isFired(output bitname) const;
    /// returns true if the input or psnm bit is fired.
    bool isFired(const char* bitname) const;
    /// returns true if the bit is fired.
    bool isFired(input bitname) const;
    bool isFiredInput(unsigned ith_bit) const;
    bool isFiredInput(input bitname) const;
    bool isFiredInput(const char* bitname) const;
    bool isFiredFtdl(unsigned ith_bit) const;
    bool isFiredFtdl(output bitname) const;
    bool isFiredFtdl(const char* bitname) const;
    bool isFiredPsnm(unsigned ith_bit) const;
    bool isFiredPsnm(output bitname) const;
    bool isFiredPsnm(const char* bitname) const;

    /// return number of used bits
    unsigned getNumOfInputs(void) {return n_input;}
    unsigned getNumOfOutputs(void) {return n_output;}

    /// returns bit number
    unsigned getOutputBitNum(const char* bitname) const;
    unsigned getOutputBitNum(output a) {return _outputMap[nconf_ftdl][a];}

    /// returns bit number
    unsigned getInputBitNum(const char* bitname) const;
    unsigned getInputBitNum(input a) {return _inputMap[nconf_input][a];}

    /// returns bit number for the exprun,
    /// if >=0 it is input_bitnum
    /// if  <0 it is -output_bitnum-1
    int getBitNum(const char* bitname) const;

    /// Whether hit information is available or not.
    bool isFiredFilled(void) {return _isFiredFilled;}

    /// Whether the bit exists for the run.
    bool isUsed(input a) const;
    bool isUsed(output a) const;
    bool isUsed(const char* bitname) const;

    /// returns bit name
    const char* getInputBitName(unsigned ith_bit) const;
    const char* getOutputBitName(unsigned ith_bit) const;

    /// returns timing source
    TRGSummary::ETimingType getTimingSource(void) const;

    /// return hit information in vector
    std::vector<unsigned> getInputVector(void) {return _itdVector;}
    std::vector<unsigned> getFtdlVector(void) {return _ftdVector;}
    std::vector<unsigned> getPsnmVector(void) {return _psnVector;}

  public:

    /// returns FTDL version;
    std::string versionFTDL(void) const;

  private:// Modifiers

    // Set configuration version from exp, run number
    // Valid only for physics run, not for cosmic.
    void mapNumber(unsigned exp, unsigned run);

  private:

    static const std::string _ftdlVersion[2];
    static const unsigned _inputMap[N_INPUT_ARRAY][N_BITS_RESERVED];
    static const unsigned _outputMap[N_OUTPUT_ARRAY][N_BITS_RESERVED];
    static const unsigned _psnmValues[N_PSNM_ARRAY][N_BITS_RESERVED];
    static const char* _inputBitNames[N_BITS_RESERVED];
    static const char* _outputBitNames[N_BITS_RESERVED];

    TRGSummary::ETimingType timtype;

    InputBitPattern _input;
    OutputBitPattern _ftdl;
    OutputBitPattern _psnm;

    /// hit information in vector
    std::vector<unsigned> _itdVector;
    std::vector<unsigned> _ftdVector;
    std::vector<unsigned> _psnVector;

  };

}

#endif
