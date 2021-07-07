/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class for  LUT in TRGCDC
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#ifndef __EXTERNAL__
#include "trg/cdc/JLUT.h"
#include "trg/cdc/FpgaUtility.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JSignalData.h"
#else
#include "JLUT.h"
#include "FpgaUtility.h"
#include "JSignal.h"
#include "JSignalData.h"
#endif
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;

namespace Belle2 {

  std::string TRGCDCJLUT::version(void) const
  {
    return string("TRGCDCJLUT 0.00");
  }

  TRGCDCJLUT::TRGCDCJLUT(const string& name)
    : m_name(name),
      m_outputFlag(0), m_inputBitsize(0), m_outputBitsize(0),
      m_inputOffset(0), m_outputOffset(0), m_outputIntMax(0),
      m_inputToReal(0), m_toReal(0), m_inputShiftBits(0),
      m_outputType(0), m_outputNBitsWithOffset(0)
  {
    m_write = 0;
  }

  TRGCDCJLUT::TRGCDCJLUT(const TRGCDCJLUT& in)
    : m_const(in.m_const),
      m_name(in.m_name),
      m_fileName(in.m_fileName),
      m_function(in.m_function),
      m_floatFunction(in.m_floatFunction),
      m_inputMin(in.m_inputMin),
      m_inputMax(in.m_inputMax),
      m_shiftOutputMin(in.m_shiftOutputMin),
      m_shiftOffsetOutputMax(in.m_shiftOffsetOutputMax)
  {
    m_outputFlag = in.m_outputFlag;
    m_inputBitsize = in.m_inputBitsize;
    m_outputBitsize = in.m_outputBitsize;
    m_inputOffset = in.m_inputOffset;
    m_inputToReal = in.m_inputToReal;
    m_outputOffset = in.m_outputOffset;
    m_outputNBitsWithOffset = in.m_outputNBitsWithOffset;
    m_outputType = in.m_outputType;
    m_toReal = in.m_toReal;
    m_inputShiftBits = in.m_inputShiftBits;
    m_outputIntMax = in.m_outputIntMax;
    m_write = 0;
  }

  TRGCDCJLUT::~TRGCDCJLUT()
  {
  }

