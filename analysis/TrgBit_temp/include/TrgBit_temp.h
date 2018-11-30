#ifndef TRGBIT_h
#define TRGBIT_h

#include <stdlib.h>
#include <string>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/TRGSummary.h>



namespace Belle2 {

  //The number input array
  const int N_INPUT_ARRAY = 5;
  //The number output array
  const int N_OUTPUT_ARRAY = 5;
  //The number PSNM output array
  const int N_PSNM_ARRAY = 10;
  //The number bit
  const int N_BITS_RESERVED = 200;


  /// A general interface to get a trigger response.
  class TrgBit_temp {

  public:

    /// Constructor
    TrgBit_temp();

    /// Destructor
    virtual ~TrgBit_temp();

  public:

    //number of input bit
    enum input {
      t3_0,
      t3_1,
      t3_2,
      t2_0,
      t2_1,
      t2_2,
      cdc_open90,
      cdc_active,
      cdc_b2b3,
      cdc_b2b5,
      cdc_b2b7, // 10
      cdc_b2b9,
      ehigh,
      elow,
      elum,
      ecl_bha,
      bha_type_0,
      bha_type_1,
      bha_type_2,
      bha_type_3,
      bha_type_4, // 20
      bha_type_5,
      bha_type_6,
      bha_type_7,
      bha_type_8,
      bha_type_9,
      bha_type_10,
      bha_type_11,
      bha_type_12,
      bha_type_13,
      clst_0,     // 30
      clst_1,
      clst_2,
      clst_3,
      ecl_bg_0,
      ecl_bg_1,
      ecl_bg_2,
      ecl_active,
      ecl_timing_fwd,
      ecl_timing_brl,
      ecl_timing_bwd, // 40
      ecl_phys,
      top_0,
      top_1,
      top_2,
      top_bb,
      top_active,
      klm_hit,
      klm_0,
      klm_1,
      klm_2,    // 50
      klm_3,
      klmb2b_0,
      klmb2b_1,
      klmb2b_2,
      revo,
      her_kick,
      ler_kick,
      bha_delay,
      pseud_rand,
      plsin,        // 60
      poissonin,
      periodin,
      veto,
      nclst1_0,
      nclst1_1,
      nclst1_2,
      nclst2_0,
      nclst2_1,
      nclst2_2,
      cdcecl_0, // 70
      cdcecl_1,
      cdcecl_2,
      cdcklm_0,
      cdcklm_1,
      cdcklm_2,
      nimin0,
      nimin1,
      nimin2,
      nimin3,
      t3_3,        // 80
      t2_3,
      nclst1_3,
      nclst2_3,
      cdcecl_3,
      cdcklm_3,
      u2_0,
      u2_1,
      u2_2,
      ecl_oflo,
      ecl_3dbha, // 90
      lml_0,
      lml_1,
      lml_2,
      lml_3,
      lml_4,
      lml_5,
      lml_6,
      lml_7,
      lml_8,
      lml_9,  // 100
      lml_10,
      samhem,
      opohem,
      d_b2b3,
      d_b2b5,
      d_b2b7,
      d_b2b9,
      p_b2b3,
      p_b2b5,
      p_b2b7, // 110
      p_b2b9
    };

    //numer of output bit
    enum output {
      zzz,
      zzzo,
      fff,
      fffo,
      zz,
      zzo,
      ffb,
      ffo,
      hie,
      lowe,
      lume,   // 10
      c2,
      c3,
      c4,
      c5,
      bhabha,
      bhabha_trk,
      bhabha_brl,
      bhabha_ecp,
      g_high,
      g_c1,   // 20
      gg,
      mu_pair,
      mu_b2b,
      revolution,
      random,
      bg,
      ecltiming,
      nim0,
      nima03,
      nimo03, // 30
      period,
      eclnima03,
      eclnimo03,
      pls,
      poisson,
      klmhit,
      f,
      fe,
      ffe,
      fc,     // 40
      ffc,
      cdctiming,
      cdcbb,
      nim1c,
      c1n0,
      c1n1,
      c1n2,
      c1n3,
      c1n4,
      c2n1,   // 50
      c2n2,
      c2n3,
      c2n4,
      cdcecl1,
      cdcecl2,
      cdcecl3,
      cdcecl4,
      cdcklm1,
      cdcklm2,
      cdcklm3, // 60
      cdcklm4,
      ff,
      uuu,
      uuuo,
      uub,
      uuo,
      c1hie,
      c1lume,
      n1hie,
      n1lume, // 70
      c3hie,
      c3lume,
      n3hie,
      n3lume,
      eed,
      fed,
      fp,
      bha3d,
      shem,
      ohem,   // 80
      lml0,
      lml1,
      lml2,
      lml3,
      lml4,
      lml5,
      lml6,
      lml7,
      lml8,
      lml9,
      lml10,
      oup92,
      oup93,
      oup94,
      oup95
    };


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
      OutputBitPattern() {};

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

