/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class for Signals in CDC Trigger for 3D tracker.
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#ifndef __EXTERNAL__
#include "trg/cdc/JSignal.h"
#include "trg/cdc/FpgaUtility.h"
#include "trg/cdc/JSignalData.h"
#else
#include "JSignal.h"
#include "FpgaUtility.h"
#include "JSignalData.h"
#endif
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>
//#include <algorithm>

using std::cout;
using std::endl;
using std::pair;
using std::make_pair;
using std::tuple;
using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::to_string;
using std::get;
using std::max;
using std::min;
using std::ofstream;
using std::ios;

namespace Belle2 {

  /// Constructor.
  TRGCDCJSignal::TRGCDCJSignal()
  {
    (*this) = TRGCDCJSignal(0);
  }

  TRGCDCJSignal::TRGCDCJSignal(TRGCDCJSignalData* commonData)
    : m_name(""),
      m_vhdlCode("")
  {
    m_type = 1;
    m_bitsize = -1;
    m_int = 0;
    m_minInt = 0;
    m_maxInt = 0;
    m_actual = 0;
    m_minActual = 0;
    m_maxActual = 0;
    m_toReal = 0;
    m_debug = 0;
    m_finishClock = -2;
    m_commonData = commonData;
  }

  TRGCDCJSignal::TRGCDCJSignal(int const& bitwidth, double const& value, double const& minValue, double const& maxValue,
                               int const& clock, TRGCDCJSignalData* commonData)
  {
    m_bitsize = bitwidth;
    m_type = calType(minValue);
    if (m_type == -1) {
      m_toReal = 1 / ((pow(2, bitwidth - 1) - 0.5) / max(maxValue, fabs(minValue)));
    } else if (m_type == 1) {
      m_toReal = 1 / ((pow(2, bitwidth) - 0.5) / maxValue);
    }
    m_int = FpgaUtility::roundInt(value / m_toReal);
    m_minInt = FpgaUtility::roundInt(minValue / m_toReal);
    m_maxInt = FpgaUtility::roundInt(maxValue / m_toReal);
    m_actual = value;
    m_minActual = minValue;
    m_maxActual = maxValue;
    m_debug = 0;
    m_name = "";
    m_vhdlCode = "";
    m_finishClock = clock;
    m_commonData = commonData;
    // Check if there is overflow or underflow.
    checkInt("TRGCDCJSignal(int, int, double, double)");
  }

  TRGCDCJSignal::TRGCDCJSignal(int const& bitwidth, double const& value, double const& minValue, double const& maxValue,
                               TRGCDCJSignalData* commonData)
  {
    (*this) = TRGCDCJSignal(bitwidth, value, minValue, maxValue, 0, commonData);
  }

  TRGCDCJSignal::TRGCDCJSignal(signed long long const& intValue, double const& toReal, signed long long const& minInt,
                               signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock,
                               TRGCDCJSignalData* commonData, bool b_slv)
  {
    m_int = intValue;
    m_minInt = minInt;
    m_maxInt = maxInt;
    m_actual = actual;
    m_minActual = minActual;
    m_maxActual = maxActual;
    m_toReal = toReal;
    if (b_slv == 0) m_type = calType();
    else m_type = 2;
    m_bitsize = calBitwidth();
    m_finishClock = finishClock;
    m_debug = 0;
    m_name = "";
    m_vhdlCode = "";
    m_commonData = commonData;
    // Check if there is overflow or underflow.
    checkInt("TRGCDCJSignal(int, double, signed long long, signed long long, double, double, double)");
  }

  TRGCDCJSignal::TRGCDCJSignal(double const& value, double const& toReal, TRGCDCJSignalData* commonData)
  {
    m_type = calType(value);
    m_toReal = toReal;
    m_int = FpgaUtility::roundInt(value / m_toReal);
    m_minInt = m_int;
    m_maxInt = m_int;
    m_actual = value;
    m_minActual = value;
    m_maxActual = value;
    m_bitsize = calBitwidth();
    m_debug = 0;
    m_name = "";
    m_vhdlCode = "";
    m_finishClock = -1;
    m_commonData = commonData;
    // Check if there is overflow or underflow.
    checkInt("TRGCDCJSignal(double, double)");
  }

  TRGCDCJSignal::TRGCDCJSignal(std::vector<bool> const& slvValue, int const& finishClock, TRGCDCJSignalData* commonData)
    : m_name("")
  {
    m_type = 2;
    m_bitsize = slvValue.size();
    m_slv = slvValue;
    m_int = 0;
    m_minInt = 0;
    m_maxInt = 0;
    m_actual = 0;
    m_minActual = 0;
    m_maxActual = 0;
    m_toReal = 0;
    m_debug = 0;
    m_vhdlCode = "";
    m_finishClock = finishClock;
    m_commonData = commonData;
  }


  TRGCDCJSignal& TRGCDCJSignal::assignTo(TRGCDCJSignal const& rhs, int targetClock, string& finalCode)
  {

    TRGCDCJSignal t_rhs(rhs);
    // Calculate result final clock.
    int t_calFinishClock = t_rhs.calFinishClock();
    // If t_rhs finish clock is blank or constant, skip.
    if (!(t_calFinishClock == -2 || t_calFinishClock == -1)) {
      // If target clock is not set, add 1 to t_rhs finish clock.
      if (targetClock == -3) t_rhs.m_finishClock = t_calFinishClock + 1;
      else {
        if (targetClock < t_calFinishClock) cout << "[Error] TRGCDCJSignal::assign() => targetClock is too small." << endl;
        else t_rhs.m_finishClock = targetClock;
      }
    }
    //if(m_name!="") cout<<"Signal name: "<<m_name<<" Clock: "<<t_rhs.m_finishClock<<endl;

    //// If Signal is not from blank constructor, or has a name.
    if ((*this).m_bitsize != -1 || (*this).m_name != "")
      //if((*this).m_bitsize != -1)
    {

      if ((*this).m_bitsize == 1) {
        // Check if rhs and lhs bitsize is the same.
        if ((*this).m_bitsize != t_rhs.m_bitsize) {
          cout << "[Warning] TRGCDCJSignal::operator<=() => rhs and lhs do not have same bitsize." << endl;
        }
        // Check if rhs and lhs is same type.
        if ((*this).m_type != t_rhs.m_type) {
          cout << "[Warning] TRGCDCJSignal::operator<=() => rhs and lhs do not have same type." << endl;
        }
        // Check if rhs and lhs clock is the same.
        if (m_finishClock != t_rhs.m_finishClock) {
          cout << "[Warning] TRGCDCJSignal::operator<=() => rhs and lhs do not have same clock." << endl;
          cout << m_name << " lhs clock: " << m_finishClock << " rhs clock: " << t_rhs.m_finishClock << endl;
        }
      }

      //int t_type;
      //if(t_rhs.m_minInt<0) t_type = -1;
      //else t_type = 1;
      //int t_bitsize = ceil(log(max(t_rhs.m_maxInt, abs(t_rhs.m_minInt))+1)/log(2));
      //if(t_type != 1) t_bitsize++;
      //// Check if bitwidth and calculated bitwidth match.
      //if(t_rhs.m_bitsize != t_bitsize){
      //  cout<<"Bitsize and calcualted bitwidth don't match"<<endl;
      //  cout<<"Bitsize: "<<t_rhs.m_bitsize<<" calBitsize: "<<t_bitsize<<endl;
      //  cout<<"LHS"<<endl;
      //  dump();
      //  cout<<"RHS"<<endl;
      //  t_rhs.dump();
      //  cout<<"Bitsize and calcualted bitwidth don't match"<<endl;
      //}
      //// Check if type and calculated type match.
      //if(t_rhs.m_type != t_type){
      //  cout<<"Type and calcualted type don't match"<<endl;
      //  cout<<"Type: "<<t_rhs.m_type<<" calType: "<<t_type<<endl;
      //  cout<<"LHS"<<endl;
      //  dump();
      //  cout<<"RHS"<<endl;
      //  t_rhs.dump();
      //  cout<<"Type and calcualted type don't match"<<endl;
      //}

      // Set arguments if needed.
      if (t_rhs.m_argumentSignals.size() == 0) t_rhs.initArgumentSignals();

      // Print m_vhdlCode.
      string t_finalCode(finalCode);

      if (getPrintVhdl() == 1) {
        finalCode = assignVhdlCode(*this, t_rhs);
        if (t_finalCode == "printAssignVhdlCode") {
          //printVhdl(finalCode);
          m_commonData->m_vhdlInProcess += finalCode + "\n";
        }
      }

    } // End of not blank constructor.

    swap(*this, t_rhs);
    // Operations due to swap.
    // Keep original name
    m_name = t_rhs.m_name;
    // Clear code.
    m_vhdlCode = "";
    m_argumentSignals.clear();

    // Don't propagate debug
    m_debug = 0;

    return *this;
  }

  TRGCDCJSignal& TRGCDCJSignal::assignTo(TRGCDCJSignal const& rhs, int targetClock)
  {
    string finalCode = "printAssignVhdlCode";
    return assignTo(rhs, targetClock, finalCode);
  }

  TRGCDCJSignal& TRGCDCJSignal::operator<= (TRGCDCJSignal const& rhs)
  {
    string finalCode = "printAssignVhdlCode";
    return assignTo(rhs, -3, finalCode);
  }

  /// Unary operator
  TRGCDCJSignal const TRGCDCJSignal::operator- () const
  {

    TRGCDCJSignal t_signal(*this);

    //if(t_signal.m_type==2) {
    //  cout<<"[Error] TRGCDCJSignal::operator-(void) => should not be used with slv type."<<endl;
    //}

    if (t_signal.m_argumentSignals.size() == 0) t_signal.initArgumentSignals();

    t_signal.m_int *= -1;
    t_signal.m_minInt = -(*this).m_maxInt;
    t_signal.m_maxInt = -(*this).m_minInt;
    t_signal.m_actual *= -1;
    t_signal.m_minActual = -m_maxActual;
    t_signal.m_maxActual = -m_minActual;
    if (m_type == 1) {
      t_signal.m_type = -1;
      t_signal.m_bitsize += 1;
    }

    //t_signal.m_vhdlCode = vhdlCode("-", *this, t_signal);
    if (getPrintVhdl() == 1) {
      vhdlCode("-", *this, t_signal, t_signal.m_vhdlCode);
    }

    return t_signal;
  }

  /// Arithmetic operator.
  TRGCDCJSignal const TRGCDCJSignal::operator+ (TRGCDCJSignal const& rhs) const
  {
    TRGCDCJSignal t_lhs(*this);
    TRGCDCJSignal t_rhs(rhs);

    //if(t_lhs.m_type==2 || t_rhs.m_type==2) {
    //  cout<<"[Error] TRGCDCJSignal::operator+() => should not be used with slv type."<<endl;
    //}

    if (t_lhs.m_argumentSignals.size() == 0) t_lhs.initArgumentSignals();
    if (t_rhs.m_argumentSignals.size() == 0) t_rhs.initArgumentSignals();

    if (m_debug == 1) {
      cout << "TRGCDCJSignal::operator+=()" << endl;
      cout << "<<<Input lhs>>>" << endl;
      t_lhs.dump();
      cout << "<<<Input lhs>>>" << endl;
      cout << "<<<Input rhs>>>" << endl;
      t_rhs.dump();
      cout << "<<<Input rhs>>>" << endl;
    }
    // Match m_toReal(unit) by shifting bits.
    matchUnit(t_lhs, t_rhs);

    // To save toReal, debug
    TRGCDCJSignal t_result(t_lhs);
    combineArguments(t_lhs, t_rhs, t_result);

    // Set name
    t_result.m_name = t_lhs.m_name + "+" + t_rhs.m_name;
    // Add values.
    t_result.m_int = t_lhs.m_int + t_rhs.m_int;
    t_result.m_minInt = t_lhs.m_minInt + t_rhs.m_minInt;
    t_result.m_maxInt = t_lhs.m_maxInt + t_rhs.m_maxInt;
    t_result.m_actual = t_lhs.m_actual + t_rhs.m_actual;
    t_result.m_minActual = t_lhs.m_minActual + t_rhs.m_minActual;
    t_result.m_maxActual = t_lhs.m_maxActual + t_rhs.m_maxActual;
    // Calculate bitsize.
    //int outputBitsize;
    //int outputType;
    //calTypeBitwidth(t_lhs, "+", t_rhs, outputType, outputBitsize);
    //Set output bitsize and type.
    //t_result.m_type = outputType;
    //t_result.m_bitsize = outputBitsize;
    t_result.m_type = t_result.calType();
    t_result.m_bitsize = t_result.calBitwidth();
    // Check for overflow underflow.
    t_result.checkInt("TRGCDCJSignal::operator+");
    if (m_debug == 1) {
      cout << "<<<Output>>>" << endl;
      t_result.dump();
      cout << "<<<Output>>>" << endl;
      cout << "TRGCDCJSignal::operator+=()" << endl;
    }

    //t_result.m_vhdlCode = vhdlCode(t_lhs, "+", t_rhs, t_result);
    if (getPrintVhdl() == 1) vhdlCode(t_lhs, "+", t_rhs, t_result, t_result.m_vhdlCode);

    return t_result;
  }

