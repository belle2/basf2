
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
  class NNTParam {
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
      return *this;
    }
    NNTParam operator()() const {return m_value;}


    bool isSet() const {return m_set;}
    void lock() {m_locked = true;}
    bool isLocked() const {return m_locked;}

  private:
    T m_value;
    bool m_set = false;
    bool m_locked = false;
  };

  class NeuroTriggerParameters : public TObject {
  public:
    NeuroTriggerParameters() {};
    NeuroTriggerParameters(std::string& filename);
    virtual ~NeuroTriggerParameters() {};
    // unfortunately i am to dumb to make the typewrapper class work for strings,
    // so the workaround are those translate functions
    int to_intTiming(const std::string& text) const
    {
      if (text == "fastestpriority") {return 0;}
      else if (text == "fastest2d") {return 1;}
      else if (text == "etf") {return 2;}
      else if (text == "etf_or_fastestpriority") {return 3;}
      else if (text == "etf_or_fastest2d") {return 4;}
      else if (text == "etf_only") {return 5;}
      else if (text == "etfcc") {return 6;}
      else if (text == "etfhwin") {return 7;}
      else if (text == "etfcc_or_fastestpriority") {return 8;}
      else if (text == "min_etf_fastestpriority") {return 9;}
      else if (text == "min_etfcc_fastestpriority") {return 10;}
      else {return -1;}
    }
    const std::string to_strTiming(const unsigned& i) const
    {
      if (i == 0) {return "fastestpriority";}
      else if (i == 1) {return "fastest2d";}
      else if (i == 2) {return "etf";}
      else if (i == 3) {return "etf_or_fastestpriority";}
      else if (i == 4) {return "etf_or_fastest2d";}
      else if (i == 5) {return "etf_only";}
      else if (i == 6) {return "etfcc";}
      else if (i == 7) {return "etfhwin";}
      else if (i == 8) {return "etfcc_or_fastestpriority";}
      else if (i == 9) {
        return "min_etf_fastestpriority";
      } else if (i == 10) {return "min_etfcc_fastestpriority";}
      else {return "invalid";}
    }


    const std::string et_option() const {return to_strTiming(ETOption);}
    void loadconfigtxt(const std::string& filename);
    void saveconfigtxt(const std::string& filename);
    template<typename X>
    std::string print2dArray(const std::string& name, std::vector<std::vector<NNTParam<X>>> vecvec);
    template<typename X>
    std::string print1dArray(const std::string& name, std::vector<NNTParam<X>> vecvec);
    template<typename X>
    std::vector<std::vector<X>> tcastvector(const std::vector<std::vector<NNTParam<X>>> vec) const
    {
      std::vector<std::vector<X>> ret;
      for (auto x : vec) {
        std::vector<X> line;
        for (auto y : x) {
          line.push_back((X) y);
        }
        ret.push_back(line);
      }
      return ret;
    }

    template<typename X>
    std::vector<X> tcastvector(const std::vector<NNTParam<X>> vec) const
    {
      std::vector<std::vector<X>> ret;
      for (auto x : vec) {
        ret.push_back((X) x);
      }
      return ret;
    }


    //void loadconfigroot(std::string& filename);
    //void saveconfigroot(std::string& filename);
    /** Network parameters **/
    NNTParam<unsigned> nInput;
    NNTParam<unsigned> nOutput;
    NNTParam<unsigned> nMLP;
    NNTParam<bool> targetZ;
    NNTParam<bool> targetTheta;
    NNTParam<bool> multiplyHidden;
    NNTParam<unsigned> tMax;
    NNTParam<bool> rescaleTarget;
    NNTParam<bool> cutSum;
    NNTParam<double> relevantCut;
    NNTParam<unsigned> ETOption;

    std::vector<std::vector<NNTParam<float>>> phiRangeUse;
    std::vector<std::vector<NNTParam<float>>> thetaRangeUse;
    std::vector<std::vector<NNTParam<float>>> invptRangeUse;
    std::vector<std::vector<NNTParam<float>>> phiRangeTrain;
    std::vector<std::vector<NNTParam<float>>> thetaRangeTrain;
    std::vector<std::vector<NNTParam<float>>> invptRangeTrain;
    std::vector<std::vector<NNTParam<float>>> nHidden;
    std::vector<std::vector<NNTParam<float>>> outputScale;
    std::vector<NNTParam<unsigned short>> maxHitsPerSL;
    std::vector<NNTParam<unsigned long>> SLpattern;
    std::vector<NNTParam<unsigned long>> SLpatternMask;
    std::vector<NNTParam<unsigned>> precision;
    std::vector<std::vector<NNTParam<float>>> IDRanges;
  private:
    template<typename X>
    bool checkarr(std::vector<std::vector<NNTParam<X>>> vec);
    template<typename X>
    bool checkarr(std::vector<NNTParam<X>> vec);

    template<typename X>
    std::vector<std::vector<NNTParam<X>>> read2dArray(std::string keyx, bool locked);

    template<typename X>
    std::vector<NNTParam<X>> read1dArray(std::string keyx, bool locked);

  };
}

#endif
