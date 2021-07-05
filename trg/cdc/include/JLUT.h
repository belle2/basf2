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
// Filename : JLUT.h
// Section  : TRG CDC
// Owner    : Jaebak KIM (K.U.)
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class for LUT in CDC Trigger
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCJLUT_FLAG_
#define TRGCDCJLUT_FLAG_

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "JSignal.h"

#ifdef TRGCDC_SHORT_NAMES
#define JLUT TRGCDCJLUT
#endif

namespace Belle2 {

//class TRGCDCJSignal;

/// A class to use LUTs for TRGCDC
  class TRGCDCJLUT {

  public:

    /// Contructor.
    TRGCDCJLUT(const std::string& name = "no_name");

    /// Copy constructor.
    TRGCDCJLUT(const TRGCDCJLUT& in);

    /// Destructor.
    virtual ~TRGCDCJLUT();

    /// Operators.
    //TRGCDCJLUT& operator= (const TRGCDCJLUT& in);

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// set LookUpTable(LUT) data.
    void setData(int inputBitsize, int outputBitSize, const std::string& filename, bool twoscomplement = 0);

    /// set LUT function directly using int values.
    void setFunction(std::function<int(int)> function);

    /// set LUT function using JSignals.
    int setFloatFunction(std::function<double(double)> function,
                         TRGCDCJSignal const& input,
                         TRGCDCJSignal const& minInv, TRGCDCJSignal const& maxInv, double outputToReal,
                         int lutInputBitwidth, int lutOutputBitwidth);

    /// set output flag.
    void setOutputFlag(int outputFlag);

    /// set input & output bitsize.
    void setBitsize(int inputBitsize, int outputBitsize);

    /// set input bitsize.
    void setInputBitsize(int inputBitsize);

    /// set output bitsize.
    void setOutputBitsize(int outputBitsize);

    /// get LUT filename
    std::string getFileName() const;

    /// get input bitsize
    int getInputBitsize() const;

    /// get output bitsize
    int getOutputBitsize() const;

    /// get LUT function using int values.
    std::function<int(int)> getFunction();

    /// get LUT function using float values.
    std::function<double(double)> getFloatFunction();

    /// get output type.
    int getOutputFlag() const;

    /// get output type.
    int getOutputType() const;

    /// get output using int values.
    int getOutput(int) const;

    /// get output using float values.
    double getFloatOutput(double) const;

    /// Print variables for LUT
    void dump();

    /// make COE file.
    void makeCOE(const std::string& fileName = "");

    /// Storage for values used in function.
    std::vector<double> m_const;

    /// Operate on LUT. (Will be deprecated.)
    std::string operate(std::string out, std::string in, std::map<std::string, std::map<std::string, double>* >& m_intStorage);

    /// Returns LUT value. Out is LUT value + offset. (LUT value + offset do not obey addition bitsize rules.)
    TRGCDCJSignal const operate(const TRGCDCJSignal& in, TRGCDCJSignal& out);

    /// Output is LUT Out + offset. Input is non offset input.
    void operateNoOffset(TRGCDCJSignal const& in, TRGCDCJSignal& out) const;

    /// Creates vhdlCode for lut.
    std::string const lutVhdlCode(std::string const& shiftOffsetInput) const;

  private:

    /// Name.
    std::string m_name;

    /// LUT data.
    std::vector<int> m_data;

    /// LUT filename.
    std::string m_fileName;

    /// LUT function.
    std::function<int(int)> m_function;

    /// Float LUT function.
    std::function<double(double)> m_floatFunction;

    /// Flag for output. 0 is from function. 1 is from LUT file.
    int m_outputFlag;

    /// input bitsize. Number of bits.
    int m_inputBitsize;

    /// output bitsize. Number of bits.
    int m_outputBitsize;

    /// VHDL write flag for signals. 1: Write is on. 0: Write is off.
    bool m_write;

    /// Changing float function to int function variables.
    double m_inputOffset;

    /// The float offset of the output for LUT.
    double m_outputOffset;

    /// The integer offset of the output for LUT.
    double m_outputIntMax;

    /// The factor of the input integer to change to float value.
    double m_inputToReal;

    /// The factor to change integer to float value.
    double m_toReal;

    /// The number of bits to shift to make input the wanted bit width.
    int m_inputShiftBits;

    /// The output type of the LUT. (Will be deprecated.)
    double m_outputType;

    /// The output bit width with offset. (Will be deprecated.)
    double m_outputNBitsWithOffset;

    /// The minimum input signal.
    TRGCDCJSignal m_inputMin;

    /// The maximum input signal.
    TRGCDCJSignal m_inputMax;

    /// The minimum output signal that is bit shifted.
    TRGCDCJSignal m_shiftOutputMin;

    /// The maximum output value that is bit shifted.
    TRGCDCJSignal m_shiftOffsetOutputMax;

    /// The bt shifted offset signal of input.
    TRGCDCJSignal m_shiftOffsetInput;

  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCJLUT::name(void) const
  {
    return m_name;
  }


} // namespace Belle2

#endif /* TRGCDCLUT_FLAG_ */