  TRGCDCJSignal const TRGCDCJSignal::operator- (TRGCDCJSignal const& rhs) const
  {

    TRGCDCJSignal t_lhs(*this);
    TRGCDCJSignal t_rhs(rhs);

    //if(t_lhs.m_type==2 || t_rhs.m_type==2) {
    //  cout<<"[Error] TRGCDCJSignal::operator-() => should not be used with slv type."<<endl;
    //}

    if (t_lhs.m_argumentSignals.size() == 0) t_lhs.initArgumentSignals();
    if (t_rhs.m_argumentSignals.size() == 0) t_rhs.initArgumentSignals();

    if (m_debug == 1) {
      cout << "TRGCDCJSignal::operator-=()" << endl;
      cout << "<<<Input lhs>>>" << endl;
      t_lhs.dump();
      cout << "<<<Input lhs>>>" << endl;
      cout << "<<<Input rhs>>>" << endl;
      t_rhs.dump();
      cout << "<<<Input rhs>>>" << endl;
    }
    // Match m_toReal(unit) by shifting bits.
    matchUnit(t_lhs, t_rhs);

    // To save toReal, debug
    TRGCDCJSignal t_result(t_lhs);
    combineArguments(t_lhs, t_rhs, t_result);
    // Set name
    t_result.m_name = t_lhs.m_name + "-" + t_rhs.m_name;
    // Subtract values.
    t_result.m_int = t_lhs.m_int - t_rhs.m_int;
    if (isSameSignal(t_lhs, t_rhs)) {
      t_result.m_minInt = 0;
      t_result.m_maxInt = 0;
      t_result.m_minActual = 0;
      t_result.m_maxActual = 0;
    } else {
      t_result.m_minInt = t_lhs.m_minInt - t_rhs.m_maxInt;
      t_result.m_maxInt = t_lhs.m_maxInt - t_rhs.m_minInt;
      t_result.m_minActual = t_lhs.m_minActual - t_rhs.m_maxActual;
      t_result.m_maxActual = t_lhs.m_maxActual - t_rhs.m_minActual;
    }
    t_result.m_actual = t_lhs.m_actual - t_rhs.m_actual;
    // Calculate bitsize.
    t_result.m_type = t_result.calType();
    t_result.m_bitsize = t_result.calBitwidth();
    // Check for overflow underflow.
    t_result.checkInt("TRGCDCJSignal::operator-");
    if (m_debug == 1) {
      cout << "<<<Output>>>" << endl;
      t_result.dump();
      cout << "<<<Output>>>" << endl;
      cout << "TRGCDCJSignal::operator-=()" << endl;
    }
    //t_result.m_vhdlCode = vhdlCode(t_lhs, "-", t_rhs, t_result);
    if (getPrintVhdl() == 1) vhdlCode(t_lhs, "-", t_rhs, t_result, t_result.m_vhdlCode);

    return t_result;
  }

  TRGCDCJSignal const TRGCDCJSignal::operator* (TRGCDCJSignal const& rhs) const
  {

    // Shift values if input bit size is larger then signed 25 bits * signed 18 bits (unsigned 24 bits * unsigned 17 bits)
    // Find largest and second bitsize.
    TRGCDCJSignal t_large(*this);
    TRGCDCJSignal t_small(rhs);

    //if(t_large.m_type==2 || t_small.m_type==2) {
    //  cout<<"[Error] TRGCDCJSignal::operator*() => should not be used with slv type."<<endl;
    //}

    if (t_large.m_argumentSignals.size() == 0) t_large.initArgumentSignals();
    if (t_small.m_argumentSignals.size() == 0) t_small.initArgumentSignals();

    if (m_debug == 1) {
      cout << "TRGCDCJSignal::operator*=()" << endl;
      cout << "<<<Input lhs>>>" << endl;
      t_large.dump();
      cout << "<<<Input lhs>>>" << endl;
      cout << "<<<Input rhs>>>" << endl;
      t_small.dump();
      cout << "<<<Input rhs>>>" << endl;
    }
    bool orderSwap = orderLargestBitsize(t_large, t_small);
    int nBitsLargeShift = 0;
    int nBitsSmallShift = 0;
    // Shift if needed.
    if (t_large.m_type == 1) {
      if (t_large.m_bitsize > 24) {
        nBitsLargeShift = t_large.m_bitsize - 24;
        t_large = t_large.shift(nBitsLargeShift, 0);
      }
    } else {
      if (t_large.m_bitsize > 25) {
        nBitsLargeShift = t_large.m_bitsize - 25;
        t_large = t_large.shift(nBitsLargeShift, 0);
      }
    }
    if (t_small.m_type == 1) {
      if (t_small.m_bitsize > 17) {
        nBitsSmallShift = t_small.m_bitsize - 17;
        t_small = t_small.shift(nBitsSmallShift, 0);
      }
    } else {
      if (t_small.m_bitsize > 18) {
        nBitsSmallShift = t_small.m_bitsize - 18;
        t_small = t_small.shift(nBitsSmallShift, 0);
      }
    }
    if (m_debug == 1) {
      cout << "<<<";
      if (orderSwap) cout << nBitsSmallShift << " bit right shifted input lhs>>>" << endl;
      else cout << nBitsLargeShift << " bit right shifted input lhs>>>" << endl;
      if (orderSwap) t_small.dump();
      else t_large.dump();
      cout << "<<<Shifted input lhs>>>" << endl;
      cout << "<<<";
      if (orderSwap) cout << nBitsLargeShift << " bit right shifted input rhs>>>" << endl;
      else cout << nBitsSmallShift << " bit right shifted input rhs>>>" << endl;
      if (orderSwap) t_large.dump();
      else t_small.dump();
      cout << "<<<Shifted input rhs>>>" << endl;
    }

    // To save debug
    TRGCDCJSignal t_result(t_large);
    combineArguments(t_large, t_small, t_result);
    // Set name
    t_result.m_name = t_large.m_name + "*" + t_small.m_name;
    // Multiply values.
    t_result.m_int = t_large.m_int * t_small.m_int;
    t_result.m_actual = t_large.m_actual * t_small.m_actual;

    if (isSameSignal(*this, rhs)) {
      if (t_large.m_minInt >= 0 && t_small.m_minInt >= 0) t_result.m_minInt = t_large.m_minInt * t_small.m_minInt;
      else t_result.m_minInt = 0;
      if (t_large.m_minActual >= 0 && t_small.m_minActual >= 0) t_result.m_minActual = t_large.m_minActual * t_small.m_minActual;
      else t_result.m_minActual = 0;
    } else {
      if (t_large.m_minInt >= 0 && t_small.m_minInt >= 0) t_result.m_minInt = t_large.m_minInt * t_small.m_minInt;
      else t_result.m_minInt = min(t_large.m_minInt * t_small.m_maxInt, t_large.m_maxInt * t_small.m_minInt);
      if (t_large.m_minActual >= 0 && t_small.m_minActual >= 0) t_result.m_minActual = t_large.m_minActual * t_small.m_minActual;
      else t_result.m_minActual = min(t_large.m_minActual * t_small.m_maxActual, t_large.m_maxActual * t_small.m_minActual);
    }
    t_result.m_maxInt = max(t_large.m_maxInt * t_small.m_maxInt, t_large.m_minInt * t_small.m_minInt);
    t_result.m_maxActual = max(t_large.m_maxActual * t_small.m_maxActual, t_large.m_minActual * t_small.m_minActual);

    // Set unit
    t_result.m_toReal = t_large.m_toReal * t_small.m_toReal;
    // Calculate bitsize and type.
    t_result.m_type = t_result.calType();
    t_result.m_bitsize = t_result.calBitwidth();
    // Check for overflow underflow.
    t_result.checkInt("TRGCDCJSignal::operator*");
    if (m_debug == 1) {
      cout << "<<<Output>>>" << endl;
      t_result.dump();
      cout << "<<<Output>>>" << endl;
      cout << "TRGCDCJSignal::operator*=()" << endl;
    }

    if (getPrintVhdl() == 1) vhdlCode(t_large, "*", t_small, t_result, t_result.m_vhdlCode);

    return t_result;
  }

  void TRGCDCJSignal::dump() const
  {
    cout << "-----------------------------------" << endl;
    cout << "name:            " << m_name << endl;
    cout << "type:            " << m_type << endl;
    cout << "bitsize:         " << m_bitsize << endl;
    int t_org = cout.precision();
    cout.precision(15);
    cout << "actual:          " << m_actual << endl;
    cout.precision(t_org);
    cout << "int:             " << m_int << endl;
    cout << "minInt:          " << m_minInt << endl;
    cout << "maxInt:          " << m_maxInt << endl;
    cout << "minActual:       " << m_minActual << endl;
    cout << "maxActual:       " << m_maxActual << endl;
    cout << "realInt:         " << m_int* m_toReal << endl;
    cout << "toReal:          " << m_toReal << endl;
    cout << "clock:           " << m_finishClock << endl;
    cout << "debug|printVhdl: " << m_debug << "|" << getPrintVhdl() << endl;
    cout << "-----------------------------------" << endl;
  }

  TRGCDCJSignal const TRGCDCJSignal::shift(int nBits, int operate) const
  {
    TRGCDCJSignal t_signal(*this);

    if (t_signal.m_argumentSignals.size() == 0) t_signal.initArgumentSignals();

    t_signal.m_bitsize -= nBits;
    t_signal.m_int = floor(this->m_int / pow(2, nBits));
    t_signal.m_minInt = floor(this->m_minInt / pow(2, nBits));
    t_signal.m_maxInt = floor(this->m_maxInt / pow(2, nBits));
    if (operate == 0) {
      t_signal.m_toReal = this->m_toReal * pow(2, nBits);
    } else {
      t_signal.m_actual = this->m_actual / pow(2, nBits);
      t_signal.m_minActual = this->m_minActual / pow(2, nBits);
      t_signal.m_maxActual = this->m_maxActual / pow(2, nBits);
    }

    t_signal.checkInt("TRGCDCJSignal::shift()");

    if (getPrintVhdl() == 1) {
      if (nBits < 0) {
        vhdlCode("shift_left", *this, t_signal, t_signal.m_vhdlCode);
      } else if (nBits != 0) {
        vhdlCode("shift_right", *this, t_signal, t_signal.m_vhdlCode);
      }
    }

    return t_signal;
  }

