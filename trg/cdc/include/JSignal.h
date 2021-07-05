/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
    /// Copy constructor.
    explicit TRGCDCJSignal(TRGCDCJSignalData*);
    /// Actual to integer constructor with clock.
    TRGCDCJSignal(int const& bitwidth, double const& value, double const& min, double const& max, int const& clock, TRGCDCJSignalData*);
    /// Actual to integer constructor with clock set to 0.
    TRGCDCJSignal(int const& bitwidth, double const& value, double const& min, double const& max, TRGCDCJSignalData*);
    /// Integer constructor.
    TRGCDCJSignal(signed long long const& intValue, double const& toReal, signed long long const& minInt,
                  signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock,
                  TRGCDCJSignalData*, bool b_slv = 0);
    /// Constant constructor.
    TRGCDCJSignal(double const& value, double const& toReal, TRGCDCJSignalData*);
    /// Slv constructor
    TRGCDCJSignal(std::vector<bool> const& slvValue, int const& finishClock, TRGCDCJSignalData*);

    /// Destructor.
    ~TRGCDCJSignal() {}

    /// Assign operator with setting target clock with outputing final code.
    TRGCDCJSignal& assignTo(TRGCDCJSignal const& rhs, int targetClock, std::string& finalCode);
    /// Assign operator with setting target clock.
    TRGCDCJSignal& assignTo(TRGCDCJSignal const& rhs, int targetClock);
    /// Assign operator.
    TRGCDCJSignal& operator<= (TRGCDCJSignal const& rhs);
    /// Unary operator.
    TRGCDCJSignal const operator- () const;
    /// Arithmetic add operator.
    TRGCDCJSignal const operator+ (TRGCDCJSignal const& rhs) const;
    /// Arithmetic minus operator.
    TRGCDCJSignal const operator- (TRGCDCJSignal const& rhs) const;
    /// Arithmetic multiply operator.
    TRGCDCJSignal const operator* (TRGCDCJSignal const& rhs) const;

    /// Methods.
    void dump() const;
    /// Shift signal. Shift direction is right. operate=0 to change m_toReal(unit).
    TRGCDCJSignal const shift(int nBits, int operate = 1) const;
    /// Outputs an offset signal which is an unsigned signal.
    TRGCDCJSignal const offset(TRGCDCJSignal const& valueMin) const;
    /// Outputs a signal that is not offset.
    TRGCDCJSignal const invOffset(TRGCDCJSignal const& valueMin) const;
    /// Case method. Reference is what is used to determine. Target is what is going to be assigned. Data is (Assignment, From, To)... (Assignment). Last is default.
    /// Choose with target clock. Also has input for target min and target max signal.
    static void choose(TRGCDCJSignal& target, TRGCDCJSignal const& targetMin, TRGCDCJSignal const& targetMax,
                       TRGCDCJSignal const& reference, std::vector<std::vector<TRGCDCJSignal> > data, int targetClock);
    /// Chooes method. Also has input for target min and target max signal.
    static void choose(TRGCDCJSignal& target, TRGCDCJSignal const& targetMin, TRGCDCJSignal const& targetMax,
                       TRGCDCJSignal const& reference, const std::vector<std::vector<TRGCDCJSignal> >& data);
    /// Choose with target clock.
    static void choose(TRGCDCJSignal& target, TRGCDCJSignal const& reference, const std::vector<std::vector<TRGCDCJSignal> >& data,
                       int targetClock);
    /// Choose method.
    static void choose(TRGCDCJSignal& target, TRGCDCJSignal const& reference, const std::vector<std::vector<TRGCDCJSignal> >& data);
    /// If else implementation with target clock.
    static void ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > >& data,
                       int targetClock);
    /// If else implementation.
    static void ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > >& data);
    /// Limits min and max values using specific integers. Use only for ifElse. Doesn't check overflow, underflow to be used in ifElse.
    TRGCDCJSignal const limit(signed long long minInt, signed long long maxInt, double minActual, double maxActual) const;
    /// Limits min and max values using JSignal. Use only for ifElse. Doesn't check overflow, underflow to be used in ifElse.
    TRGCDCJSignal const limit(TRGCDCJSignal const& min, TRGCDCJSignal const& max) const;
    /// Resizes signal.
    TRGCDCJSignal const resize(int bitwidth) const;
    ///// Change to signed signal.
    //TRGCDCJSignal const toSigned() const;
    ///// Change to unsigned signal.
    //TRGCDCJSignal const toUnsigned() const;
    /// Change signal to std_logic_vector using vector<bool>. (Will be deprecated.)
    static TRGCDCJSignal const toSlv(TRGCDCJSignal const& in, int test);
    /// Change singal to std_logic_vector.
    static TRGCDCJSignal const toSlv(TRGCDCJSignal const&);

    /// Change slv to signed with target clock.
    static TRGCDCJSignal const slvToSigned(TRGCDCJSignal const& in, double const& toReal, signed long long const& minInt,
                                           signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock);
    /// Change slv to signed.
    static TRGCDCJSignal const slvToSigned(TRGCDCJSignal const& in);
    /// Change slv to unsigned with target clock.
    static TRGCDCJSignal const slvToUnsigned(TRGCDCJSignal const& in, double const& toReal, signed long long const& minInt,
                                             signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock);
    /// Change slv to unsigned with target clock.
    static TRGCDCJSignal const slvToUnsigned(TRGCDCJSignal const& in);
    /// Change signed to slv.
    static TRGCDCJSignal const signedToSlv(TRGCDCJSignal const& in);
    /// Change unsigned to slv.
    static TRGCDCJSignal const unsignedToSlv(TRGCDCJSignal const& in);

    /// Compare two signals.
    static TRGCDCJSignal comp(TRGCDCJSignal const& lhs, const std::string& operate, TRGCDCJSignal const& rhs);


    /// Setters, Getters
    /// Sets name of signal.
    void setName(std::string name);
    /// Sets integer value of signal.
    void setInt(signed long long value);
    /// Sets float value of signal.
    void setActual(double value);
    /// Sets toReal factor of signal.
    void setToReal(double value);
    /// Sets type of signal.
    void setType(int type);
    /// Sets bitwidth of signal.
    void setBitsize(int bitsize);
    /// Sets finish clock of signal.
    void setFinishClock(int finishClock);
    /// Sets debug flag for signal.
    void setDebug(bool debug);
    /// Sets print flag for signal.
    void setPrintVhdl(bool printVhdl);
    /// Sets ouptut VHDL filename.
    void setVhdlOutputFile(std::string filename);
    /// Sets JSignalData to signal.
    void setCommonData(TRGCDCJSignalData*);
    /// Get name of signal.
    std::string getName() const;
    /// Get type of signal.
    int getType() const;
    /// Get bitwidth of signal.
    double getBitsize() const;
    /// Get integer value of signal.
    signed long long getInt() const;
    /// Get minimum integer value of signal.
    signed long long getMinInt() const;
    /// Get maximum integer value of signal.
    signed long long getMaxInt() const;
    /// Get float value of signal.
    double getActual() const;
    /// Get minimum float value of signal.
    double getMinActual() const;
    /// Get maximum float value of signal.
    double getMaxActual() const;
    /// Get converted float value of integer.
    double getRealInt() const;
    /// Get converted float value of minimum integer.
    double getRealMinInt() const;
    /// Get converted float value of maximum integer.
    double getRealMaxInt() const;
    /// Get toReal value of signal.
    double getToReal() const;
    /// Gets clock tick where signal is make.
    int getFinishClock() const;
    /// Gets the signal names and values that made this signal.
    std::vector<std::pair<std::string, std::vector<int> > > const getArgumentSignals() const;
    /// Get the sharged JSignalData.
    TRGCDCJSignalData* getCommonData() const;
    /// Calculated what type the signal is.
    int calType() const;
    /// Calculated the type using the argument.
    int calType(double minValue) const;
    /// Calculates the bit width.
    int calBitwidth() const;
    /// Calculates the bit width using the arguments.
    int calBitwidth(int type, signed long long minInt, signed long long maxInt);
    /// Claculates the clock tick where the signal is valid.
    int calFinishClock() const;
    /// Gets the vhdl code for the signal.
    std::string getVhdlCode() const;
    /// Gets the print vhdl flag from common JSignalData class.
    bool getPrintVhdl() const;
    /// Gets the vhdl output file name.
    std::string getVhdlOutputFile() const;

    /// Utilities.
    /// Matches unit by shifting bits.
    void matchUnit(TRGCDCJSignal& first, TRGCDCJSignal& second) const;
    /// Swaps the TRGCDCJSignals.
    void swap(TRGCDCJSignal& first, TRGCDCJSignal& second) const;
    /// Orders the TRGCDCJSignals by bitsize. Next priority is unsigned, signed, minus unsigned. 1: was swapped. 0: was not swapped.
    bool orderLargestBitsize(TRGCDCJSignal& large, TRGCDCJSignal& small) const;
    /// Calculate max value for a binary number.
    signed long long calMaxInteger() const;
    /// Calculate min value for a binary number.
    signed long long calMinInteger() const;
    /// Absolute TRGCDCJSignal. Removes 1 bit if signed or minus unsigned.
    static TRGCDCJSignal const absolute(TRGCDCJSignal const& first);
    //// Calculates output bitwidth and type for operation.
    //void calTypeBitwidth(TRGCDCJSignal const & first, std::string operation, TRGCDCJSignal const & second, int & type, int & bitwidth) const;
    /// Calculates integer value with unit of a TRGCDCJSignal.
    static signed long long calInt(double value, TRGCDCJSignal const& mother);
    /// Calculates vhdl bitwidth and type for operation.
    static void calVhdlTypeBitwidth(TRGCDCJSignal const& first, const std::string& operation, TRGCDCJSignal const& second, int& type,
                                    int& bitwidth);
    /// Initializes the argument signals for the signal.
    void initArgumentSignals();
    /// Combines arguments of signals.
    static void combineArguments(TRGCDCJSignal const& first, TRGCDCJSignal const& second, TRGCDCJSignal& result) ;
    ///  Initializes the vhdl code.
    void initVhdlCode();
    /// Changes signal type to signed type in VHDL.
    void toSignedVhdlCode();
    /// Changes signal type to unsigned type in VHDL.
    void toUnsignedVhdlCode();
    /// Writes vhdl code using two signals.
    static void vhdlCode(TRGCDCJSignal const& first, std::string operation, TRGCDCJSignal const& second, TRGCDCJSignal const& result,
                         std::string& targtVhdlCode);
    /// Writes vhdl code using one signal.
    static void vhdlCode(std::string operation, TRGCDCJSignal const& first, TRGCDCJSignal const& result, std::string& targetVhdlCode);
    /// Writes final vhdl code.
    static std::string replaceWithSignalNames(std::string const& fromVhdlCode,
                                              std::vector<std::pair<std::string, std::vector<int> > > const& fromVhdlSignals, int const& finishClock,
                                              std::map<std::string, std::vector<int> >& buffers);
    /// Writes the assign vhdl code.
    static std::string assignVhdlCode(TRGCDCJSignal const& target, TRGCDCJSignal const& from);
    /// Writes the choose vhdl code.
    static std::string chooseVhdlCode(TRGCDCJSignal const& target, TRGCDCJSignal const& reference,
                                      std::vector<std::vector<TRGCDCJSignal> > const& data);
    /// Writes the ifElse vhdl code.
    static std::string ifElseVhdlCode(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > >
                                      const& data);
    /// Prints vhdl code.
    void printVhdl(const std::string& vhdlCode) const;
    /// Checks underflow or overflow for TRGCDCJSignal.
    void checkInt(const std::string& name) const;
    /// Checks if signal is same signal
    static bool isSameSignal(TRGCDCJSignal const& lhs, TRGCDCJSignal const& rhs);
    /// Values => [name, value, bitwidth, min, max, clock]
    /// Changes values to signals.
    static void valuesToMapSignals(std::vector<std::tuple<std::string, double, int, double, double, int> > const& inValues,
                                   Belle2::TRGCDCJSignalData* inCommonData, std::map<std::string, Belle2::TRGCDCJSignal>& outMap);
    /// Choose => [signalName, FpgaEffects(=1)/NoFpgaEffects(=0)]
    /// Values => [name, value, bitwidth, min, max, clock]
    /// Changes signals to values.
    static void mapSignalsToValues(std::map<std::string, Belle2::TRGCDCJSignal>const& inMap,
                                   std::vector<std::pair<std::string, int> > const& inChoose,
                                   std::vector<std::tuple<std::string, double, int, double, double, int> >& outValues);

  private:

    /// Memebers.
    /// Name of signal
    std::string m_name;
    /// Type of signal.
    int m_type; /// +1: unsigned. -1: unsigned.
    /// Bitwidth of signal.
    int m_bitsize;
    /// integer value of signal.
    signed long long m_int;
    /// storage for std_logic_vector. (Will be deprecated.)
    std::vector<bool> m_slv;
    /// The minimum integer value.
    signed long long m_minInt;
    /// The maximum integer value.
    signed long long m_maxInt;
    /// The float value.
    double m_actual;
    /// The minimum float value.
    double m_minActual;
    /// The maximum float value.
    double m_maxActual;
    /// The toReal value.
    double m_toReal;
    /// The debug flag.
    bool m_debug;
    /// Holds the vhdl code.
    std::string m_vhdlCode;
    /// vector<int> is {type, bitwidth, clock}
    /// Holds the arguments that made the signal.
    std::vector<std::pair<std::string, std::vector<int> > > m_argumentSignals;
    /// Holds the clock tick when the signal is valid.
    int m_finishClock;
    //bool m_printVhdl;
    /// The common JSignalData.
    TRGCDCJSignalData* m_commonData;

  };

} /// namespace Belle2

#endif /* TRGCDCJSignal_FLAG_ */
