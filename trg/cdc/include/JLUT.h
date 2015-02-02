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
    TRGCDCJLUT(const std::string & name="no_name");
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
    void setData(int inputBitsize, int outputBitSize, const std::string & filename, bool twoscomplement=0);

    /// set LUT function.
    void setFunction(std::function<int(int)> function);
    int setFloatFunction(std::function<double(double)> function,
                          TRGCDCJSignal const & input,
                          TRGCDCJSignal const & minInv, TRGCDCJSignal const & maxInv, double outputToReal,
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

    /// get LUT function.
    std::function<int(int)> getFunction();
    std::function<double(double)> getFloatFunction();

    /// get output type.
    int getOutputFlag() const;

    /// get output type.
    int getOutputType() const;

    /// get output.
    int getOutput(int) const;
    double getFloatOutput(double) const;

    /// Print variables for LUT
    void dump();

    /// make COE file.
    void makeCOE(std::string="");

    /// Storage for values used in function.
    std::vector<double> m_const;

    /// Operate
    std::string operate(std::string out, std::string in, std::map<std::string, std::map<std::string, double>* >& m_intStorage);
    // Returns LUT value. Out is LUT value + offset. (LUT value + offset do not obey addition bitsize rules.)
    TRGCDCJSignal const operate(TRGCDCJSignal const& in, TRGCDCJSignal & out);
    TRGCDCJSignal const operate(TRGCDCJSignal const& in, TRGCDCJSignal & out, int) const;
    // Output is LUT Out + offset. Input is non offset input. 
    void operateNoOffset(TRGCDCJSignal const& in, TRGCDCJSignal & out) const;

    // Creates vhdlCode for lut.
    std::string const lutVhdlCode(std::string const & shiftOffsetInput) const;

  private:

    /// Name.
    std::string m_name;

    /// LUT data.
    std::vector<int> m_data;

    /// LUT filename.
    std::string m_fileName;
    std::string m_outFileName;

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

    bool m_write;

    /// Changing float function to int function variables.
    double m_inputOffset;
    double m_inputActualMax;
    double m_inputActualOffset;
    double m_outputOffset;
    double m_outputIntMax;
    double m_inputToReal;
    double m_toReal;
    // Below is needed.
    int m_inputShiftBits;
    double m_outputType;
    double m_outputNBitsWithOffset;
    // Simpiler signals.
    TRGCDCJSignal m_inputMin;
    TRGCDCJSignal m_inputMax;
    TRGCDCJSignal m_shiftOutputMin;
    TRGCDCJSignal m_shiftOffsetOutputMax;
    TRGCDCJSignal m_shiftOffsetInput;

};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCJLUT::name(void) const {
    return m_name;
}


} // namespace Belle2

#endif /* TRGCDCLUT_FLAG_ */