  TRGCDCJSignal const TRGCDCJSignal::offset(TRGCDCJSignal const& valueMin) const
  {
    TRGCDCJSignal t_offset = *this - valueMin;
    // Need to find minimum bitsize
    t_offset.m_bitsize = t_offset.calBitwidth();
    if (t_offset.calType() != 1) cout << "[Error] TRGCDCJSignal::offset() => t_offset min value below 0. " << endl;
    t_offset.m_type = 1;
    // Check if there is overflow or underflow
    t_offset.checkInt("TRGCDCJSignal::offset()");
    // Reset vhdlCode.
    if (getPrintVhdl() == 1) vhdlCode(*this, "-", valueMin, t_offset, t_offset.m_vhdlCode);
    return t_offset;
  }

  TRGCDCJSignal const TRGCDCJSignal::invOffset(TRGCDCJSignal const& valueMin) const
  {
    TRGCDCJSignal t_result = *this + valueMin;
    // Set bitsize and type.
    t_result.m_type = t_result.calType();
    t_result.m_bitsize = t_result.calBitwidth();

    // Check if there is overflow or underflow
    t_result.checkInt("TRGCDCJSignal::invOffset()");
    // Reset vhdlCode.
    if (getPrintVhdl() == 1) vhdlCode(*this, "+", valueMin, t_result, t_result.m_vhdlCode);
    return t_result;
  }


  void TRGCDCJSignal::choose(TRGCDCJSignal& target, TRGCDCJSignal const& targetMin, TRGCDCJSignal const& targetMax,
                             TRGCDCJSignal const& reference, std::vector<std::vector<TRGCDCJSignal> > data, int targetClock)
  {

    TRGCDCJSignal t_reference(reference);
    if (t_reference.m_argumentSignals.size() == 0) t_reference.initArgumentSignals();

    // Handle input errors.
    if (data.size() == 0 || data.size() == 1) {
      cout << "[Error] TRGCDCJSignal::choose() => There is not enough data." << endl;
      return;
    }
    if ((*(data.end() - 1)).size() != 1) {
      cout << "[Error] TRGCDCJSignal::choose() => Last data doesn't contain 1 TRGCDCJSignal." << endl;
      return;
    }

    // Calculate result final clock.
    int t_maxClock = t_reference.m_finishClock;
    for (unsigned iCondition = 0; iCondition < data.size(); iCondition++) {
      const TRGCDCJSignal* assign = &(data[iCondition][0]);
      if (t_maxClock < assign->m_finishClock) t_maxClock = assign->m_finishClock;
    }
    // Save final clock.
    if (targetClock == -3) target.m_finishClock = t_maxClock + 1;
    else {
      if (targetClock < t_maxClock) cout << "[Error] TRGCDCJSignal::choose() => targetClock is too small." << endl;
      else target.m_finishClock = targetClock;
    }

    // Change assignment black signal constructors to min and max.
    // Change assignment min, max (int, actual).
    signed long long t_minInt = targetMin.m_minInt;
    signed long long t_maxInt = targetMax.m_maxInt;
    double t_minActual = targetMin.m_minActual;
    double t_maxActual = targetMax.m_maxActual;
    for (unsigned iCondition = 0; iCondition < data.size(); iCondition++) {
      TRGCDCJSignal* from = 0;
      TRGCDCJSignal* to = 0;
      TRGCDCJSignal* assign = &(data[iCondition][0]);

      // To keep properties.
      TRGCDCJSignal t_assign(*assign);

      // To prevent target clock to become constant.
      //if(assign->m_minInt==assign->m_maxInt) assign->m_finishClock= target.m_finishClock-1;
      //else assign->m_finishClock= target.m_finishClock;
      // Change finish clock to target clock.
      // Init arguments.
      if (assign->m_argumentSignals.size() == 0) assign->initArgumentSignals();
      assign->m_finishClock = target.m_finishClock;

      // Change min, max (int, actual), type values.
      assign->m_minInt = t_minInt;
      assign->m_maxInt = t_maxInt;
      assign->m_minActual = t_minActual;
      assign->m_maxActual = t_maxActual;
      assign->m_type = assign->calType();
      assign->m_bitsize = assign->calBitwidth();
      // Calculated vhdlCode due to above changes.
      if (reference.getPrintVhdl() == 1) vhdlCode("none", t_assign, *assign, assign->m_vhdlCode);

      // For non-default
      if (data[iCondition].size() != 1) {
        // Change blank signal constructors to min and max.
        from = &(data[iCondition][1]);
        to = &(data[iCondition][2]);
        if (from->m_bitsize == -1) {
          (*from) = TRGCDCJSignal(t_reference.m_minInt, t_reference.m_toReal, t_reference.m_minInt, t_reference.m_minInt,
                                  t_reference.m_minActual, t_reference.m_minActual, t_reference.m_minActual, -1, t_reference.m_commonData);
        }
        if (to->m_bitsize == -1) {
          (*to) = TRGCDCJSignal(t_reference.m_maxInt, t_reference.m_toReal, t_reference.m_maxInt, t_reference.m_maxInt,
                                t_reference.m_maxActual, t_reference.m_maxActual, t_reference.m_maxActual, -1, t_reference.m_commonData);
        }
      }

      if (from) {
        if (from->m_argumentSignals.size() == 0) from->initArgumentSignals();
      }
      if (to) {
        if (to->m_argumentSignals.size() == 0) to->initArgumentSignals();
      }
    }
    // For integer case.
    for (unsigned iCondition = 0; iCondition < data.size(); iCondition++) {
      TRGCDCJSignal const* assign = &(data[iCondition][0]);
      // For default case.
      if (data[iCondition].size() == 1) {
        string t_string = "";
        target.assignTo(*assign, target.m_finishClock, t_string);
        // For non-default cases.
      } else {
        TRGCDCJSignal const* from = &(data[iCondition][1]);
        TRGCDCJSignal const* to = &(data[iCondition][2]);
        if (t_reference.m_int >= from->m_int && t_reference.m_int <= to->m_int) {
          string t_string = "";
          target.assignTo(*assign, target.m_finishClock, t_string);
          break;
        }
      }
    }
    // For actual case.
    for (unsigned iCondition = 0; iCondition < data.size(); iCondition++) {
      TRGCDCJSignal const* assign = &(data[iCondition][0]);
      // For default case.
      if (data[iCondition].size() == 1) {
        target.m_actual = (*assign).m_actual;
        // For non-default case.
      } else {
        TRGCDCJSignal const* from = &(data[iCondition][1]);
        TRGCDCJSignal const* to = &(data[iCondition][2]);
        assign = &(data[iCondition][0]);
        if (t_reference.m_actual >= from->m_actual && t_reference.m_actual <= to->m_actual) {
          target.m_actual = (*assign).m_actual;
          break;
        }
      }
    }

    target.checkInt("TRGCDCJSignal::choose()");

    // Print Vhdl code.
    if (reference.getPrintVhdl() == 1) {
      string t_finalCode = chooseVhdlCode(target, t_reference, data);
      target.m_commonData->m_vhdlInProcess += t_finalCode + "\n";
      //target.printVhdl(t_finalCode);
    }

  }

  void TRGCDCJSignal::choose(TRGCDCJSignal& target, TRGCDCJSignal const& targetMin, TRGCDCJSignal const& targetMax,
                             TRGCDCJSignal const& reference, const std::vector<std::vector<TRGCDCJSignal> >& data)
  {
    choose(target, targetMin, targetMax, reference, data, -3);
  }

  void TRGCDCJSignal::choose(TRGCDCJSignal& target, TRGCDCJSignal const& reference,
                             const std::vector<std::vector<TRGCDCJSignal> >& data,
                             int targetClock)
  {

    signed long long t_minInt;
    signed long long t_maxInt;
    double t_minActual;
    double t_maxActual;
    double t_toReal;
    // Find targetMin and targetMax between assignments.
    for (unsigned iCondition = 0; iCondition < data.size(); iCondition++) {
      const TRGCDCJSignal* assign = &(data[iCondition][0]);
      if (iCondition == 0) {
        t_minInt = assign->getMinInt();
        t_maxInt = assign->getMaxInt();
        t_minActual = assign->getMinActual();
        t_maxActual = assign->getMaxActual();
        t_toReal = assign->getToReal();
      } else {
        if (t_minInt > assign->getMinInt()) t_minInt = assign->getMinInt();
        if (t_maxInt < assign->getMaxInt()) t_maxInt = assign->getMaxInt();
        if (t_minActual > assign->getMinActual()) t_minActual = assign->getMinActual();
        if (t_maxActual < assign->getMaxActual()) t_maxActual = assign->getMaxActual();
        if (t_toReal != assign->getToReal()) cout << "[Error] TRGCDCJSignal::choose() => toReal are not same between assignments in data."
                                                    << endl;
      }
    }

    TRGCDCJSignal t_targetMax(t_maxInt, t_toReal, t_maxInt, t_maxInt, t_maxActual, t_maxActual, t_maxActual, -1, target.m_commonData);
    TRGCDCJSignal t_targetMin(t_minInt, t_toReal, t_minInt, t_minInt, t_minActual, t_minActual, t_minActual, -1, target.m_commonData);

    choose(target, t_targetMin, t_targetMax, reference, data, targetClock);
  }

  void TRGCDCJSignal::choose(TRGCDCJSignal& target, TRGCDCJSignal const& reference,
                             const std::vector<std::vector<TRGCDCJSignal> >& data)
  {
    choose(target, reference, data, -3);
  }


