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
#include "trg/gdl/TrgBitEnum.h"

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
      unsigned _pattern[6];
    };

  private:

    unsigned _exp;
    unsigned _run;
    unsigned n_psnm;
    unsigned n_ftdl;
    unsigned n_input;

  public:// Selectors
    /// returns TrgBit version;
    std::string version(void) const;

    /// returns pre-scale value.
    unsigned preScaleValue(output A) const;

    /// returns true if A is on.
    bool get(input A) const;
    bool get(output A) const;
    bool getPSNM(output A) const;

    TRGSummary::ETimingType getTimingSource(void) const;

  public:// Obsolete functions
    /// returns FTDL version;
    std::string versionFTDL(void) const;

  private:// Modifiers

    void mapNumber(unsigned exp, unsigned run);

  private: // Masks

    static const std::string _ftdlVersion[2];
    static const unsigned _inputMap[N_INPUT_ARRAY][192];
    static const unsigned _outputMap[N_OUTPUT_ARRAY][192];
    static const unsigned _psnmValues[N_PSNM_ARRAY][192];

    TRGSummary::ETimingType timtype;
    InputBitPattern _input;
    OutputBitPattern _ftdl;
    OutputBitPattern _psnm;
  };

}

#endif