  void TRGCDCJLUT::setData(int inputBitsize, int outputBitsize, const string& filename, bool twosComplement)
  {
    ifstream coeFile;
    coeFile.open(filename.c_str());
    if (coeFile.fail()) {
      cout << "TRGCDCJLUT !!! can not open file : " << filename << endl;
      cout << "              LUT is not initialized yet" << endl;
      return;
    } else {
      // Set values
      setInputBitsize(inputBitsize);
      setOutputBitsize(outputBitsize);
      m_fileName = filename;
      setOutputFlag(1);
      int inputSize = pow(2, inputBitsize);
      m_data.resize(inputSize);
      // Read file
      string t_line;
      stringstream reformatCoeFile;
      // Reformat file.
      while (getline(coeFile, t_line)) {
        // Remove all spaces
        t_line.erase(remove_if(t_line.begin(), t_line.end(), ::isspace), t_line.end());
        // Remove blank lines
        if (t_line.size() == 0) continue;
        // Remove all characters after ';'
        size_t t_iFind = t_line.find(";");
        if (t_iFind != string::npos) {
          t_line.erase(t_iFind + 1, string::npos);
        }
        // Remove lines for comments
        if (t_line == ";") continue;
        // Combine lines until ';'
        if (t_line.find(";") == string::npos) reformatCoeFile << t_line;
        // Replace ';' with endl
        else reformatCoeFile << t_line.substr(0, t_line.size() - 1) << endl;
      }
      // Process reformatted file.
      // vector<string> keywords = {"memory_initialization_radix", "memory_initialization_vector"};
      int t_radix = 0;
      vector<string> t_rawData;
      while (getline(reformatCoeFile, t_line)) {
        //cout<<t_line<<endl;
        // Find keywords
        size_t t_iFind = t_line.find("=");
        if (t_iFind != string::npos) {
          // Find values for keywords
          // Find radix
          if (t_line.substr(0, t_iFind) == "memory_initialization_radix") {
            t_radix = atoi(t_line.substr(t_iFind + 1, string::npos).c_str());
          } else if (t_line.substr(0, t_iFind) == "memory_initialization_vector") {
            // Find data values
            string t_dataLine = t_line.substr(t_iFind + 1, string::npos);
            while (1) {
              t_iFind = t_dataLine.find(",");
              if (t_iFind != string::npos) {
                t_rawData.push_back(t_dataLine.substr(0, t_iFind));
                t_dataLine.erase(0, t_iFind + 1);
              } else {
                // last entry
                t_rawData.push_back(t_dataLine);
                break;
              }
            } // Finding data values loop
          } else {
            cout << "[Error] TRGCDCJLUT::setData() => .coe format keyword is wrong. Aborting" << endl;
            break;
          } // End of finding values for keywords
        } else {
          cout << "[Error] TRGCDCJLUT::setData() => .coe format is wrong. Needs keywords. Aborting" << endl;
          break;
        } // End of finding keywords
      }
      //// Print rawData
      //for(auto it=t_rawData.begin(); it!=t_rawData.end(); it++){
      //  cout<<(*it)<<endl;
      //}
      if (!twosComplement) {
        for (int iData = 0; iData < int(t_rawData.size()); iData++) {
          // Change string to float octal.
          m_data[iData] = FpgaUtility::arbToDouble(t_rawData[iData], t_radix);
        }
      } else {
        // Find number of bits in binary for data.
        int nBits = t_rawData[0].size();
        // Calculate max value.
        int t_max = pow(t_radix, nBits) - 1;
        // Calculate number of bits
        nBits = floor(log(t_max) / log(2)) + 1;
        for (int iData = 0; iData < int(t_rawData.size()); iData++) {
          // Change to binary.
          string t_binary = FpgaUtility::arbToArb(t_rawData[iData], t_radix, 2, nBits);
          // Change to signed.
          string t_signedBinary = FpgaUtility::twosComplementToSigned(t_binary);
          // Change to float octal.
          m_data[iData] = FpgaUtility::arbToDouble(t_signedBinary, 2);
        }
      } // End filling m_data.
      // Print all values of LUT
      cout << "LUT(" << m_name << ") data" << endl;
      for (int iData = 0; iData < int(m_data.size()); iData++) {
        cout << "[" << iData << "] " << m_data[iData] << endl;
      }
      cout << "TTRGCDCJLUT ... LUT(" << m_name << ") initilized with " << filename << endl;
    }
    coeFile.close();
  }

  void TRGCDCJLUT::setFunction(function<int(int)> function)
  {
    m_function = function;
  }