  void TRGCDCJSignal::ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > >& data,
                             int targetClock)
  {
    // Handle input errors.
    if (data.size() == 0) {
      cout << "[Error] TRGCDCJSignal::ifElse() => There is not enough data." << endl;
      return;
    }

    // Init arguments of data.
    // These errors prevent else case being in between data.
    // Loop over all cases.
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal const& comp = data[iCase].first;
      if (iCase != (data.size() - 1)) {
        if (comp.m_argumentSignals.size() == 0) cout << "[Error] TRGCDCJSignal::ifElse() => comp signal has no m_vhdl." << endl;
        if (comp.m_minInt != 0
            || comp.m_maxInt != 1) cout << "[Error] TRGCDCJSignal::ifElse() => comp signal m_minInt or m_maxInt is not correct." << endl;
      }
      // Loop over all assignments.
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
        //if(rhs.m_type==2) cout<<"[Error] TRGCDCJSignal::ifElse() => Cannot handle slv in assignments currently. Should be fixed later."<<endl;
        if (rhs.m_argumentSignals.size() == 0) rhs.initArgumentSignals();
      }
    }

    // Calculate final clock.
    // Loop over all cases.
    int t_maxClock = data[0].first.calFinishClock();
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      int t_clock = data[iCase].first.calFinishClock();
      if (t_maxClock < t_clock) t_maxClock = t_clock;
      // Loop over all assignments.
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal const& rhs = data[iCase].second[iAssignment].second;
        t_clock = rhs.calFinishClock();
        if (t_maxClock < t_clock) t_maxClock = t_clock;
      }
    }
    int t_finishClock = 0;
    if (targetClock == -3) {
      if (t_maxClock < 0) cout << "[Error] TRGCDCJSignal::ifElse() => t_maxClock is negative. Did not consider this case." << endl;
      t_finishClock = t_maxClock + 1;
    } else {
      if (targetClock < t_maxClock) cout << "[Error] TRGCDCJSignal::ifElse() => targetClock is too small." << endl;
      else t_finishClock = targetClock;
    }

    // Change rhs, comp clock to finish clock.
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal& comp = data[iCase].first;
      comp.m_finishClock = t_finishClock;
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
        rhs.m_finishClock = t_finishClock;
      }
    }

    // Find min and max ranges between all rhs.
    // <minInt, maxInt, minActual, maxActual>
    map<TRGCDCJSignal*, pair<vector<long long int>, vector<double> > > t_signalsRange;
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
        TRGCDCJSignal& lhs = *(data[iCase].second[iAssignment].first);
        if (t_signalsRange.find(&lhs) == t_signalsRange.end()) {
          vector<long long int> t_intRange = {rhs.m_minInt, rhs.m_maxInt};
          vector<double> t_actualRange = {rhs.m_minActual, rhs.m_maxActual};
          t_signalsRange[&lhs] = make_pair(t_intRange, t_actualRange);
        } else {
          long long int& t_minInt = t_signalsRange[&lhs].first[0];
          long long int& t_maxInt = t_signalsRange[&lhs].first[1];
          double& t_minActual = t_signalsRange[&lhs].second[0];
          double& t_maxActual = t_signalsRange[&lhs].second[1];
          if (t_minInt > rhs.m_minInt) t_minInt = rhs.m_minInt;
          if (t_maxInt < rhs.m_maxInt) t_maxInt = rhs.m_maxInt;
          if (t_minActual > rhs.m_minActual) t_minActual = rhs.m_minActual;
          if (t_maxActual < rhs.m_maxActual) t_maxActual = rhs.m_maxActual;
        }
      }
    }
    // Change min, max ranges and type, bitwidth for all rhs.
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
        TRGCDCJSignal& lhs = *(data[iCase].second[iAssignment].first);
        TRGCDCJSignal orgRhs(rhs);
        long long int& t_minInt = t_signalsRange[&lhs].first[0];
        long long int& t_maxInt = t_signalsRange[&lhs].first[1];
        double& t_minActual = t_signalsRange[&lhs].second[0];
        double& t_maxActual = t_signalsRange[&lhs].second[1];
        rhs.m_minInt = t_minInt;
        rhs.m_maxInt = t_maxInt;
        rhs.m_minActual = t_minActual;
        rhs.m_maxActual = t_maxActual;
        rhs.m_type = rhs.calType();
        rhs.m_bitsize = rhs.calBitwidth();
        if (rhs.getPrintVhdl() == 1) vhdlCode("none", orgRhs, rhs, rhs.m_vhdlCode);
      }
    }

    // For integer case.
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal const& comp = data[iCase].first;
      // If comp is 1 or else.
      if (comp.m_int == 1 || comp.m_bitsize == -1) {
        for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
          TRGCDCJSignal& lhs = *(data[iCase].second[iAssignment].first);
          TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
          string t_string = "";
          lhs.assignTo(rhs, t_finishClock, t_string);
          // Check ranges.
          lhs.checkInt("TRGCDCJSignal::ifElse()");
        }
        break;
      }
    }

    // For actual case.
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal const& comp = data[iCase].first;
      // If comp is 1 or else.
      if (comp.m_actual == 1 || comp.m_bitsize == -1) {
        for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
          TRGCDCJSignal& lhs = *(data[iCase].second[iAssignment].first);
          TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
          lhs.m_actual = rhs.m_actual;
        }
        break;
      }
    }

    // Print vhdl code.
    if (data[0].first.getPrintVhdl() == 1) {
      // Set m_commonData for all lhs signals.
      for (unsigned iCase = 0; iCase < data.size(); iCase++) {
        for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
          TRGCDCJSignal& lhs = *(data[iCase].second[iAssignment].first);
          /* cppcheck-suppress variableScope */
          TRGCDCJSignal& rhs = data[iCase].second[iAssignment].second;
          if (lhs.m_commonData == 0) lhs.m_commonData = rhs.m_commonData;
        }
      }
      string t_finalCode = ifElseVhdlCode(data);
      data[0].first.m_commonData->m_vhdlInProcess += t_finalCode + "\n";
    }


  }

  void TRGCDCJSignal::ifElse(std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > >& data)
  {
    ifElse(data, -3);
  }


  TRGCDCJSignal const TRGCDCJSignal::limit(signed long long minInt, signed long long maxInt, double minActual,
                                           double maxActual) const
  {
    TRGCDCJSignal t_result(*this);

    //if(t_result.m_type==2) {
    //  cout<<"[Error] TRGCDCJSignal::limit() => should not be used with slv type."<<endl;
    //}


    if (t_result.m_argumentSignals.size() == 0) t_result.initArgumentSignals();

    t_result.m_minInt = minInt;
    t_result.m_maxInt = maxInt;
    t_result.m_minActual = minActual;
    t_result.m_maxActual = maxActual;
    t_result.m_type = t_result.calType();
    t_result.m_bitsize = t_result.calBitwidth();
    //// Check if there is overflow or underflow.
    //t_result.checkInt("TRGCDCJSignal::limit()");


    if (getPrintVhdl() == 1) vhdlCode("none", *this, t_result, t_result.m_vhdlCode);
    //t_result.m_vhdlCode = "resize("+t_result.m_vhdlCode+","+to_string(t_result.m_bitsize)+")";
    return t_result;
  }

  TRGCDCJSignal const TRGCDCJSignal::limit(TRGCDCJSignal const& min, TRGCDCJSignal const& max) const
  {
    return limit(min.m_int, max.m_int, min.m_actual, max.m_actual);
  }

  TRGCDCJSignal const TRGCDCJSignal::resize(int bitwidth) const
  {
    if (this->m_bitsize == bitwidth) return (*this);

    TRGCDCJSignal t_result(*this);
    if (t_result.m_argumentSignals.size() == 0) t_result.initArgumentSignals();

    t_result.m_bitsize = bitwidth;
    // Check if there is overflow or underflow.
    t_result.checkInt("TRGCDCJSignal::resize()");


    if (getPrintVhdl() == 1) vhdlCode("resize", *this, t_result, t_result.m_vhdlCode);
    //t_result.m_vhdlCode = "resize("+t_result.m_vhdlCode+","+to_string(t_result.m_bitsize)+")";
    return t_result;
  }

  //TRGCDCJSignal const TRGCDCJSignal::toSigned() const {
  //  if(m_type==-1) return (*this);

  //  TRGCDCJSignal t_result(*this);
  //  if(t_result.m_argumentSignals.size() ==0) t_result.initArgumentSignals();

  //  t_result.m_type = -1;
  //  t_result.m_bitsize = m_bitsize+1;
  //  // Check if there is overflow or underflow.
  //  t_result.checkInt("TRGCDCJSignal::toSigned()");
  //

  //  if(getPrintVhdl()==1) vhdlCode("signed", *this, t_result, t_result.m_vhdlCode);
  //  //t_result.m_vhdlCode = "resize("+t_result.m_vhdlCode+","+to_string(t_result.m_bitsize)+")";
  //  return t_result;
  //}

  //TRGCDCJSignal const TRGCDCJSignal::toUnsigned() const {
  //  if(m_type==1) return (*this);
  //  TRGCDCJSignal t_result(*this);
  //  if(t_result.m_argumentSignals.size() ==0) t_result.initArgumentSignals();

  //  t_result.m_type = 1;
  //  t_result.m_bitsize = m_bitsize-1;
  //  // Check if there is overflow or underflow.
  //  t_result.checkInt("TRGCDCJSignal::toUnsigned()");
  //

  //  if(getPrintVhdl()==1) vhdlCode("unsigned", *this, t_result, t_result.m_vhdlCode);
  //  //t_result.m_vhdlCode = "resize("+t_result.m_vhdlCode+","+to_string(t_result.m_bitsize)+")";
  //  return t_result;
  //}


  TRGCDCJSignal const TRGCDCJSignal::slvToSigned(TRGCDCJSignal const& in, double const& toReal, signed long long const& minInt,
                                                 signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock)
  {

    vector<bool> const& slv = in.m_slv;

    // Check input
    if (in.m_type != 2) cout << "[Error] TRGCDCJSignal::slvToSigned() => type is not slv." << endl;
    if (slv.size() > 63) cout << "[Error] TRGCDCJSignal::slvToSigned() => bitwidth is too large." << endl;

    TRGCDCJSignal result(in);

    // Calculate value. [Index 0 is lsb]
    result.m_int = 0;
    for (unsigned iBit = 0; iBit < slv.size() - 1; iBit++) {
      result.m_int += slv[iBit] * pow(2, iBit);
    }
    result.m_int -= slv[slv.size() - 1] * pow(2, slv.size() - 1);
    // Set values.
    result.m_minInt = minInt;
    result.m_maxInt = maxInt;
    result.m_actual = actual;
    result.m_minActual = minActual;
    result.m_maxActual = maxActual;
    result.m_toReal = toReal;
    result.m_type = 0;
    result.m_type = result.calType();
    result.m_bitsize = result.calBitwidth();
    result.m_finishClock = finishClock;
    // Check if there is overflow or underflow.
    result.checkInt("slvToSigned");
    result.m_commonData = in.m_commonData;

    return result;
  }

  TRGCDCJSignal const TRGCDCJSignal::slvToSigned(TRGCDCJSignal const& in)
  {
    return slvToSigned(in, in.m_toReal, in.m_minInt, in.m_maxInt, in.m_actual, in.m_minActual, in.m_maxActual, in.m_finishClock);
  }

  TRGCDCJSignal const TRGCDCJSignal::slvToUnsigned(TRGCDCJSignal const& in, double const& toReal, signed long long const& minInt,
                                                   signed long long const& maxInt, double const& actual, double const& minActual, double const& maxActual, int const& finishClock)
  {

    vector<bool> const& slv = in.m_slv;

    // Check input
    if (in.m_type != 2) cout << "[Error] TRGCDCJSignal::slvToUnsigned() => type is not slv." << endl;
    if (slv.size() > 63) cout << "[Error] TRGCDCJSignal::slvToUnsigned() => bitwidth is too large." << endl;

    TRGCDCJSignal result(in);

    // Calculate value. [Index 0 is lsb]
    result.m_int = 0;
    for (unsigned iBit = 0; iBit < slv.size(); iBit++) {
      result.m_int += slv[iBit] * pow(2, iBit);
    }
    // Set values.
    result.m_minInt = minInt;
    result.m_maxInt = maxInt;
    result.m_actual = actual;
    result.m_minActual = minActual;
    result.m_maxActual = maxActual;
    result.m_toReal = toReal;
    result.m_type = 0;
    result.m_type = result.calType();
    result.m_bitsize = result.calBitwidth();
    result.m_finishClock = finishClock;
    // Check if there is overflow or underflow.
    result.checkInt("slvToSigned");
    result.m_commonData = in.m_commonData;

    return result;
  }

  TRGCDCJSignal const TRGCDCJSignal::slvToUnsigned(TRGCDCJSignal const& in)
  {
    return slvToUnsigned(in, in.m_toReal, in.m_minInt, in.m_maxInt, in.m_actual, in.m_minActual, in.m_maxActual, in.m_finishClock);
  }

  TRGCDCJSignal const TRGCDCJSignal::toSlv(TRGCDCJSignal const& in, int /*test*/)
  {

    TRGCDCJSignal result(in);

    // Temporary signal.
    //test += 0; //TODO why?
    signed long long t_int = in.m_int;
    if (in.m_type == -1) {
      // Change signed m_int to vector<bool>
      t_int = ~t_int;
      t_int += 1;
      result.m_slv.clear();
      result.m_slv.resize(in.m_bitsize);
      for (int iBit = 0; iBit < in.m_bitsize; iBit++) {
        result.m_slv[iBit] = (bool)(t_int >> iBit) & 1;
      }
    } else if (in.m_type == 1) {
      // Change unsigned m_int to vector<bool>
      result.m_slv.clear();
      result.m_slv.resize(in.m_bitsize);
      for (int iBit = 0; iBit < in.m_bitsize; iBit++) {
        result.m_slv[iBit] = (bool)(t_int >> iBit) & 1;
      }
    } else {
      cout << "[Error] TRGCDCJSignal::toSlv() => m_type is not 1 or -1." << endl;
    }

    return result;
  }

  TRGCDCJSignal const TRGCDCJSignal::toSlv(TRGCDCJSignal const& in)
  {
    if (in.m_type == 2) return (in);
    // Don't think I need below but will keep it for now.
    if (in.m_type == -1) {
      cout << "[Error] TRGCDCJSignal::toSlv() => Slv is not same as unsigned anymore." << endl;
    }
    TRGCDCJSignal t_result(in);
    t_result.m_type = 2;

    if (in.getPrintVhdl() == 1) vhdlCode("std_logic_vector", in, t_result, t_result.m_vhdlCode);

    return t_result;
  }



  TRGCDCJSignal TRGCDCJSignal::comp(TRGCDCJSignal const& lhs, const std::string& operate, TRGCDCJSignal const& rhs)
  {
    // Make copies for method.
    TRGCDCJSignal t_lhs(lhs);
    TRGCDCJSignal t_rhs(rhs);
    TRGCDCJSignal t_result(t_lhs);
    // Make arguments.
    if (t_lhs.m_argumentSignals.size() == 0) t_lhs.initArgumentSignals();
    if (t_rhs.m_argumentSignals.size() == 0) t_rhs.initArgumentSignals();
    combineArguments(t_lhs, t_rhs, t_result);
    // Int Operation.
    bool t_bool = false;
    if (operate == "=") {
      t_bool = (t_lhs.m_int == t_rhs.m_int ? 1 : 0);
    } else if (operate == "/=") {
      t_bool = (t_lhs.m_int != t_rhs.m_int ? 1 : 0);
    } else if (operate == "<") {
      t_bool = (t_lhs.m_int < t_rhs.m_int ? 1 : 0);
    } else if (operate == "<=") {
      t_bool = (t_lhs.m_int <= t_rhs.m_int ? 1 : 0);
    } else if (operate == ">") {
      t_bool = (t_lhs.m_int > t_rhs.m_int ? 1 : 0);
    } else if (operate == ">=") {
      t_bool = (t_lhs.m_int >= t_rhs.m_int ? 1 : 0);
    } else if (operate == "and") {
      t_bool = (t_lhs.m_int && t_rhs.m_int ? 1 : 0);
    } else if (operate == "or") {
      t_bool = (t_lhs.m_int || t_rhs.m_int ? 1 : 0);
    } else {
      cout << "[Error] TRGCDCJSignal::comp() => No operator named " << operate << "." << endl;
    }
    // Actual Operation.
    if (operate == "=") {
      t_result.m_actual = (t_lhs.m_actual == t_rhs.m_actual ? 1 : 0);
    } else if (operate == "/=") {
      t_result.m_actual = (t_lhs.m_actual != t_rhs.m_actual ? 1 : 0);
    } else if (operate == "<") {
      t_result.m_actual = (t_lhs.m_actual < t_rhs.m_actual ? 1 : 0);
    } else if (operate == "<=") {
      t_result.m_actual = (t_lhs.m_actual <= t_rhs.m_actual ? 1 : 0);
    } else if (operate == ">") {
      t_result.m_actual = (t_lhs.m_actual > t_rhs.m_actual ? 1 : 0);
    } else if (operate == ">=") {
      t_result.m_actual = (t_lhs.m_actual >= t_rhs.m_actual ? 1 : 0);
    } else if (operate == "and") {
      t_result.m_actual = (t_lhs.m_actual && t_rhs.m_actual ? 1 : 0);
    } else if (operate == "or") {
      t_result.m_actual = (t_lhs.m_actual || t_rhs.m_actual ? 1 : 0);
    } else {
      cout << "[Error] TRGCDCJSignal::comp() => No operator named " << operate << "." << endl;
    }
    // Change values.
    t_result.m_name = t_lhs.m_name + operate + t_rhs.m_name;
    t_result.m_type = 1;
    t_result.m_bitsize = 1;
    t_result.m_minInt = 0;
    t_result.m_maxInt = 1;
    t_result.m_minActual = 0;
    t_result.m_maxActual = 1;
    t_result.m_toReal = 1;
    if (t_bool == 1) {
      t_result.m_int = 1;
    } else {
      t_result.m_int = 0;
    }

    if (t_result.getPrintVhdl() == 1) vhdlCode(t_lhs, operate, t_rhs, t_result, t_result.m_vhdlCode);

    return t_result;

  }

  void TRGCDCJSignal::swap(TRGCDCJSignal& first, TRGCDCJSignal& second) const
  {
    std::swap(first.m_name, second.m_name);
    std::swap(first.m_type, second.m_type);
    std::swap(first.m_bitsize, second.m_bitsize);
    std::swap(first.m_int, second.m_int);
    std::swap(first.m_minInt, second.m_minInt);
    std::swap(first.m_maxInt, second.m_maxInt);
    std::swap(first.m_actual, second.m_actual);
    std::swap(first.m_minActual, second.m_minActual);
    std::swap(first.m_maxActual, second.m_maxActual);
    std::swap(first.m_toReal, second.m_toReal);
    std::swap(first.m_debug, second.m_debug);
    std::swap(first.m_vhdlCode, second.m_vhdlCode);
    std::swap(first.m_argumentSignals, second.m_argumentSignals);
    std::swap(first.m_finishClock, second.m_finishClock);
    std::swap(first.m_commonData, second.m_commonData);
  }

  void TRGCDCJSignal::matchUnit(TRGCDCJSignal& first, TRGCDCJSignal& second) const
  {
    if (first.m_toReal != second.m_toReal) {
      // Find number of bits to shift to match unit.
      double t_nBits = log(first.m_toReal / second.m_toReal) / log(2);
      // Round  to integer
      if (t_nBits > 0) {
        t_nBits = int(t_nBits + 0.5);
      } else {
        t_nBits = int(t_nBits - 0.5);
      }
      // Shift bits to match unit
      if (t_nBits > 0) {
        first = first.shift(-t_nBits, 0);
        //cout<<"mu::shift_left(resize(" + first.m_name + "," + to_string(int(first.m_bitsize+t_nBits)) + ")," + to_string(int(t_nBits)) + ")"<<endl;
        //cout<<first.m_vhdlCode<<endl;
      } else if (t_nBits < 0) {
        second = second.shift(t_nBits, 0);
        //cout<<"mu::shift_left(resize(" + second.m_name + "," + to_string(int(second.m_bitsize-t_nBits)) + ")," + to_string(int(-t_nBits)) + ")"<<endl;
        //cout<<second.m_vhdlCode<<endl;
      }

      if (fabs(first.m_toReal - second.m_toReal) > 0.00001 * first.m_toReal) {
        cout << "[Warning] TRGCDCJSignal::matchUnit() => in1 and in2 units are still different." << endl;
        cout << first.m_toReal << " " << second.m_toReal << endl;
      }

      if (m_debug == 1) {
        cout << "TRGCDCJSignal::matchUnit()" << endl;
        if (t_nBits > 0) cout << "lhs was shifted to left by " << t_nBits << endl;
        else if (t_nBits < 0) cout << "rhs was shifted to left by " << -t_nBits << endl;
        cout << "TRGCDCJSignal::matchUnit()" << endl;
      }
    } // End of if units are different.
    first.checkInt("TRGCDCJSignal::matchUnit()::first");
    second.checkInt("TRGCDCJSignal::matchUnit()::second");
  }

  void TRGCDCJSignal::setName(const string& name)
  {
    m_name = name;
  }

  void TRGCDCJSignal::setInt(signed long long value)
  {
    m_int = value;
  }

  void TRGCDCJSignal::setActual(double value)
  {
    m_actual = value;
  }

  void TRGCDCJSignal::setToReal(double value)
  {
    m_toReal = value;
  }

  void TRGCDCJSignal::setType(int type)
  {
    m_type = type;
  }

  void TRGCDCJSignal::setBitsize(int bitsize)
  {
    m_bitsize = bitsize;

    // Check if there is overflow or underflow.
    checkInt("TRGCDCJSignal::setBitsize()");
  }

  void TRGCDCJSignal::setFinishClock(int finishClock)
  {
    m_finishClock = finishClock;
  }

  void TRGCDCJSignal::setDebug(bool debug)
  {
    m_debug = debug;
  }

  void TRGCDCJSignal::setPrintVhdl(bool printVhdl)
  {
    if (m_commonData) m_commonData->m_printVhdl = printVhdl;
    else {
      cout << "[Error] TRGCDCJSignal::setPrintVhdl() => m_commonData does not exist." << endl;
    }
  }

  void TRGCDCJSignal::setVhdlOutputFile(const string& filename)
  {
    if (m_commonData) m_commonData->m_vhdlOutputFile = filename;
    else {
      cout << "[Error] TRGCDCJSignal::setVhdlOutputFile() => m_commonData does not exist." << endl;
    }
  }

  void TRGCDCJSignal::setCommonData(TRGCDCJSignalData* commonData)
  {
    m_commonData = commonData;
  }

  string TRGCDCJSignal::getName() const
  {
    return m_name;
  }

  int TRGCDCJSignal::getType() const
  {
    return m_type;
  }

  double TRGCDCJSignal::getBitsize() const
  {
    return m_bitsize;
  }

  signed long long TRGCDCJSignal::getInt() const
  {
    return m_int;
  }

  signed long long TRGCDCJSignal::getMinInt() const
  {
    return m_minInt;
  }

  signed long long TRGCDCJSignal::getMaxInt() const
  {
    return m_maxInt;
  }

  double TRGCDCJSignal::getActual() const
  {
    return m_actual;
  }

  double TRGCDCJSignal::getMinActual() const
  {
    return m_minActual;
  }

  double TRGCDCJSignal::getMaxActual() const
  {
    return m_maxActual;
  }

  double TRGCDCJSignal::getRealInt() const
  {
    return m_int * m_toReal;
  }

  double TRGCDCJSignal::getRealMinInt() const
  {
    return m_minInt * m_toReal;
  }

  double TRGCDCJSignal::getRealMaxInt() const
  {
    return m_maxInt * m_toReal;
  }

  double TRGCDCJSignal::getToReal() const
  {
    return m_toReal;
  }

  int TRGCDCJSignal::getFinishClock() const
  {
    return m_finishClock;
  }

  std::vector<std::pair<std::string, vector<int> > > const TRGCDCJSignal::getArgumentSignals() const
  {
    return m_argumentSignals;
  }

  TRGCDCJSignalData* TRGCDCJSignal::getCommonData() const
  {
    if (m_commonData) return m_commonData;
    else {
      cout << "[Error] TRGCDCJSignal::getCommonData() => m_commonData does not exist." << endl;
      return 0;
    }
  }


  int TRGCDCJSignal::calType() const
  {
    //if(m_type==2) cout<<"[Error] TRGCDCJSignal::calType() => m_type should not be slv."<<endl;
    if (m_minInt < 0) return -1;
    else return 1;
  }

  int TRGCDCJSignal::calType(double minValue) const
  {
    //if(m_type==2) cout<<"[Error] TRGCDCJSignal::calType() => m_type should not be slv."<<endl;
    if (minValue < 0) return -1;
    else return 1;
  }

  int TRGCDCJSignal::calBitwidth() const
  {
    //int t_bitwidth = ceil(log(max(m_maxInt, llabs(m_minInt))+1)/log(2));
    int t_bitwidth = ceil(log1p(max(m_maxInt, llabs(m_minInt))) / log(2));
    if (calType() != 1) t_bitwidth++;
    return t_bitwidth;
  }

  int TRGCDCJSignal::calBitwidth(int type, signed long long minInt, signed long long maxInt)
  {
    //int t_bitwidth = ceil(log(max(maxInt, llabs(minInt))+1)/log(2));
    int t_bitwidth = ceil(log1p(max(maxInt, llabs(minInt))) / log(2));
    if (type != 1) t_bitwidth++;
    return t_bitwidth;
  }

  int TRGCDCJSignal::calFinishClock() const
  {
    // Combined signal?
    if (m_argumentSignals.size() != 0) {
      int maxClock = -1;
      // Find max clock between signals.
      for (unsigned iSignals = 0; iSignals < m_argumentSignals.size(); iSignals++) {
        if (maxClock < m_argumentSignals[iSignals].second[2]) maxClock = m_argumentSignals[iSignals].second[2];
      }
      return maxClock;
    } else {
      return m_finishClock;
    }
  }

  string TRGCDCJSignal::getVhdlCode() const
  {
    return m_vhdlCode;
  }

  bool TRGCDCJSignal::getPrintVhdl() const
  {
    if (m_commonData) return m_commonData->m_printVhdl;
    else {
      cout << "[Error] TRGCDCJSignal::getPrintVhdl() => m_commonData does not exist." << endl;
      return 0;
    }
  }

  string TRGCDCJSignal::getVhdlOutputFile() const
  {
    if (m_commonData) return m_commonData->m_vhdlOutputFile;
    else {
      cout << "[Error] TRGCDCJSignal::getVhdlOutputFile() => m_commonData does not exist." << endl;
      return "";
    }
  }

  bool TRGCDCJSignal::orderLargestBitsize(TRGCDCJSignal& large, TRGCDCJSignal& small) const
  {
    bool result = 0;
    if (large.m_bitsize < small.m_bitsize) {
      swap(large, small);
      result = 1;
    } else if (large.m_bitsize == small.m_bitsize) {
      if (large.m_type == 1) {;}
      else if (large.m_type == -1 && small.m_type == 1) {
        swap(large, small);
        result = 1;
      } else if (large.m_type == 0 && small.m_type != 0) {
        swap(large, small);
        result = 1;
      }
    }

    return result;
  }

  signed long long TRGCDCJSignal::calMaxInteger() const
  {
    signed long long t_maxValue;
    if (calType() == 1) {
      t_maxValue = pow(2, m_bitsize) - 1;
    } else if (calType() == -1) {
      t_maxValue = pow(2, m_bitsize - 1) - 1;
    } else {
      t_maxValue = 0;
    }
    return t_maxValue;
  }

  signed long long TRGCDCJSignal::calMinInteger() const
  {
    //if(m_type==2) cout<<"[Error] TRGCDCJSignal::calMinInteger() => m_type should not be slv"<<endl;
    signed long long t_minValue;
    if (m_type == 1) {
      t_minValue = 0;
    } else if (m_type == -1) {
      t_minValue = -pow(2, m_bitsize - 1) + 1;
    } else {
      t_minValue = -pow(2, m_bitsize - 1) + 1;
    }
    return t_minValue;
  }

  TRGCDCJSignal const TRGCDCJSignal::absolute(TRGCDCJSignal const& first)
  {

    TRGCDCJSignal result(first);

    //if(result.m_type==2) cout<<"[Error] TRGCDCJSignal::absolute() => m_type should not be slv"<<endl;

    if (result.m_argumentSignals.size() == 0) result.initArgumentSignals();

    if (result.m_type != 1) {
      result.m_type = 1;
      result.m_int = llabs(first.m_int);
      if (first.m_minInt < 0 && first.m_maxInt > 0) result.m_minInt = 0;
      else result.m_minInt = min(llabs(first.m_maxInt), llabs(first.m_minInt));
      result.m_maxInt = max(llabs(first.m_maxInt), llabs(first.m_minInt));
      result.m_actual = fabs(first.m_actual);
      if (first.m_minActual < 0 && first.m_maxActual > 0) result.m_minActual = 0;
      else result.m_minActual = min(fabs(first.m_maxActual), fabs(first.m_minActual));
      result.m_maxActual = max(fabs(first.m_maxActual), fabs(first.m_minActual));
      result.m_bitsize = first.m_bitsize - 1;
    }

    result.checkInt("TRGCDCJSignal::absolute()");

    // For vhdl code.
    if (first.getPrintVhdl() == 1) vhdlCode("abs", first, result, result.m_vhdlCode);

    return result;

  }

  //void TRGCDCJSignal::calTypeBitwidth(TRGCDCJSignal const & lhs, string operation, TRGCDCJSignal const & rhs, int & type, int & bitwidth) const{
  //  int outputBitsize;
  //  int outputType;
  //  if(operation=="+"){
  //    // Find largest and second bitsize.
  //    TRGCDCJSignal t_large(lhs);
  //    TRGCDCJSignal t_small(rhs);
  //    orderLargestBitsize(t_large, t_small);
  //    // Calculate.
  //    if(t_large.m_type == 1) {
  //      if(t_small.m_type == 1) {outputBitsize = t_large.m_bitsize + 1; outputType = 1;}
  //      else if(t_small.m_type == -1) {outputBitsize = t_large.m_bitsize + 2; outputType = -1;}
  //      else {outputBitsize = t_large.m_bitsize + 1; outputType = -1;}
  //    } else if(t_large.m_type == -1) {
  //      outputBitsize = t_large.m_bitsize + 1; outputType = -1;
  //    } else {
  //      if(t_small.m_type == 1) {outputBitsize = t_large.m_bitsize; outputType = -1;}
  //      else if(t_small.m_type == -1) {outputBitsize = t_large.m_bitsize + 1; outputType = -1;}
  //      else {outputBitsize = t_large.m_bitsize + 1; outputType = 0;}
  //    }
  //  } else if(operation=="-"){
  //    // Find largest and second bitsize.
  //    TRGCDCJSignal t_large(lhs);
  //    TRGCDCJSignal t_small(rhs);
  //    orderLargestBitsize(t_large, t_small);
  //    // Calculate.
  //    if(t_large.m_type == 1) {
  //      if(t_small.m_type == 1) {outputBitsize = t_large.m_bitsize + 1; outputType = -1;}
  //      else if(t_small.m_type == -1) {outputBitsize = t_large.m_bitsize + 2; outputType = -1;}
  //      else {
  //        if(t_large.m_type == lhs.m_type){
  //          outputBitsize = t_large.m_bitsize + 1; outputType = 1;
  //        } else {
  //          outputBitsize = t_large.m_bitsize + 1; outputType = 0;
  //        }
  //      }
  //    } else if(t_large.m_type == -1) {
  //      outputBitsize = t_large.m_bitsize + 1; outputType = -1;
  //    } else {
  //      if(t_small.m_type == 1) {
  //        if(t_large.m_type == lhs.m_type){
  //          outputBitsize = t_large.m_bitsize + 1; outputType = 0;
  //        } else {
  //          outputBitsize = t_large.m_bitsize + 1; outputType = 1;
  //        }
  //      }
  //      else if(t_small.m_type == -1) {outputBitsize = t_large.m_bitsize + 1; outputType = -1;}
  //      else {outputBitsize = t_large.m_bitsize; outputType = -1;}
  //    }
  //  } else if(operation=="*"){
  //    TRGCDCJSignal t_large(lhs);
  //    TRGCDCJSignal t_small(rhs);
  //    orderLargestBitsize(t_large, t_small);
  //    if(t_large.m_type == 1 || t_small.m_type == 1) {
  //      outputBitsize = t_large.m_bitsize + t_small.m_bitsize;
  //      if(t_large.m_type == 1) outputType = t_small.m_type;
  //      else outputType = t_large.m_type;
  //    } else if(t_large.m_type == 0 && t_small.m_type == 0) {
  //      outputBitsize = t_large.m_bitsize + t_small.m_bitsize - 2;
  //      outputType = 1;
  //    } else {
  //      outputBitsize = t_large.m_bitsize + t_small.m_bitsize - 1;
  //      outputType = -1;
  //    }
  //  }
  //  type = outputType;
  //  bitwidth = outputBitsize;
  //}

  signed long long TRGCDCJSignal::calInt(double value, TRGCDCJSignal const& mother)
  {
    return (signed long long) FpgaUtility::roundInt(value / mother.m_toReal);
  }

  void TRGCDCJSignal::calVhdlTypeBitwidth(TRGCDCJSignal const& first, const std::string& operation, TRGCDCJSignal const& second,
                                          int& type,
                                          int& bitwidth)
  {
    if (first.m_type != second.m_type) {
      cout << "[Error] TRGCDCJSignal::calVhdlTypeBitwidth() => first and second type are different." << endl;
      cout << first.m_name << " " << second.m_name << endl;
      cout << first.m_type << " " << second.m_type << endl;
      return;
    }
    int firstBitwidth = first.m_bitsize;
    int secondBitwidth = second.m_bitsize;

    if (operation == "+" || operation == "-") {
      if (first.m_type != 1 || second.m_type != 1) {
        type = -1;
        bitwidth = max(firstBitwidth, secondBitwidth);
      } else {
        type = 1;
        bitwidth = max(first.m_bitsize, second.m_bitsize);
      }
    } else if (operation == "*") {
      if (first.m_type != 1 || second.m_type != 1) {
        type = -1;
        bitwidth = firstBitwidth + secondBitwidth;
      } else {
        type = 1;
        bitwidth = first.m_bitsize + second.m_bitsize;
      }
    } else if (operation == "=" || operation == "/=" || operation == "<" || operation == "<=" ||
               operation == ">" || operation == ">=" || operation == "and" || operation == "or") {
      type = 1;
      bitwidth = 1;
    } else {
      cout << "[Error] TRGCDCJSignal::calVhdlTypeBitwidth() => No operation named " << operation << "." << endl;
    }
  }

  void TRGCDCJSignal::initArgumentSignals()
  {
    string t_name;
    if (m_argumentSignals.size() == 0) {
      if (m_minInt == m_maxInt && m_type != 2) {
        if (m_type == 1) {
          //t_name="to_unsigned("+to_string(m_int)+","+to_string(m_bitsize)+")";
          t_name = "decimal_string_to_unsigned(\"" + to_string(m_int) + "\"," + to_string(m_bitsize) + ")";
        } else {
          //t_name="to_signed("+to_string(m_int)+","+to_string(m_bitsize)+")";
          t_name = "decimal_string_to_signed(\"" + to_string(m_int) + "\"," + to_string(m_bitsize) + ")";
        }
        // For non-constant constructor signal.
      } else if (m_name == "") {
        t_name = "INPUT(" + to_string(m_int) + ")";
        // For normal signals.
      } else {
        t_name = m_name;
      }
      //m_argumentSignals.push_back(make_pair(t_name,m_finishClock));
      vector<int> t_argument = {m_type, m_bitsize, m_finishClock};
      m_argumentSignals.push_back(make_pair(t_name, t_argument));
    } else {
      cout << "[Error] TRGCDCJSignal::initArgumentSignals() => m_argumentSignals.size() is not 0." << endl;
    }
  }

  void TRGCDCJSignal::combineArguments(TRGCDCJSignal const& first, TRGCDCJSignal const& second, TRGCDCJSignal& result)
  {
    result.m_argumentSignals.clear();
    result.m_argumentSignals.insert(result.m_argumentSignals.end(), first.m_argumentSignals.begin(), first.m_argumentSignals.end());
    result.m_argumentSignals.insert(result.m_argumentSignals.end(), second.m_argumentSignals.begin(), second.m_argumentSignals.end());
  }

  void TRGCDCJSignal::initVhdlCode()
  {
    if (m_vhdlCode != "") {
      cout << "[Error] TRGCDCJSignal::initVhdlCode() => m_vhdlCode is not \"\"." << endl;
    } else {
      m_vhdlCode = " ";
    }
  }

  void TRGCDCJSignal::toSignedVhdlCode()
  {
    if (m_type == -1) return;
    // Find if bitwidth is optimized for signed or unsigned. t_bitwidth is optimized for signed.
    int t_bitwidth = calBitwidth();
    if (calType() == 1) t_bitwidth++;

    if (t_bitwidth <= m_bitsize) {
      m_vhdlCode = "signed(" + m_vhdlCode + ")";
    } else if (t_bitwidth - 1 == m_bitsize) {
      m_vhdlCode = "signed('0'&(" + m_vhdlCode + "))";
      m_bitsize += 1;
    } else {
      cout << "[Error] TRGCDCJSignal::toSignedVhdlCode() => m_bitsize is too small." << endl;
    }
    m_type = -1;
  }

  void TRGCDCJSignal::toUnsignedVhdlCode()
  {
    if (m_type == 1) return;

    if (calType() == -1) {
      cout << "[Warning] TRGCDCJSignal::toUnsignedVhdlCode() => signal can be minus." << endl;
    } else {
      m_vhdlCode = "unsigned(" + m_vhdlCode + ")";
      m_type = 1;
    }

  }

  void TRGCDCJSignal::vhdlCode(TRGCDCJSignal const& first, const std::string& operation, TRGCDCJSignal const& second,
                               TRGCDCJSignal const& result, string& resultVhdlCode)
  {
    // Copy first and second for VHDL version.
    TRGCDCJSignal t_first(first);
    TRGCDCJSignal t_second(second);
    // Set m_vhdlCode if needed. (Will require clock implementation later)
    if (t_first.m_vhdlCode == "") {
      t_first.initVhdlCode();
    }
    if (t_second.m_vhdlCode == "") {
      t_second.initVhdlCode();
    }

    // Match types.
    // Changed to signed.
    if (t_first.m_type * t_second.m_type < 0 || (t_first.m_type != -1 && result.m_type == -1)) {
      if (t_first.m_type != -1) {
        t_first.toSignedVhdlCode();
      }
      if (t_second.m_type != -1) {
        t_second.toSignedVhdlCode();
      }
    }
    // Change to unsigned.
    if (t_first.m_type == 2) t_first.toUnsignedVhdlCode();
    if (t_second.m_type == 2) t_second.toUnsignedVhdlCode();

    // Calculate result type and bitwidth.
    int logicType, logicBitwidth, vhdlType, vhdlBitwidth;
    logicType = result.m_type;
    logicBitwidth = result.m_bitsize;
    calVhdlTypeBitwidth(t_first, operation, t_second, vhdlType, vhdlBitwidth);

    // Make code.
    string resultCode;
    // For compare operations.
    if (operation == "=" || operation == "/=" || operation == "<" || operation == "<=" ||
        operation == ">" || operation == ">=" || operation == "and" || operation == "or") {
      resultCode = "(" + t_first.m_vhdlCode + operation + t_second.m_vhdlCode + ")";
    } else {
      // For arithmetic operations.
      // If special case.
      if (logicType > 0 && vhdlType == -1) {
        if (logicBitwidth + 1 > vhdlBitwidth) t_first.m_vhdlCode = "resize(" + t_first.m_vhdlCode + "," + to_string(
              logicBitwidth + 1) + ")";
        resultCode = t_first.m_vhdlCode + operation + t_second.m_vhdlCode;
        resultCode = "resize(unsigned(" + resultCode + ")," + to_string(logicBitwidth) + ")";
      } else {
        // Normal case.
        if (logicBitwidth > vhdlBitwidth) {
          t_first.m_vhdlCode = "resize(" + t_first.m_vhdlCode + "," + to_string(logicBitwidth) + ")";
        }
        resultCode = t_first.m_vhdlCode + operation + t_second.m_vhdlCode;
        // Resize output to minimum bitwidth.
        if (logicBitwidth < vhdlBitwidth) {
          resultCode = "resize(" + resultCode + "," + to_string(logicBitwidth) + ")";
        }
      }
    }
    resultVhdlCode = resultCode;
  }

  void TRGCDCJSignal::vhdlCode(const std::string& operation, TRGCDCJSignal const& first, TRGCDCJSignal const& result,
                               string& resultVhdlCode)
  {
    // Copy first for VHDL version.
    TRGCDCJSignal t_first(first);
    // Set m_vhdlCode if needed. (Will require clock implementation later)
    if (t_first.m_vhdlCode == "") {
      t_first.initVhdlCode();
    }
    // Match types
    // Change VHDL unsigned to VHDL signed if required.
    if (t_first.m_type != -1 && result.m_type == -1) {
      t_first.toSignedVhdlCode();
    }
    // Change to unsigned if slv.
    if (t_first.m_type == 2) t_first.toUnsignedVhdlCode();

    // Calculate result type and bitwidth.
    int logicType, logicBitwidth, vhdlType, vhdlBitwidth;
    logicType = result.m_type;
    logicBitwidth = result.m_bitsize;
    // Will always be same as first type and bitsize.
    vhdlType = t_first.m_type;
    vhdlBitwidth = t_first.m_bitsize;

    // Make code.
    string resultCode;
    // If logic(out) is unsigned and vhdl(in) is signed.
    if (logicType == 1 && vhdlType == -1) {
      if (logicBitwidth + 1 > vhdlBitwidth) t_first.m_vhdlCode = "resize(" + t_first.m_vhdlCode + "," + to_string(
            logicBitwidth + 1) + ")";
      if (operation == "shift_left"
          || operation == "shift_right") resultCode = operation + "(" + t_first.m_vhdlCode + "," + to_string(abs(
                                                          first.m_bitsize - result.m_bitsize)) + ")";
      else if (operation == "unsigned" || operation == "none") resultCode = t_first.m_vhdlCode;
      else resultCode = operation + "(" + t_first.m_vhdlCode + ")";
      resultCode = "resize(unsigned(" + resultCode + ")," + to_string(logicBitwidth) + ")";
    } else {
      // Normal case.
      if (logicBitwidth > vhdlBitwidth) {
        t_first.m_vhdlCode = "resize(" + t_first.m_vhdlCode + "," + to_string(logicBitwidth) + ")";
      }
      if (operation == "shift_left"
          || operation == "shift_right") resultCode = operation + "(" + t_first.m_vhdlCode + "," + to_string(abs(
                                                          first.m_bitsize - result.m_bitsize)) + ")";
      else if (operation == "resize" || operation == "signed" || operation == "none") resultCode = t_first.m_vhdlCode;
      else resultCode = operation + "(" + t_first.m_vhdlCode + ")";
      // Resize output to minimum bitwidth.
      if (logicBitwidth < vhdlBitwidth) {
        resultCode = "resize(" + resultCode + "," + to_string(logicBitwidth) + ")";
      }
    }
    resultVhdlCode = resultCode;
  }

  std::string TRGCDCJSignal::replaceWithSignalNames(std::string const& fromVhdlCode,
                                                    std::vector<pair<std::string, vector<int> > > const& fromArgumentSignals, int const& finishClock,
                                                    std::map<std::string, vector<int> >& buffers)
  {
    // Replace names with token(" ") in from.m_vhdlCode
    string t_vhdlCode = "";
    size_t start = 0;
    size_t end = fromVhdlCode.find(" ");
    int nIndex = (int)fromArgumentSignals.size();
    int index = 0;
    while (end != string::npos) {
      // Check if there are enough signal names.
      if (nIndex - 1 < index) {
        cout << "[Error] TRGCDCJSignal::replaceWithSignalNames() => Not enough signal names in m_argumentSignals." << endl;
        return "";
      }
      // Replace token(" ") with name.
      //cout<<"Name: "<<fromArgumentSignals[index].first<<" Clock: "<<fromArgumentSignals[index].second[2]<<endl;
      int t_buffer = (finishClock - 1) - fromArgumentSignals[index].second[2] - 1;
      // No buffer needed or instant assignment or constant.
      if (t_buffer == -1 || t_buffer == -2
          || fromArgumentSignals[index].second[2] == -1) t_vhdlCode += fromVhdlCode.substr(start,
                end - start) + fromArgumentSignals[index].first;
      else if (t_buffer > -1) {
        t_vhdlCode += fromVhdlCode.substr(start, end - start) + fromArgumentSignals[index].first + "_b(" + to_string(t_buffer) + ")";
        // Fill map for buffers.
        if (buffers.find(fromArgumentSignals[index].first) == buffers.end()) {
          buffers[fromArgumentSignals[index].first] = {fromArgumentSignals[index].second[0], fromArgumentSignals[index].second[1], t_buffer};
        } else {
          if (buffers[fromArgumentSignals[index].first][2] < t_buffer) buffers[fromArgumentSignals[index].first][2] = t_buffer;
        }
      } else {
        cout << "[Error] TRGCDCJSignal::replaceWithSignalNames() => finishClock is too small." << endl;
        t_vhdlCode += fromVhdlCode.substr(start, end - start) + fromArgumentSignals[index].first;
      }

      index++;
      start = end + 1;
      end = fromVhdlCode.find(" ", start);
    }
    t_vhdlCode += fromVhdlCode.substr(start, end - start);
    // Check if number of spaces and signal names are same.
    if (nIndex != index) {
      cout << "[Error] TRGCDCJSignal::replaceWithSignalNames() => Number of signal names and number of " " do not match." << endl;
      return "";
    }
    return t_vhdlCode;
  }

  std::string TRGCDCJSignal::assignVhdlCode(TRGCDCJSignal const& target, TRGCDCJSignal const& from)
  {

    // Check if target has m_commonData.
    if (target.m_commonData == 0) {
      cout << "[Error] TRGCDCJSignal::assignVhdlCode() => target.m_commonData is not set." << endl;
      return "";
    }

    TRGCDCJSignal t_from(from);

    // Set m_vhdlCode if needed.
    if (t_from.m_vhdlCode == "") {
      t_from.initVhdlCode();
    }

    // Fill signal map for definition.
    {
      if (target.m_name == "") cout << "[Error] TRGCDCJSignal::assignVhdlCode() => target.m_name is \"\" " << endl;
      map<string, vector<int> >& t_signals = target.m_commonData->m_signals;
      if (t_signals.find(target.m_name) == t_signals.end()) {
        //t_signals[target.m_name] = {target.m_type, target.m_bitsize};
        // From and target should be same.
        t_signals[target.m_name] = {from.m_type, from.m_bitsize};
      }
    }

    // Replace names with spaces in from.m_vhdlCode
    string t_vhdlCode = replaceWithSignalNames(t_from.m_vhdlCode, t_from.m_argumentSignals, t_from.m_finishClock,
                                               t_from.m_commonData->m_buffers);

    return target.m_name + "<=" + t_vhdlCode + ";";
    //// For signal <= combined signal.
    //if(t_vhdlCode != "") {
    //  return target.m_name + "<=" + t_vhdlCode + ";";
    //// For signal <= constant. Do we need a seperate flag?
    //} else if (from.m_minInt == from.m_maxInt) {
    //  if(from.m_type == 1) {
    //    return target.m_name + "<=to_unsigned(" + to_string(from.m_int) + "," + to_string(from.m_bitsize) + ")";
    //  } else {
    //    return target.m_name + "<=to_signed(" + to_string(from.m_int) + "," + to_string(from.m_bitsize) + ")";
    //  }
    //// For input signals. [Not exactly sure why this works.]
    //} else if (from.m_name=="") {
    //  if(from.m_type == 1) {
    //    return target.m_name + "<=to_unsigned(" + to_string(from.m_int) + "," + to_string(from.m_bitsize) + ")";
    //  } else {
    //    return target.m_name + "<=to_signed(" + to_string(from.m_int) + "," + to_string(from.m_bitsize) + ")";
    //  }
    //} else {
    //// For signal <= single signal.
    //  return target.m_name + "<=" + from.m_name + ";";
    //}
  }

  std::string TRGCDCJSignal::chooseVhdlCode(TRGCDCJSignal const& target, TRGCDCJSignal const& reference,
                                            std::vector<std::vector<TRGCDCJSignal> > const& data)
  {
    // Check if target has m_name.
    if (target.m_name == "") {
      cout << "[Error] TRGCDCJSignal::chooseVhdlCode() => target has no name." << endl;
      return "";
    }
    // Check if reference has m_name or vhdlCode.
    if (reference.m_name == "" && reference.m_vhdlCode == "") {
      cout << "[Error] TRGCDCJSignal::cooseVhdlCode() => reference has no name or vhdlCode." << endl;
      return "";
    }

    TRGCDCJSignal t_reference(reference);

    // Set m_vhdlCode if needed.
    if (t_reference.m_vhdlCode == "") t_reference.initVhdlCode();
    // Replace names with spaces in reference.m_vhdlCode
    string t_referenceVhdlCode = replaceWithSignalNames(t_reference.m_vhdlCode, t_reference.m_argumentSignals, target.m_finishClock,
                                                        t_reference.m_commonData->m_buffers);

    // Storage for used values.
    map<signed long long, bool> t_used;
    // Create string for choice().
    stringstream t_vhdlCode;
    //t_vhdlCode<<"case to_integer("<<t_referenceVhdlCode<<") is"<<endl;
    //for(unsigned iCondition=0; iCondition<data.size()-1; iCondition++){
    //  TRGCDCJSignal const & from = data[iCondition][1];
    //  TRGCDCJSignal const & to = data[iCondition][2];
    //  TRGCDCJSignal const & assign = data[iCondition][0];
    //  signed long long fromInt = from.m_int;
    //  signed long long toInt = to.m_int;
    //  // Don't allow same int numbers for from and to.
    //  if(t_used.count(fromInt)!=0) fromInt += 1;
    //  else t_used[fromInt] = 1;
    //  if(t_used.count(toInt)!=0) {
    //    // Allow same from and to when in one line.
    //    if(fromInt!=toInt) toInt -= 1;
    //  } else t_used[toInt] = 1;
    //  t_vhdlCode<<"  when "<<fromInt<<" to "<<toInt<<"=>"<<assignVhdlCode(target,assign)<<endl;;
    //}
    //t_vhdlCode<<"  when others=>"<<assignVhdlCode(target,(*(data.end()-1))[0])<<endl;
    //t_vhdlCode<<"end case;";

    // For if
    TRGCDCJSignal from = data[0][1];
    TRGCDCJSignal to = data[0][2];
    TRGCDCJSignal assign = data[0][0];
    if (from.m_vhdlCode == "") from.initVhdlCode();
    if (to.m_vhdlCode == "") to.initVhdlCode();
    string t_fromVhdlCode = replaceWithSignalNames(from.m_vhdlCode, from.m_argumentSignals, target.m_finishClock,
                                                   target.m_commonData->m_buffers);
    string t_toVhdlCode = replaceWithSignalNames(to.m_vhdlCode, to.m_argumentSignals, target.m_finishClock,
                                                 target.m_commonData->m_buffers);
    string t_fromPrefixSigned;
    string t_fromPostfixSigned;
    string t_toPrefixSigned;
    string t_toPostfixSigned;
    if (reference.m_type == -1 && from.m_type == 1) {
      t_fromPrefixSigned = "signed('0'&";
      t_fromPostfixSigned = ")";
    } else {
      t_fromPrefixSigned = "";
      t_fromPostfixSigned = "";
    }
    if (reference.m_type == -1 && to.m_type == 1) {
      t_toPrefixSigned = "signed('0'&";
      t_toPostfixSigned = ")";
    } else {
      t_toPrefixSigned = "";
      t_toPostfixSigned = "";
    }
    t_vhdlCode << "if (" << t_referenceVhdlCode << " >= " << t_fromPrefixSigned << t_fromVhdlCode << t_fromPostfixSigned << ") and (" <<
               t_referenceVhdlCode << " <= " << t_toPrefixSigned << t_toVhdlCode << t_toPostfixSigned << ") then" << endl;
    t_vhdlCode << "  " << assignVhdlCode(target, assign) << endl;

    // For if else
    for (unsigned iCondition = 1; iCondition < data.size() - 1; iCondition++) {
      TRGCDCJSignal from1 = data[iCondition][1];
      TRGCDCJSignal to1 = data[iCondition][2];
      TRGCDCJSignal assign1 = data[iCondition][0];
      if (from1.m_vhdlCode == "") from1.initVhdlCode();
      if (to1.m_vhdlCode == "") to1.initVhdlCode();
      if (reference.m_type == -1 && from1.m_type == 1) {
        t_fromPrefixSigned = "signed('0'&";
        t_fromPostfixSigned = ")";
      } else {
        t_fromPrefixSigned = "";
        t_fromPostfixSigned = "";
      }
      if (reference.m_type == -1 && to1.m_type == 1) {
        t_toPrefixSigned = "signed('0'&";
        t_toPostfixSigned = ")";
      } else {
        t_toPrefixSigned = "";
        t_toPostfixSigned = "";
      }
      t_fromVhdlCode = replaceWithSignalNames(from1.m_vhdlCode, from1.m_argumentSignals, target.m_finishClock,
                                              target.m_commonData->m_buffers);
      t_toVhdlCode = replaceWithSignalNames(to1.m_vhdlCode, to1.m_argumentSignals, target.m_finishClock, target.m_commonData->m_buffers);
      t_vhdlCode << "elsif (" << t_referenceVhdlCode << " >= " << t_fromPrefixSigned << t_fromVhdlCode << t_fromPostfixSigned << ") and ("
                 << t_referenceVhdlCode << " <= " << t_toPrefixSigned << t_toVhdlCode << t_toPostfixSigned << ") then" << endl;
      t_vhdlCode << "  " << assignVhdlCode(target, assign1) << endl;
    }
    t_vhdlCode << "else" << endl;
    t_vhdlCode << "  " << assignVhdlCode(target, (*(data.end() - 1))[0]) << endl;
    t_vhdlCode << "end if;";
    return t_vhdlCode.str();
  }

  std::string TRGCDCJSignal::ifElseVhdlCode(
    std::vector<std::pair<TRGCDCJSignal, std::vector<std::pair<TRGCDCJSignal*, TRGCDCJSignal> > > > const& data)
  {
    // Check if comp and lhs have name
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal const& comp = data[iCase].first;
      if (iCase != data.size() - 1) {
        if (comp.m_name == "") {
          cout << "[Error] TRGCDCJSignal::ifElseVhdlCode() => comp has no name." << endl;
          return "";
        }
      }
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal const& lhs = *(data[iCase].second[iAssignment].first);
        if (lhs.m_name == "") {
          cout << "[Error] TRGCDCJSignal::ifElseVhdlCode() => lhs has no name." << endl;
          return "";
        }
      }
    }

    // Write vhdl
    // Else will be last in data.
    stringstream t_vhdlCode;
    for (unsigned iCase = 0; iCase < data.size(); iCase++) {
      TRGCDCJSignal const& comp = data[iCase].first;
      string t_compVhdlCode = replaceWithSignalNames(comp.m_vhdlCode, comp.m_argumentSignals, comp.m_finishClock,
                                                     comp.m_commonData->m_buffers);
      // Choose which if to use.
      if (iCase == 0) t_vhdlCode << "if " << t_compVhdlCode << " then" << endl;
      else if (comp.m_bitsize == -1) t_vhdlCode << "else" << endl;
      else t_vhdlCode << "elsif " << t_compVhdlCode << " then" << endl;
      for (unsigned iAssignment = 0; iAssignment < data[iCase].second.size(); iAssignment++) {
        TRGCDCJSignal const& lhs = *(data[iCase].second[iAssignment].first);
        TRGCDCJSignal const& rhs = data[iCase].second[iAssignment].second;
        t_vhdlCode << "  " << assignVhdlCode(lhs, rhs) << endl;
      }
    }
    t_vhdlCode << "end if;";

    return t_vhdlCode.str();

  }

  void TRGCDCJSignal::printVhdl(const std::string& vhdlCode) const
  {
    if (m_commonData) {
      ofstream outFile;
      outFile.open(m_commonData->m_vhdlOutputFile, ios::app);
      if (outFile.is_open()) {
        outFile << vhdlCode << endl;
        outFile.close();
      }
    } else {
      cout << "[Error] TRGCDCJSignal::printVhdl() => m_commonData doesn't exist." << endl;
    }
  }

  void TRGCDCJSignal::checkInt(const std::string& name) const
  {
    if (m_int > calMaxInteger()) {
      cout << "[Error] " << name << " => m_int overflow" << endl;
      cout << " Start dumping" << endl;
      dump();
      cout << " End dumping" << endl;
    }
    if (m_int < calMinInteger()) {
      cout << "[Error] " << name << " => m_int underflow" << endl;
      cout << " Start dumping" << endl;
      dump();
      cout << " End dumping" << endl;
    }
    if (m_maxInt > calMaxInteger()) {
      cout << "[Error] " << name << " => m_maxInt overflow" << endl;
      cout << " Start dumping" << endl;
      dump();
      cout << " End dumping" << endl;
    }
    if (m_minInt < calMinInteger()) {
      cout << "[Error] " << name << " => m_minInt underflow" << endl;
      cout << " Start dumping" << endl;
      dump();
      cout << " End dumping" << endl;
    }
  }

  bool TRGCDCJSignal::isSameSignal(TRGCDCJSignal const& lhs, TRGCDCJSignal const& rhs)
  {
    return lhs.m_int == rhs.m_int && lhs.m_minInt == rhs.m_minInt && lhs.m_maxInt == rhs.m_maxInt
           && lhs.m_toReal == rhs.m_toReal && lhs.m_actual == rhs.m_actual && lhs.m_minActual == rhs.m_minActual
           && lhs.m_maxActual == rhs.m_maxActual;
  }

  void TRGCDCJSignal::valuesToMapSignals(std::vector<std::tuple<string, double, int, double, double, int> > const& inValues,
                                         Belle2::TRGCDCJSignalData* inCommonData, std::map<std::string, Belle2::TRGCDCJSignal>& outMap)
  {
    for (unsigned iSignals = 0; iSignals < inValues.size(); iSignals++) {
      string const& t_name = get<0>(inValues[iSignals]);
      double const& t_value = get<1>(inValues[iSignals]);
      int const& t_bitWidth = get<2>(inValues[iSignals]);
      double const& t_min = get<3>(inValues[iSignals]);
      double const& t_max = get<4>(inValues[iSignals]);
      int const& t_clock = get<5>(inValues[iSignals]);
      // Put input to signal.
      // Input to finish clock is 0.
      // Signals that will be used will have finish clock 1 due to below line.
      outMap[t_name] <= Belle2::TRGCDCJSignal(t_bitWidth, t_value, t_min, t_max, t_clock, inCommonData);
    }
  }

  void TRGCDCJSignal::mapSignalsToValues(std::map<std::string, Belle2::TRGCDCJSignal> const& inMap,
                                         std::vector<std::pair<std::string, int> > const& inChoose,
                                         std::vector<std::tuple<string, double, int, double, double, int> >& outValues)
  {
    for (unsigned iSignals = 0; iSignals < inChoose.size(); iSignals++) {
      int const& b_integerEffect = inChoose[iSignals].second;
      string const& t_name = inChoose[iSignals].first;
      Belle2::TRGCDCJSignal const& t_signal = inMap.find(t_name)->second;
      double t_value = 0, t_min = 0, t_max = 0;
      int t_bitwidth = t_signal.getBitsize();
      if (b_integerEffect == 1) {
        t_value = t_signal.getRealInt();
        t_min = t_signal.getRealMinInt();
        t_max = t_signal.getRealMaxInt();
      } else {
        t_value = t_signal.getActual();
        t_min = t_signal.getMinActual();
        t_max = t_signal.getMaxActual();
      }
      int t_clock = t_signal.getFinishClock();
      tuple<string, double, int, double, double, int> outValue(t_name, t_value, t_bitwidth, t_min, t_max, t_clock);
      outValues.push_back(outValue);
    }
  }

}