    // exp number
    unsigned _exp;

    // run number
    unsigned _run;

    // configuration numbers of PSNM
    unsigned nconf_psnm;

    // configuration numbers of FTDL
    unsigned nconf_ftdl;

    // configuration numbers of input
    unsigned nconf_input;

    // number of using input bits
    unsigned n_input;

    // number of using output bits
    unsigned n_output;

    // Whether hit information is filled or not.
    // This is true when TRGSummary class is available.
    bool _isFiredFilled;

  public:// Selectors

    /// returns TrgBit_temp version;
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

    /// returns true if the input bit is fired.
    bool isFiredInput(unsigned ith_bit) const;
    bool isFiredInput(input bitname) const;
    bool isFiredInput(const char* bitname) const;

    /// returns true if the FTDL bit is fired.
    bool isFiredFtdl(unsigned ith_bit) const;
    bool isFiredFtdl(output bitname) const;
    bool isFiredFtdl(const char* bitname) const;

    /// returns true if the PSNM bit is fired.
    bool isFiredPsnm(unsigned ith_bit) const;
    bool isFiredPsnm(output bitname) const;
    bool isFiredPsnm(const char* bitname) const;

    /// return number of used input bits
    unsigned getNumOfInputs(void) {return n_input;}

    /// return number of used output bits
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

    /// returns input bit name
    const char* getInputBitName(unsigned ith_bit) const;

    /// returns output bit name
    const char* getOutputBitName(unsigned ith_bit) const;

    /// returns timing source
    TRGSummary::ETimingType getTimingSource(void) const;

    /// return input hit information in vector
    std::vector<unsigned> getInputVector(void) {return _itdVector;}

    /// return FTDL hit information in vector
    std::vector<unsigned> getFtdlVector(void) {return _ftdVector;}

    /// return PSNM hit information in vector
    std::vector<unsigned> getPsnmVector(void) {return _psnVector;}

  public:

    /// returns FTDL version;
    std::string versionFTDL(void) const;

  private:// Modifiers

    // Set configuration version from exp, run number
    // Valid only for physics run, not for cosmic.
    void mapNumber(unsigned exp, unsigned run);

  private:

    //version of FTDL
    static const std::string _ftdlVersion[2];

    //bit number of input
    static const unsigned _inputMap[N_INPUT_ARRAY][N_BITS_RESERVED];

    //bit number of output
    static const unsigned _outputMap[N_OUTPUT_ARRAY][N_BITS_RESERVED];

    /// Prescale in each bit
    static const unsigned _psnmValues[N_PSNM_ARRAY][N_BITS_RESERVED];

    /// input bit name
    static const char* _inputBitNames[N_BITS_RESERVED];

    /// output bit name
    static const char* _outputBitNames[N_BITS_RESERVED];

    /// type of timing determined by TMDL
    TRGSummary::ETimingType timtype;

    /// input hit information in Bit pattern
    InputBitPattern _input;

    /// FTDL hit information in Bit pattern
    OutputBitPattern _ftdl;

    /// PSNM hit information in Bit pattern
    OutputBitPattern _psnm;

    /// input hit information in vector
    std::vector<unsigned> _itdVector;

    /// FTDL information in vector
    std::vector<unsigned> _ftdVector;

    /// PSNM information in vector
    std::vector<unsigned> _psnVector;

  };

}

#endif
