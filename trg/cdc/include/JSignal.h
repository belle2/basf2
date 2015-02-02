//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : JSignal.h
// Section  : TRG CDC
// Owner    : Jaebak KIM (K.U.)
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class for Signals in CDC Trigger for 3D tracker.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCJSignal_FLAG_
#define TRGCDCJSignal_FLAG_

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <tuple>

#ifdef TRGCDC_SHORT_NAMES
#define JSignal TRGCDCJSignal
#endif

namespace Belle2 {

class TRGCDCJSignalData;

/// A class to use Signals for TRGCDC 3D tracker
class TRGCDCJSignal {

  public:

  /// Constructor.
  TRGCDCJSignal();
  TRGCDCJSignal(TRGCDCJSignalData *);
  // Actual to integer constructor.
  TRGCDCJSignal(int const & bitwidth, double const & value, double const & min, double const & max, int const & clock, TRGCDCJSignalData *);
  TRGCDCJSignal(int const & bitwidth, double const & value, double const & min, double const & max, TRGCDCJSignalData *);
  // Integer constructor.
  TRGCDCJSignal(signed long long const & intValue, double const & toReal, signed long long const & minInt, signed long long const & maxInt, double const & actual, double const & minActual, double const & maxActual, int const & finishClock, TRGCDCJSignalData *, bool b_slv=0);
  // Constant constructor.
  TRGCDCJSignal(double const & value, double const & toReal, TRGCDCJSignalData *);
  // Slv constructor
  TRGCDCJSignal(std::vector<bool> const & slvValue, int const& finishClock, TRGCDCJSignalData* );

  /// Destructor.
  ~TRGCDCJSignal(){}

  /// Operators.
  //TRGCDCJSignal& operator= (TRGCDCJSignal rhs);
  TRGCDCJSignal& assignTo (TRGCDCJSignal const rhs, int targetClock, std::string & finalCode);
  TRGCDCJSignal& assignTo (TRGCDCJSignal const rhs, int targetClock);
  TRGCDCJSignal& operator<= (TRGCDCJSignal const rhs);
  // Unary operator.
  TRGCDCJSignal const operator- () const;
  // Arithmetic operators.
  TRGCDCJSignal const operator+ (TRGCDCJSignal const& rhs) const;
  TRGCDCJSignal const operator- (TRGCDCJSignal const& rhs) const;
  TRGCDCJSignal const operator* (TRGCDCJSignal const& rhs) const;

