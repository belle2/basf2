/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERMLP_H
#define CDCTRIGGERMLP_H

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class CDCTriggerMLP : public TObject {

    // weights etc. are set only by the trainer
    friend class CDCTriggerNeuroTrainerModule;
    friend class NeuroTrigger;

  public:
    /** default constructor. */
    CDCTriggerMLP();

    /** constructor to set all parameters (not weights and relevantID ranges). */
    CDCTriggerMLP(std::vector<unsigned short>& nodes,
                  unsigned short targets,
                  std::vector<float>& outputscale,
                  std::vector<float>& phirangeUse,
                  std::vector<float>& invptrangeUse,
                  std::vector<float>& thetarangeUse,
                  std::vector<float>& phirangeTrain,
                  std::vector<float>& invptrangeTrain,
                  std::vector<float>& thetarangeTrain,
                  unsigned short maxHits,
                  unsigned long pattern,
                  unsigned long patternMask,
                  unsigned short tmax,
                  const std::string& etoption);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerMLP() { }

    /** check if weights are default values or set by some trainer */
    bool isTrained() const { return m_trained; }
    /** get number of layers */
    unsigned nLayers() const { return m_nNodes.size(); }
    /** get number of nodes in a layer */
    unsigned nNodesLayer(unsigned iLayer) const { return m_nNodes[iLayer]; }
    /** get number of weights from length of weights vector */
    unsigned nWeights() const { return m_weights.size(); }
    /** calculate number of weights from number of nodes */
    unsigned nWeightsCal() const;
    /** get weights vector */
    std::vector<float> getWeights() const { return m_weights; }
    /** set weights vector */
    void setWeights(std::vector<float> xweights) {m_weights = xweights; }
    /** get maximum hit number for a single super layer */
    unsigned short getMaxHitsPerSL() const { return m_maxHitsPerSL; }
    /** get super layer pattern */
    unsigned long getSLpattern() const { return m_SLpattern & m_SLpatternMask; }
    /** get bitmask for super layer pattern */
    unsigned long getSLpatternMask() const { return m_SLpatternMask; }
    /** get raw super layer pattern */
    unsigned long getSLpatternUnmasked() const { return m_SLpattern; }
    /** get maximal drift time */
    unsigned short getTMax() const { return m_tMax; }
    /** get relevant ID range for given super layer */
    std::vector<float> getIDRange(unsigned iSL) const
    {
      return {m_relevantID[2 * iSL], m_relevantID[2 * iSL + 1]};
    }
    /** set and get total relevant ID range */
    void setRelID(std::vector<float> relid) {m_relevantID = relid;}
    std::vector<float> getRelID() const {return m_relevantID;}
    /** Returns way of obtaining the event time */
    std::string get_et_option() const { return m_et_option; }

    /** check whether given phi value is in sector */
    bool inPhiRangeUse(float phi) const;
    /** check whether given pt value is in sector */
    bool inPtRangeUse(float pt) const;
    /** check whether given 1/pt value is in sector */
    bool inInvptRangeUse(float invpt) const;
    /** check whether given theta value is in sector */
    bool inThetaRangeUse(float theta) const;
    /** check whether given phi value is in training sector */
    bool inPhiRangeTrain(float phi) const;
    /** check whether given pt value is in training sector */
    bool inPtRangeTrain(float pt) const;
    /** check whether given 1/pt value is in training sector */
    bool inInvptRangeTrain(float invpt) const;
    /** check whether given theta value is in training sector */
    bool inThetaRangeTrain(float theta) const;
    /** check whether given relative TS ID is in relevant range */
    bool isRelevant(float relId, unsigned iSL) const;

    /** scale relative TS ID from relevant range to approximately [-1, 1]
     *  (to facilitate the FPGA implementation, the scale factor is rounded to a power of 2)*/
    float scaleId(double relId, unsigned iSL) const;
    /** scale target value from outputScale to [-1, 1] */
    std::vector<float> scaleTarget(std::vector<float> target) const;
    /** scale target value from [-1, 1] to outputScale */
    std::vector<float> unscaleTarget(std::vector<float> target) const;

    /** get target index for z (-1 if no output is trained for z) */
    int zIndex() const;
    /** get target index for theta (-1 if no output is trained for theta) */
    int thetaIndex() const;

  private:
    /** Number of nodes in each layer, not including bias nodes. */
    std::vector<unsigned short> m_nNodes;
    /** Weights of the network. */
    std::vector<float> m_weights;
    /** Indicator whether the weights are just default values or have
     *  been set by some trainer (set to true when setWeights() is first called). */
    bool m_trained;

    /** output variables: 1: z, 2: theta, 3: (z, theta) */
    unsigned short m_targetVars;
    /** Output[i] of the MLP is scaled from [-1, 1]
     *  to [outputScale[2i], outputScale[2i+1]]. */
    std::vector<float> m_outputScale;

    /** Phi region in radian for which this expert is used.
      * Valid ranges go from -2pi to 2pi. */
    std::vector<float> m_phiRangeUse;
    /** Charge / Pt region in 1/GeV for which this expert is used.
      * Taking 1 / Pt instead of Pt means that straight tracks are at 0,
      * i.e. there is a smooth transition from positive to negative charge. */
    std::vector<float> m_invptRangeUse;
    /** Theta region in radian for which this expert is trained. */
    std::vector<float> m_thetaRangeUse;
    /** Phi region in radian for which this expert is used.
      * Valid ranges go from -2pi to 2pi. */
    std::vector<float> m_phiRangeTrain;
    /** Charge / Pt region in 1/GeV for which this expert is trained.
      * Taking 1 / Pt instead of Pt means that straight tracks are at 0,
      * i.e. there is a smooth transition from positive to negative charge. */
    std::vector<float> m_invptRangeTrain;
    /** Theta region in radian for which this expert is trained. */
    std::vector<float> m_thetaRangeTrain;

    /** Maximum number of inputs for a single super layer. */
    unsigned short m_maxHitsPerSL;
    /** Super layer pattern for which this expert is trained.
      * Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
      * 0 means no restriction rather than no inputs. */
    unsigned long m_SLpattern;
    /** Bitmask for comparing the super layer pattern.
      * A track matches a sector, if
      * SLpattern & SLpatternMask == hitPattern(track) & SLpatternMask. */
    unsigned long m_SLpatternMask;

    /** Maximal drift time (for scaling), hits with larger values are ignored. */
    unsigned short m_tMax;
    /** Hits must be within ID region around 2D track to be used as input.
      * Default for axial layers is +- 1 wire,
      * default for stereo layers is region spanned by stereos +- 1 wire. */
    std::vector<float> m_relevantID;

    /**
     * Returns way of obtaining the event time.
     * The different options are:
     *   "etf_only"                 :   only ETF info is used, otherwise an error
     *                                  is thrown.
     *   "fastestpriority"          :   event time is estimated by fastest priority
     *                                  time in selected track segments. if something
     *                                  fails, it is set to 0.
     *   "zero"                     :   the event time is set to 0.
     *   "etf_or_fastestpriority"   :   the event time is obtained by the ETF, if
     *                                  not possible, the flag
     *                                  "fastestppriority" is used.
     *   "etf_or_zero"              :   the event time is obtained by the ETF, if
     */
    std::string m_et_option;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLP, 10);
  };
}
#endif
