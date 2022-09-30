
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef NEUROTRIGGERPARAMETERS_H
#define NEUROTRIGGERPARAMETERS_H
#pragma once
#include <iostream>
#include <TObject.h>

//#include <string>
//#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
//#include <framework/datastore/StoreArray.h>
//#include <framework/datastore/StoreObjPtr.h>
//#include <framework/database/DBObjPtr.h>
//#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
//#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
//#include <framework/dataobjects/BinnedEventT0.h>

namespace Belle2 {


  /** Class to represent a complete set to describe a Neurotrigger.
   */
  template <class T>
  class NNTParam : public TObject {
    /** this is a typewrapper class for the parameters for the neurotrigger. for every parameter, it can also store the information wether the parameter was aleady set or if it is locked.
     * */
  public:
    NNTParam() {}
    NNTParam(const T& xvalue)
    {
      m_value = xvalue;
      m_set = true;
    }
    operator T() const { return m_value;}
    T operator= (const T t)
    {
      if (!m_locked) {
        m_value = t;
        m_set = true;
      } else {
        throw std::invalid_argument("parameter is read only! (locked)");
      }
    }
    NNTParam operator()() const {return m_value;}


    bool isSet() {return m_set;}
    void lock() {m_locked = true;}
    bool isLocked() {return m_locked;}

  private:
    T m_value;
    bool m_set = false;
    bool m_locked = false;
  };

  class NeuroTriggerParameters : public TObject {
  public:
    NeuroTriggerParameters::NeuroTriggerParameters() {};
    NeuroTriggerParameters::NeuroTriggerParameters(std::string filename);
    virtual ~NeuroTriggerParameters() {};
    int to_intTiming(std::string& text)
    {
      if (text == "fastestpriority") {return 0;}
      else if (text == "fastest2d") {return 1;}
      else if (text == "etf") {return 2;}
      else if (text == "etf_or_fastestpriority") {return 3;}
      else if (text == "etf_or_fastest2d") {return 4;}
      else if (text == "etf_only") {return 5;}
      else {return -1;}
    }
    std::string to_strTiming(unsigned& i)
    {
      if (i == 0) {return "fastestpriority";}
      else if (i == 1) {return "fastest2d";}
      else if (i == 2) {return "etf";}
      else if (i == 3) {return "etf_or_fastestpriority";}
      else if (i == 4) {return "etf_or_fastest2d";}
      else if (i == 5) {return "etf_only";}
      else {return "invalid";}
    }
    std::string et_option() {return to_strTiming(ETOption);}
    void loadconfigtxt(std::string& filename);
    void saveconfigtxt(std::string& filename);
    void loadconfigroot(std::string& filename);
    void saveconfigroot(std::string& filename);
  private:
    /** Network parameters **/
    NNTParam<unsigned> nInput;
    NNTParam<unsigned> nHidden;
    NNTParam<unsigned> nOutput;
    NNTParam<unsigned> nMLP;
    NNTParam<bool> targetZ;
    NNTParam<bool> targetTheta;
    NNTParam<unsigned> tMax;
    //multiplynhidden was removed, because it is not used
    std::vector<std::vector<NNTParam<float>>> phiRangeUse;
    std::vector<std::vector<NNTParam<float>>> thetaRangeUse;
    std::vector<std::vector<NNTParam<float>>> invptRangeUse;
    std::vector<std::vector<NNTParam<float>>> phiRangeTrain;
    std::vector<std::vector<NNTParam<float>>> thetaRangeTrain;
    std::vector<std::vector<NNTParam<float>>> invptRangeTrain;
    std::vector<NNTParam<unsigned short>> maxHitsperSL;
    std::vector<NNTParam<float>>
                              std::vector<NNTParam<float>>
                              std::vector<NNTParam<float>> outputScale;
    std::vector<NNTParam<unsigned long>> SLPattern;
    std::vector<NNTParam<unsigned long>> SLPatternMask;
    NNTParam<unsigned> ETOption;
    std::vector<NNTParam<unsigned>> precision;

  };
}