  /// Methods.
  void dump() const;
  /// Shift signal. Shift direction is right. operate=0 to change m_toReal(unit).
  TRGCDCJSignal const shift(int nBits, int operate=1) const;
  /// Outputs an offset signal which is an unsigned signal.
  TRGCDCJSignal const offset(TRGCDCJSignal const valueMin) const;
  /// Outputs a signal that is not offset.
  TRGCDCJSignal const invOffset(TRGCDCJSignal const valueMin) const;
  /// Case. Reference is what is used to determine. Target is what is going to be assigned. Data is (Assignment, From, To)... (Assignment). Last is default.
  static void choose(TRGCDCJSignal & target, TRGCDCJSignal const & targetMin, TRGCDCJSignal const & targetMax, TRGCDCJSignal const & reference, std::vector<std::vector<TRGCDCJSignal> > data, int targetClock);
  static void choose(TRGCDCJSignal & target, TRGCDCJSignal const & targetMin, TRGCDCJSignal const & targetMax, TRGCDCJSignal const & reference, std::vector<std::vector<TRGCDCJSignal> > data);
  static void choose(TRGCDCJSignal & target, TRGCDCJSignal const & reference, std::vector<std::vector<TRGCDCJSignal> > data, int targetClock);
  static void choose(TRGCDCJSignal & target, TRGCDCJSignal const & reference, std::vector<std::vector<TRGCDCJSignal> > data);
  /// If else implementation
  static void ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal *, TRGCDCJSignal> > > > & data, int targetClock);
  static void ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal *, TRGCDCJSignal> > > > & data);
  /// Limits min and max values.
  TRGCDCJSignal const limit(signed long long minInt, signed long long maxInt, double minActual, double maxActual) const;
  TRGCDCJSignal const limit(TRGCDCJSignal const & min, TRGCDCJSignal const & max) const;
  /// Resizes signal.
  TRGCDCJSignal const resize(int bitwidth) const;
  ///// Change to signed signal.
  //TRGCDCJSignal const toSigned() const;
  ///// Change to unsigned signal.
  //TRGCDCJSignal const toUnsigned() const;
  static TRGCDCJSignal const toSlv(TRGCDCJSignal const & in, int test);
  /// Change to std_logic_vector.
  static TRGCDCJSignal const toSlv(TRGCDCJSignal const &);

  // Change slv to signed.
  static TRGCDCJSignal const slvToSigned(TRGCDCJSignal const & in, double const & toReal, signed long long const & minInt, signed long long const & maxInt, double const & actual, double const & minActual, double const & maxActual, int const & finishClock);
  static TRGCDCJSignal const slvToSigned(TRGCDCJSignal const & in);
  // Change slv to unsigned.
  static TRGCDCJSignal const slvToUnsigned(TRGCDCJSignal const & in, double const & toReal, signed long long const & minInt, signed long long const & maxInt, double const & actual, double const & minActual, double const & maxActual, int const & finishClock);
  static TRGCDCJSignal const slvToUnsigned(TRGCDCJSignal const & in);
  // Change signed to slv.
  static TRGCDCJSignal const signedToSlv(TRGCDCJSignal const & in);
  // Change unsigned to slv.
  static TRGCDCJSignal const unsignedToSlv(TRGCDCJSignal const & in);

  /// Compare two signals.
  static TRGCDCJSignal comp(TRGCDCJSignal const & lhs, std::string operate, TRGCDCJSignal const & rhs);
  

  /// Setters, Getters
  void setName(std::string name);
  void setInt(signed long long value);
  void setActual(double value);
  void setToReal(double value);
  void setType(int type);
  void setBitsize(int bitsize);
  void setFinishClock(int finishClock);
  void setDebug(bool debug);
  void setPrintVhdl(bool printVhdl);
  void setFinalCode(std::string);
  void setVhdlOutputFile(std::string filename);
  void setCommonData(TRGCDCJSignalData *);
  std::string getName() const;
  int getType() const;
  double getBitsize() const;
  signed long long getInt() const;
  signed long long getMinInt() const;
  signed long long getMaxInt() const;
  double getActual() const;
  double getMinActual() const;
  double getMaxActual() const;
  double getRealInt() const;
  double getRealMinInt() const;
  double getRealMaxInt() const;
  double getToReal() const;
  int getFinishClock() const;
  std::vector<std::pair<std::string, std::vector<int> > > const getArgumentSignals() const;
  TRGCDCJSignalData * getCommonData() const;
  int calType() const;
  int calType(double minValue) const;
  int calBitwidth() const;
  int calBitwidth(int type, signed long long minInt, signed long long maxInt);
  int calFinishClock() const;
  std::string getVhdlCode() const;
  std::string getFinalCode() const;
  bool getPrintVhdl() const;
  std::string getVhdlOutputFile() const;

  /// Utilities.
  // Matches unit by shifting bits.
  void matchUnit(TRGCDCJSignal& first, TRGCDCJSignal& second) const;
  // Swaps the TRGCDCJSignals.
  void swap(TRGCDCJSignal& first, TRGCDCJSignal& second) const;
  // Orders the TRGCDCJSignals by bitsize. Next priority is unsigned, signed, minus unsigned. 1: was swapped. 0: was not swapped.
  bool orderLargestBitsize(TRGCDCJSignal& large, TRGCDCJSignal& small) const;
  // Calculate max value for a binary number.
  signed long long calMaxInteger() const;
  signed long long calMinInteger() const;
  // Absolute TRGCDCJSignal. Removes 1 bit if signed or minus unsigned.
  static TRGCDCJSignal const absolute(TRGCDCJSignal const & first);
  //// Calculates output bitwidth and type for operation.
  //void calTypeBitwidth(TRGCDCJSignal const & first, std::string operation, TRGCDCJSignal const & second, int & type, int & bitwidth) const;
  // Calculates integer value with unit of a TRGCDCJSignal.
  static signed long long calInt(double value, TRGCDCJSignal const & mother);
  // Calculates vhdl bitwidth and type for operation.
  static void calVhdlTypeBitwidth(TRGCDCJSignal const & first, std::string operation, TRGCDCJSignal const & second, int & type, int & bitwidth);
  void initArgumentSignals();
  static void combineArguments(TRGCDCJSignal const & first, TRGCDCJSignal const & second, TRGCDCJSignal & result ) ;
  // Sets vhdl code.
  void initVhdlCode();
  void toSignedVhdlCode();
  void toUnsignedVhdlCode();
  // Writes vhdl code.
  static void vhdlCode(TRGCDCJSignal const & first, std::string operation, TRGCDCJSignal const & second, TRGCDCJSignal const & result, std::string & targtVhdlCode);
  static void vhdlCode(std::string operation, TRGCDCJSignal const & first, TRGCDCJSignal const & result, std::string & targetVhdlCode);
  // Writes final vhdl code.
  static std::string replaceWithSignalNames(std::string const & fromVhdlCode, std::vector<std::pair<std::string, std::vector<int> > > const & fromVhdlSignals, int const & finishClock, std::map<std::string, std::vector<int> > & buffers);
  static std::string assignVhdlCode(TRGCDCJSignal const & target, TRGCDCJSignal const & from);
  static std::string chooseVhdlCode(TRGCDCJSignal const & target, TRGCDCJSignal const & reference, std::vector<std::vector<TRGCDCJSignal> > const & data);
  static std::string ifElseVhdlCode(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal *, TRGCDCJSignal> > > > const & data);
  // Prints vhdl code.
  void printVhdl(std::string vhdlCode);
  // Checks underflow or overflow for TRGCDCJSignal.
  void checkInt(std::string name) const;
  // Checks if signal is same signal
  static bool isSameSignal(TRGCDCJSignal const & lhs, TRGCDCJSignal const & rhs);
  // Values => [name, value, bitwidth, min, max, clock]
  static void valuesToMapSignals(std::vector<std::tuple<std::string, double,int,double,double, int> > const & inValues, Belle2::TRGCDCJSignalData* inCommonData, std::map<std::string, Belle2::TRGCDCJSignal> & outMap);
  // Choose => [signalName, FpgaEffects(=1)/NoFpgaEffects(=0)]
  // Values => [name, value, bitwidth, min, max, clock]
  static void mapSignalsToValues(std::map<std::string, Belle2::TRGCDCJSignal>const & inMap, std::vector<std::pair<std::string, int> > const & inChoose, std::vector<std::tuple<std::string, double,int,double,double,int> > & outValues);
  
  private:

  /// Memebers.
  std::string m_name;
  int m_type; // +1: unsigned. -1: unsigned.
  int m_bitsize;
  signed long long m_int;
  std::vector<bool> m_slv;
  signed long long m_minInt;
  signed long long m_maxInt;
  double m_actual;
  double m_minActual;
  double m_maxActual;
  double m_toReal;
  bool m_debug;
  std::string m_vhdlCode;
  // vector<int> is {type, bitwidth, clock}
  std::vector<std::pair<std::string, std::vector<int> > > m_argumentSignals;
  int m_finishClock;
  //bool m_printVhdl;
  TRGCDCJSignalData * m_commonData;

};

} // namespace Belle2

#endif /* TRGCDCJSignal_FLAG_ */
