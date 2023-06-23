
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <iostream>
#include <TObject.h>


namespace Belle2 {


  /** Class to represent a complete set to describe a Neurotrigger.
   */
  template <class T>
  class NNTParam {
    /** this is a typewrapper class for the parameters for the neurotrigger. for every parameter, it can also store the information wether the parameter was aleady set or if it is locked.
     * */
  public:
    NNTParam() {}
    /** copy constructor */
    NNTParam(const T& xvalue)
    {
      m_value = xvalue;
      m_set = true;
    }
    /** return the value of the data type this wrapper class is holding */
    operator T() const { return m_value;}

    /** assignment operator, but checks first if the variable is already locked */
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

    /** return the value of the data type this wrapper class is holding */
    NNTParam operator()() const {return m_value;}


    /** check, if variable was already initialized **/
    bool isSet() const {return m_set;}
    /** lock the variable, to make it read only */
    void lock() {m_locked = true;}
    /** check, if variable was already locked and is read only now */
    bool isLocked() const {return m_locked;}

  private:
    /** holding the value */
    T m_value;
    /* saves the "set" state */
    bool m_set = false;
    /** saves the "locked" state */
    bool m_locked = false;
  };

  class NeuroTriggerParameters : public TObject {
  public:
    NeuroTriggerParameters() {};
    NeuroTriggerParameters(std::string& filename);
    virtual ~NeuroTriggerParameters() {};
    // unfortunately i am too dumb to make the typewrapper class work for strings,
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
      else if (i == 9) {return "min_etf_fastestpriority";}
      else if (i == 10) {return "min_etfcc_fastestpriority";}
      else {return "invalid";}
    }


    /** return the string variant ov the et option */
    const std::string et_option() const {return to_strTiming(ETOption);}
    /** load the configuration from a file */
    void loadconfigtxt(const std::string& filename);
    /** save the configuration to a file */
    void saveconfigtxt(const std::string& filename);
    /** this is a class for piping the output of a 2d array to a string with brackets */
    template<typename X>
    std::string print2dArray(const std::string& name, std::vector<std::vector<NNTParam<X>>> vecvec);
    /** this is a class for piping the output of a 1d array to a string with brackets */
    template<typename X>
    std::string print1dArray(const std::string& name, std::vector<NNTParam<X>> vecvec);
    /** the parameters stored in the parameterset are not advanced enough to be vectors,
     * they can only be single data types. the workaround was to make every variable
     * contained in the (nested) vector an NNTParam. for the further use, those have to
     * be converted to float vecors, which is done by the tcastvector function. */
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


    /** Network parameters **/

    /** number of input nodes */
    NNTParam<unsigned> nInput;
    /** number of output nodes */
    NNTParam<unsigned> nOutput;
    /** Number of networks.
     * For network specific parameters you can give either a list with
     * values for each network, or a single value that will be used for all.
     * The ranges are also valid if nPhi * nPt * nTheta * nPattern = nMLPs.
     */
    NNTParam<unsigned> nMLP;
    /** train z as output */
    NNTParam<bool> targetZ;
    /** train theta as output */
    NNTParam<bool> targetTheta;
    /** If true, multiply nHidden with number of input nodes. */
    NNTParam<bool> multiplyHidden;
    /** Maximal drift time, identical for all networks. */
    NNTParam<unsigned> tMax;
    /** flag to allow for target tracks lying out of the output range to be rescaled during training. */
    NNTParam<bool> rescaleTarget;
    /** only used in the idhist module. it defines wether to use a cutsum or not */
    NNTParam<bool> cutSum;
    /** only used in the idhist module. it defines the how much of the idrange is cut off after making the histogram */
    NNTParam<double> relevantCut;
    /** Determine, how the event time should be obtained. The options are:
     *fastestpriority
     *fastest2d
     *etf
     *etf_or_fastestpriority
     *etf_or_fastest2d
     *etf_only
     *etfcc
     *etfhwin
     *etfcc_or_fastestpriority
     *min_etf_fastestpriority
     *min_etfcc_fastestpriority
     */
    NNTParam<unsigned> ETOption;

    /** Phi region for which MLP is used in degree for all networks. */
    std::vector<std::vector<NNTParam<float>>> phiRangeUse;
    /** Theta region for which MLP is used in degree for all networks. */
    std::vector<std::vector<NNTParam<float>>> thetaRangeUse;
    /** Charge / Pt region for which MLP is used in 1/GeV for all networks. */
    std::vector<std::vector<NNTParam<float>>> invptRangeUse;
    /** Phi region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
    std::vector<std::vector<NNTParam<float>>> phiRangeTrain;
    /** Theta region for which MLP is trained in degree for all networks.
       *  Can be larger than use range to avoid edge effects. */
    std::vector<std::vector<NNTParam<float>>> thetaRangeTrain;
    /** Charge / Pt region for which MLP is trained in 1/GeV for all networks.
       *  Can be larger than use range to avoid edge effects. */
    std::vector<std::vector<NNTParam<float>>> invptRangeTrain;
    /** Number of nodes in each hidden layer for all networks
       * or factor to multiply with number of inputs.
       * The number of layers is derived from the shape.
       */
    std::vector<std::vector<NNTParam<float>>> nHidden;
    /** Output scale for all networks. */
    std::vector<std::vector<NNTParam<float>>> outputScale;
    /** Maximum number of hits in a single super layer for all networks. */
    std::vector<NNTParam<unsigned short>> maxHitsPerSL;
    /** Super layer pattern for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: hits from super layer <i> are not used.
       *  1 in bit <i>: hits from super layer <i> are used.
       *  SLpattern = 0: use any hits present, don't check the pattern. */
    std::vector<NNTParam<unsigned long>> SLpattern;
    /** Super layer pattern mask for which MLP is trained for all networks.
       *  Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
       *  0 in bit <i>: super layer <i> may or may not have a hit.
       *  1 in bit <i>: super layer <i>
       *                - must have a hit if SLpattern bit <i> = 1
       *                - must not have a hit if SLpattenr bit <i> = 0 */
    std::vector<NNTParam<unsigned long>> SLpatternMask;
    /** precision used for the hardware simulation */
    std::vector<NNTParam<unsigned>> precision;
    /** relative ID range of the relevant wire IDs of the track segments that are taken into consideration when determining the best fitting track segments. */
    std::vector<std::vector<NNTParam<float>>> IDRanges;
  private:
    /** check, if a vector is already set. this is done by just checking the first member of the vector. */
    template<typename X>
    bool checkarr(std::vector<std::vector<NNTParam<X>>> vec);
    /** check, if a vector is already set. this is done by just checking the first member of the vector. */
    template<typename X>
    bool checkarr(std::vector<NNTParam<X>> vec);
    /** fill the array from a given string that looks like: [[1,2],[3, 4]]*/
    template<typename X>
    std::vector<std::vector<NNTParam<X>>> read2dArray(std::string keyx, bool locked);
    /** fill the array from a given string that looks like: [1,2,3]*/
    template<typename X>
    std::vector<NNTParam<X>> read1dArray(std::string keyx, bool locked);

  };
}

