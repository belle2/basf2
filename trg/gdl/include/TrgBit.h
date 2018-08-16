#ifndef TRGBIT_h
#define TRGBIT_h
#define N_INPUT_ARRAY 5
#define N_OUTPUT_ARRAY 5
#define N_PSNM_ARRAY 10

#include <string>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/TRGSummary.h>

namespace Belle2 {

/// A general interface to get a trigger response.
  class TrgBit {

  public:

    /// Constructor
    TrgBit();

    /// Destructor
    virtual ~TrgBit();

  public:

#include <trg/gdl/TrgBitEnum.h>

  public:
    /// Input bit pattern class.
    class InputBitPattern {

    public:
      /// Constructor
      InputBitPattern() {};

      /// Destructor
      virtual ~InputBitPattern() {};

    public:

      /// reads/sets a bit.
      void set(unsigned pattern, unsigned wordPosition);

      /// get fired or not
      bool get(unsigned bit) const;

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
      OutputBitPattern() {};

      /// Destructor
      virtual ~OutputBitPattern() {};

    public:

      /// reads/sets a bit.
      void set(unsigned pattern, unsigned wordPosition);

      /// get fired or not
      bool get(unsigned bit) const;

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

  public:// Selectors

    /// returns TrgBit version;
    std::string version(void) const;

    /// returns pre-scale value.
    unsigned preScaleValue(output A) const;
    unsigned preScaleValue(unsigned i) const;

    /// print
    void printPreScaleValues(void) const;

    /// returns true if the bit is fired.
    bool get(input bitname) const;
    bool getInput(input bitname) const;
    bool get(output bitname) const;
    bool getOutput(output bitname) const;
    bool getInput(unsigned ith_bit) const;
    bool getOutput(unsigned ith_bit) const;
    bool getPSNM(unsigned ith_bit) const;
    bool getPSNM(output bitname) const;

    // returns bit name
    const char* getInputBitName(unsigned ith_bit) const;
    const char* getOutputBitName(unsigned ith_bit) const;

    // returns timing source
    TRGSummary::ETimingType getTimingSource(void) const;

  public:

    /// returns FTDL version;
    std::string versionFTDL(void) const;

  private:// Modifiers

    // Set configuration version from exp, run number
    // Valid only for physics run, not for cosmic.
    void mapNumber(unsigned exp, unsigned run);

  private:

    static const std::string _ftdlVersion[2];
    static const unsigned _inputMap[N_INPUT_ARRAY][192];
    static const unsigned _outputMap[N_OUTPUT_ARRAY][192];
    static const unsigned _psnmValues[N_PSNM_ARRAY][192];
    static const char* _inputBitNames[192];
    static const char* _outputBitNames[192];

    TRGSummary::ETimingType timtype;

    InputBitPattern _input;
    OutputBitPattern _ftdl;
    OutputBitPattern _psnm;

  };

}

#endif