  int TRGCDCJLUT::setFloatFunction(function<double(double)> function,
                                   TRGCDCJSignal const& input,
                                   //TRGCDCJSignal const & outputMin, TRGCDCJSignal const & outputMax,
                                   TRGCDCJSignal const& minInv, TRGCDCJSignal const& maxInv, double outputToReal,
                                   int lutInputBitwidth, int lutOutputBitwidth)
  {

    signed long long t_int;
    double t_toReal;
    double t_actual;
    TRGCDCJSignalData* t_commonData;

    // Save values.
    m_inputBitsize = lutInputBitwidth;
    m_outputBitsize = lutOutputBitwidth;
    m_floatFunction = function;
    // Create input min and max.
    t_int = input.getMinInt();
    t_toReal = input.getToReal();;
    t_actual = input.getMinActual();
    t_commonData = input.getCommonData();
    m_inputMin = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, t_commonData);
    t_int = input.getMaxInt();
    t_actual = input.getMaxActual();
    m_inputMax = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, t_commonData);

    //cout<<"<<<m_inputMin>>>"<<endl; m_inputMin.dump();
    //cout<<"<<<m_inputMax>>>"<<endl; m_inputMin.dump();

    // Calculates number of bits to shift for input.
    //int t_offsetNBits = m_inputMax.offset(m_inputMin, m_inputMax).getBitsize();
    int t_offsetNBits = m_inputMax.offset(m_inputMin).getBitsize();
    m_inputShiftBits = t_offsetNBits - lutInputBitwidth;
    if (m_inputShiftBits < 0) {
      m_inputShiftBits = 0;
      //cout<<"[Warning] TRGCDCJLUT::setFloatFunction() => Changing m_inputBitsize from "<<lutInputBitwidth<<" to optimal bitwidth "<<t_offsetNBits<<". "<<endl;
      m_inputBitsize = t_offsetNBits;
    }

    // Calculates number of bits to shift for lut output bitwidth to be lutOutputBitwidth.
    //cout<<"<<<minInv>>>"<<endl; minInv.dump();
    //cout<<"<<<maxInv>>>"<<endl; maxInv.dump();
    double t_outputRealMinInt = function(minInv.getRealInt());
    double t_outputRealMaxInt = function(maxInv.getRealInt());
    if (std::isnan(t_outputRealMinInt)) {
      cout << "[Error] TRGCDCJLUT::setFloatFunction() => t_outputRealMinInt is nan. Please change minInv signal so that function(minInv.getRealInt()) does not give nan."
           << endl;
    }
    if (std::isnan(t_outputRealMaxInt)) {
      cout << "[Error] TRGCDCJLUT::setFloatFunction() => t_outputRealMaxInt is nan. Please change maxInv so that function(maxInv.getRealInt()) does not give nan"
           << endl;
    }
    if (t_outputRealMaxInt < t_outputRealMinInt) {
      cout << "[Error] TRGCDCJLUT::setFloatFunction() => t_outputRealMaxInt is smaller than t_outputRealMinInt." << endl;
      return 0;
    }
    double t_outputMinActual = function(minInv.getActual());
    double t_outputMaxActual = function(maxInv.getActual());
    double t_outputToReal;
    t_outputToReal = outputToReal;
    double tt_factor = log((max(t_outputRealMaxInt, abs(t_outputRealMinInt)) - t_outputRealMinInt) / ((pow(2,
                           lutOutputBitwidth) - 0.5) * t_outputToReal)) / log(2);
    tt_factor = ceil(tt_factor);
    // Create outputMin.
    //cout<<"t_outputRealMinInt:"<<t_outputRealMinInt<<" t_outputToReal:"<<t_outputToReal<<" tt_factor:"<<tt_factor<<endl;
    //cout<<"rountInt:"<<t_outputRealMinInt / t_outputToReal / pow(2, tt_factor)<<endl;
    t_int = FpgaUtility::roundInt(t_outputRealMinInt / t_outputToReal / pow(2, tt_factor));
    t_toReal = t_outputToReal * pow(2, tt_factor);
    t_actual = t_outputMinActual;
    //cout<<"t_int:"<<t_int<<" t_toReal:"<<t_toReal<<" t_actual:"<<t_actual<<endl;
    Belle2::TRGCDCJSignal t_shiftOutputMin = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1,
                                             t_commonData);
    //cout<<"<<<t_shiftOutputMin>>>"<<endl; t_shiftOutputMin.dump();
    //cout<<"minInvRealInt: "<<minInv.getRealInt()<<" RealMinInt: "<<t_outputRealMinInt<<" int: "<<t_int<<endl;
    //cout<<"minInvActual:  "<<minInv.getActual()<<" minActual: "<<t_outputMinActual<<endl;
    // Create outputMax.
    t_int = FpgaUtility::roundInt(t_outputRealMaxInt / t_shiftOutputMin.getToReal()) - t_shiftOutputMin.getInt();
    t_toReal = t_shiftOutputMin.getToReal();
    t_actual = t_outputMaxActual - t_shiftOutputMin.getActual();
    Belle2::TRGCDCJSignal t_shiftOffsetOutputMax = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual,
                                                   -1, t_commonData);
    //cout<<"maxInv: "<<maxInv.getRealInt()<<" RealMaxInt: "<<t_outputRealMaxInt<<" int: "<<FpgaUtility::roundInt(t_outputRealMaxInt/t_shiftOutputMin.getToReal())<<endl;
    //cout<<"maxInvActual:  "<<maxInv.getActual()<<" maxActual: "<<t_outputMaxActual<<endl;
    //cout<<"<<<t_shiftOffsetOutputMax>>>"<<endl; t_shiftOffsetOutputMax.dump();

    //// Compare between before and after.
    m_shiftOutputMin = t_shiftOutputMin;
    m_shiftOffsetOutputMax = t_shiftOffsetOutputMax;

    // Save value for operator function.
    //cout<<"actual_range:   "<<max(outputActualMax, abs(outputActualMin)) - outputActualMin<<endl;
    //cout<<"int_range:      "<<(pow(2,requiredNBits)-0.5)<<endl;
    //cout<<"intReal_range:  "<<(pow(2,requiredNBits)-0.5)*outputToReal<<endl;
    //cout<<"outputToReal:   "<<outputToReal<<endl;
    //cout<<"actual/int:     "<<(max(outputActualMax, abs(outputActualMin)) - outputActualMin)/((pow(2,requiredNBits)-0.5)*outputToReal)<<endl;
    //cout<<"t_factor_raw:   "<<log((max(outputActualMax, abs(outputActualMin)) - outputActualMin)/((pow(2,requiredNBits)-0.5)*outputToReal))/log(2)<<endl;
    //cout<<"t_factor:       "<<t_factor<<endl;
    //cout<<"m_outputOffset: "<<m_outputOffset<<endl;
    //cout<<"m_outputType:   "<<m_outputType<<endl;
    //cout<<"m_outputNBits:  "<<m_outputNBitsWithOffset<<endl;
    //cout<<"m_toReal:       "<<m_toReal<<endl;
    // Set integer function.
    m_function = [&](int anInt) ->
    int{
      // Limit input
      if (anInt < 0)
      {
        if (m_write != 1) cout << "[Warning] TRGCDCJLUT::m_function => input is smaller then 0. Changed to 0." << endl;
        anInt = 0;
      }
      double t_input = (anInt + m_inputMin.shift(m_inputShiftBits, 0).getInt()) * m_inputMin.shift(m_inputShiftBits, 0).getToReal();
      double t_realResult = m_floatFunction(t_input);
      double result = FpgaUtility::roundInt(t_realResult / m_shiftOutputMin.getToReal() - m_shiftOutputMin.getInt());
      // Limit output according to requiredNBits
      if (result < 0)
      {
        result = 0;
      }
      if (result > m_shiftOffsetOutputMax.getInt())
      {
        result = m_shiftOffsetOutputMax.getInt();
      }
      // Warnings. Use ideal result for checking output.
      double t_inputActual = anInt * m_inputMin.shift(m_inputShiftBits, 0).getToReal() + m_inputMin.getActual();
      double t_outputActual = m_floatFunction(t_inputActual);
      if (t_outputActual < m_shiftOutputMin.getActual())
      {
        if (m_write != 1) {
          cout << "[Warning] TRGCDCJLUT::m_function => output is smaller then 0. Changed to 0." << endl;
          cout << "Could happen if invY_min and invY_max are inside x range." << endl;
        }
      }
      if (t_outputActual > m_shiftOffsetOutputMax.getActual() + m_shiftOutputMin.getActual())
      {
        if (m_write != 1) {
          cout << "[Warning] TRGCDCJLUT::m_function => output is larger then allowed max value. Changed to " <<
               m_shiftOffsetOutputMax.getInt() << "." << endl;
          cout << "Could happen if invY_min and invY_max are inside x range." << endl;
        }
      }
      return result;
    };

    return 1;
  }


  void TRGCDCJLUT::setBitsize(int inputBitsize, int outputBitsize)
  {
    setInputBitsize(inputBitsize);
    setOutputBitsize(outputBitsize);
  }

  void TRGCDCJLUT::setOutputFlag(int outputFlag)
  {
    m_outputFlag = outputFlag;
  }

  void TRGCDCJLUT::setInputBitsize(int inputBitsize)
  {
    m_inputBitsize = inputBitsize;
  }

  void TRGCDCJLUT::setOutputBitsize(int outputBitsize)
  {
    m_outputBitsize = outputBitsize;
  }

  string TRGCDCJLUT::getFileName() const
  {
    return m_fileName;
  }

  int TRGCDCJLUT::getInputBitsize() const
  {
    return m_inputBitsize;
  }

  int TRGCDCJLUT::getOutputBitsize() const
  {
    return m_outputBitsize;
  }

  function<int(int)> TRGCDCJLUT::getFunction()
  {
    return m_function;
  }

  function<double(double)> TRGCDCJLUT::getFloatFunction()
  {
    return m_floatFunction;
  }

  int TRGCDCJLUT::getOutputFlag() const
  {
    return m_outputFlag;
  }

  int TRGCDCJLUT::getOutputType() const
  {
    return m_outputType;
  }

  int TRGCDCJLUT::getOutput(int input) const
  {
    return m_function(input);
  }

  double TRGCDCJLUT::getFloatOutput(double input) const
  {
    return m_floatFunction(input);
  }

  void TRGCDCJLUT::dump()
  {
    cout << "<<<[LUT] " << m_name << ">>>" << endl;
    if (m_fileName != "") cout << "LUT filename: " << m_fileName << endl;
    cout << "In bitsize: " << m_inputBitsize << " Out bitsize: " << m_outputBitsize << endl;
    cout << "[input] offset: " << m_inputOffset << " toReal: " << m_inputToReal << endl;
    cout << "[output] offset: " << m_outputOffset << " toReal: " << m_toReal << endl;
    cout << "[output] type: " << m_outputType << " NBitsWithOffset: " << m_outputNBitsWithOffset << endl;
    cout << "<<<[LUT] " << m_name << ">>>" << endl;
  }

  /// make COE file.
  void TRGCDCJLUT::makeCOE(const string& fileName)
  {
    m_write = 1;
    // Check if function is set.
    if (!m_function.operator bool()) {
      cout << "[Error] TRGCDCJLUT::makeCOE() => m_function is not set. Aborting." << endl;
      return;
    }
    // Check if m_inputBitsize is set.
    if (m_inputBitsize == 0) {
      cout << "[Error] TRGCDCJLUT::makeCOE() => inputBitSize is not set. Aborting." << endl;
      return;
    }
    // Set file name.
    string t_fileName;
    if (fileName != "") t_fileName = fileName;
    else if (name() != "") {
      t_fileName = name();
      t_fileName += ".coe";
    } else t_fileName = "untitled.coe";
    // Create file.
    ofstream coeFile;
    coeFile.open(t_fileName.c_str());

    coeFile << "* [Information for COE " << fileName << " ]" << endl;
    coeFile << "* in_real = in * " << m_inputMin.getToReal() << endl;
    coeFile << "* LUT_in = (in - " << m_inputMin.getInt() << ") * 2^" << m_inputShiftBits << endl;
    coeFile << "* x = (LUT_in + " << m_inputMin.getInt() << " * 2^" << m_inputShiftBits << ") * " << m_inputMin.shift(m_inputShiftBits,
            0).getToReal() << endl;
    coeFile << "* y_int = round( f(x) / " << m_shiftOutputMin.getToReal() << " - " << m_shiftOutputMin.getInt() << " )" << endl;
    coeFile << "* LUT_out = Limit to 0 < y_int < " << m_shiftOffsetOutputMax.getInt() << endl;
    coeFile << "* out = LUT_out + " << m_shiftOutputMin.getInt() << endl;
    coeFile << "* out_Real = out * " << m_shiftOutputMin.getToReal() << endl;

    // Meta data for file.
    coeFile << "memory_initialization_radix=10;" << endl;
    coeFile << "memory_initialization_vector=" << endl;
    // Fill content of file.
    for (unsigned index = 0; index < pow(2, m_inputBitsize) - 1; index++) {
      //cout<<"Directly: "<<m_function(index)<<endl;
      //cout<<"Indirect: "<<getOutput(index)<<endl;
      coeFile << m_function(index) << "," << endl;
    }
    coeFile << m_function(pow(2, m_inputBitsize) - 1) << ";" << endl;
    // Close file.
    coeFile.close();
    m_write = 0;
  }

  // Return name of offset.
  string TRGCDCJLUT::operate(std::string out, std::string in, std::map<std::string, std::map<std::string, double>* >& m_intStorage)
  {
    double floatInput = (*m_intStorage["int"])[in] + m_inputOffset;
    floatInput *= m_inputToReal;
    // Make offset value.
    stringstream t_offsetName;
    t_offsetName << out << "_o" << m_outputOffset;
    (*m_intStorage["type"])[t_offsetName.str()] = 1;
    (*m_intStorage["bitSize"])[t_offsetName.str()] = m_outputBitsize;
    (*m_intStorage["toReal"])[t_offsetName.str()] = m_toReal;
    (*m_intStorage["real"])[t_offsetName.str()] = getFloatOutput(floatInput) - m_outputOffset * m_toReal;
    (*m_intStorage["int"])[t_offsetName.str()] = getOutput((*m_intStorage["int"])[in]);
    (*m_intStorage["realInt"])[t_offsetName.str()] = (*m_intStorage["int"])[t_offsetName.str()] * m_toReal;
    // Make no offset value.
    (*m_intStorage["type"])[out] = m_outputType;
    (*m_intStorage["bitSize"])[out] = m_outputNBitsWithOffset;
    (*m_intStorage["toReal"])[out] = m_toReal;
    (*m_intStorage["real"])[out] = getFloatOutput(floatInput);
    (*m_intStorage["int"])[out] = getOutput((*m_intStorage["int"])[in]) + m_outputOffset;
    (*m_intStorage["realInt"])[out] = (*m_intStorage["int"])[out] * m_toReal;;

    return t_offsetName.str();
  }

  // cppcheck-suppress constParameter
  TRGCDCJSignal const TRGCDCJLUT::operate(const TRGCDCJSignal& in, TRGCDCJSignal& out)
  {

    //cout<<"Operate start"<<endl;

    Belle2::TRGCDCJSignal t_offsetInput;
    // Offset input. (If needed)
    if (m_inputMin.getInt() != 0) {
      //t_offsetInput = in.offset(m_inputMin, m_inputMax);
      t_offsetInput = in.offset(m_inputMin);
    } else {
      t_offsetInput = in;
    }
    if (t_offsetInput.getMinInt() != 0) {
      cout << "[Warning] TRGCDCJLUT::operate() => Offsetted input minInt is not 0." << endl;
    }

    //cout<<"in"<<endl;
    //in.dump();
    ////cout<<"inMin"<<endl;
    ////m_inputMin.dump();
    ////cout<<"inMax"<<endl;
    ////m_inputMax.dump();
    //cout<<"m_inputMin"<<endl;
    //m_inputMin.dump();
    //cout<<"t_offsetInput"<<endl;
    //t_offsetInput.dump();

    // Set m_shiftOffsetInput.
    // [TODO] Change to constructor?
    m_shiftOffsetInput.setName("lut_" + out.getName() + "_in");
    // To remove warning of no m_commonData.
    m_shiftOffsetInput.setCommonData(t_offsetInput.getCommonData());

    // Shift input (If needed)
    string t_shiftOffsetInputCode = "";
    if (m_inputShiftBits > 0) m_shiftOffsetInput.assignTo(Belle2::TRGCDCJSignal::toSlv(t_offsetInput.shift(m_inputShiftBits, 0)),
                                                            t_offsetInput.calFinishClock(), t_shiftOffsetInputCode);
    else m_shiftOffsetInput.assignTo(Belle2::TRGCDCJSignal::toSlv(t_offsetInput), t_offsetInput.calFinishClock(),
                                       t_shiftOffsetInputCode);
    m_shiftOffsetInput.setPrintVhdl(in.getPrintVhdl());
    m_shiftOffsetInput.setVhdlOutputFile(in.getVhdlOutputFile());

    //cout<<"m_inputShiftBits"<<endl;
    //cout<<m_inputShiftBits<<endl;
    //cout<<"m_offsetShiftInput"<<endl;
    //m_shiftOffsetInput.dump();

    // Get output of LUT and invOffset it.
    signed long long intOutput = getOutput(m_shiftOffsetInput.getInt());
    //signed long long intOutput = getOutput(in.getInt());
    int t_finishClock;
    // Create offsetted output signal.
    double t_actual = getFloatOutput(in.getActual()) - m_shiftOutputMin.getActual();
    TRGCDCJSignalData* t_commonData = in.getCommonData();
    t_finishClock = in.getFinishClock() + 1;
    Belle2::TRGCDCJSignal t_offsetOutput(intOutput, m_shiftOutputMin.getToReal(), 0, m_shiftOffsetOutputMax.getInt(), t_actual, 0,
                                         m_shiftOffsetOutputMax.getActual(), t_finishClock, t_commonData, 1);
    t_offsetOutput.setName("lut_" + out.getName() + "_out");

    ////cout<<" JB:LUT::operate dump start"<<endl;
    //cout<<"t_offsetOutput"<<endl;
    //t_offsetOutput.dump();
    ////cout<<"min"<<endl;
    ////m_shiftOutputMin.dump();
    ////cout<<"max"<<endl;
    ////cout<<" JB:LUT::operate dump end"<<endl;
    //// Add offseted output signal with output min.

    //out <= t_offsetOutput.invOffset(m_shiftOutputMin,m_shiftOutputMin+m_shiftOffsetOutputMax);
    out <= t_offsetOutput.invOffset(m_shiftOutputMin);

    //cout<<"m_shiftOutputMin"<<endl;
    //m_shiftOutputMin.dump();
    //cout<<"out"<<endl;
    //out.dump();

    // Print Vhdl code.
    if (out.getPrintVhdl() == 1) {
      string t_finalCode = lutVhdlCode(t_shiftOffsetInputCode);
      out.getCommonData()->setVhdlOutProcess(out.getCommonData()->getVhdlOutProcess() + t_finalCode);
      // Fill signal map for definition.
      {
        map<string, vector<int> >& t_signals = out.getCommonData()->m_signals;
        if (!t_signals.count(t_offsetOutput.getName())) {
          t_signals[t_offsetOutput.getName()] = {t_offsetOutput.getType(), (int)t_offsetOutput.getBitsize()};
        }
      }
    }

    //cout<<"Operate end"<<endl;

    // LUT result value
    return t_offsetOutput;
  }

  std::string const TRGCDCJLUT::lutVhdlCode(string const& shiftOffsetInput) const
  {
    //cout<<shiftOffsetInput.getFinalCode()<<endl;
    //cout<<out.getFinalCode()<<endl;
    stringstream t_vhdlCode;
    t_vhdlCode << "lut_" << m_name << "_i: entity work.lut_" << m_name << endl;
    t_vhdlCode << "  port map(" << endl;
    t_vhdlCode << "    clka=>CLKIN, " << endl;
    t_vhdlCode << "    addra=>lut_" << m_name << "_in," << endl;
    t_vhdlCode << "    douta=>lut_" << m_name << "_out" << endl;
    t_vhdlCode << ");" << endl;
    t_vhdlCode << shiftOffsetInput << endl;
    return t_vhdlCode.str();
  }

}
