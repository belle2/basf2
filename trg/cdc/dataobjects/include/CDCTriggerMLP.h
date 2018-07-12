#ifndef CDCTRIGGERMLP_H
#define CDCTRIGGERMLP_H

#include <TObject.h>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class CDCTriggerMLP : public TObject {

    // weights etc. are set only by the trainer
    friend class CDCTriggerNeuroTrainerModule;

  public:
    /** default constructor. */
    CDCTriggerMLP();

    /** constructor to set all parameters (not weights and relevantID ranges). */
    CDCTriggerMLP(std::vector<unsigned short>& nodes,
                  unsigned short targets,
                  std::vector<float>& outputscale,
                  std::vector<float>& phirange,
                  std::vector<float>& invptrange,
                  std::vector<float>& thetarange,
                  unsigned short maxHits,
                  unsigned long pattern,
                  unsigned long patternMask,
                  unsigned short tmax,
                  bool calcT0);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerMLP() { }

    /** check if weights are default values or set by some trainer */
    bool isTrained() const { return trained; }
    /** get number of layers */
    unsigned nLayers() const { return nNodes.size(); }
    /** get number of nodes in a layer */
    unsigned nNodesLayer(unsigned iLayer) const { return nNodes[iLayer]; }
    /** get number of weights from length of weights vector */
    unsigned nWeights() const { return weights.size(); }
    /** calculate number of weights from number of nodes */
    unsigned nWeightsCal() const;
    /** get weights vector */
    std::vector<float> getWeights() const { return weights; }
    /** get maximum hit number for a single super layer */
    unsigned short getMaxHitsPerSL() const { return maxHitsPerSL; }
    /** get super layer pattern */
    unsigned long getSLpattern() const { return SLpattern & SLpatternMask; }
    /** get bitmask for super layer pattern */
    unsigned long getSLpatternMask() const { return SLpatternMask; }
    /** get raw super layer pattern */
    unsigned long getSLpatternUnmasked() const { return SLpattern; }
    /** get maximal drift time */
    unsigned short getTMax() const { return tMax; }
    /** get relevant ID range for given super layer */
    std::vector<float> getIDRange(unsigned iSL) const
    {
      return {relevantID[2 * iSL], relevantID[2 * iSL + 1]};
    }
    /** get flag for event time definition */
    bool getT0fromHits() const { return T0fromHits; }

    /** check whether given phi value is in sector */
    bool inPhiRange(float phi) const;
    /** check whether given pt value is in sector */
    bool inPtRange(float pt) const;
    /** check whether given 1/pt value is in sector */
    bool inInvptRange(float invpt) const;
    /** check whether given theta value is in sector */
    bool inThetaRange(float theta) const;
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
    std::vector<unsigned short> nNodes;
    /** Weights of the network. */
    std::vector<float> weights;
    /** Indicator whether the weights are just default values or have
     *  been set by some trainer (set to true when setWeights() is first called). */
    bool trained;

    /** output variables: 1: z, 2: theta, 3: (z, theta) */
    unsigned short targetVars;
    /** Output[i] of the MLP is scaled from [-1, 1]
     *  to [outputScale[2i], outputScale[2i+1]]. */
    std::vector<float> outputScale;

    /** Phi region in radian for which this expert is trained.
      * Valid ranges go from -2pi to 2pi. */
    std::vector<float> phiRange;
    /** Charge / Pt region in 1/GeV for which this expert is trained.
      * Taking 1 / Pt instead of Pt means that straight tracks are at 0,
      * i.e. there is a smooth transition from positive to negative charge. */
    std::vector<float> invptRange;
    /** Theta region in radian for which this expert is trained. */
    std::vector<float> thetaRange;

    /** Maximum number of inputs for a single super layer. */
    unsigned short maxHitsPerSL;
    /** Super layer pattern for which this expert is trained.
      * Binary pattern of 9 * maxHitsPerSL bits (on/off for each hit).
      * 0 means no restriction rather than no inputs. */
    unsigned long SLpattern;
    /** Bitmask for comparing the super layer pattern.
      * A track matches a sector, if
      * SLpattern & SLpatternMask == hitPattern(track) & SLpatternMask. */
    unsigned long SLpatternMask;

    /** Maximal drift time (for scaling), hits with larger values are ignored. */
    unsigned short tMax;
    /** Hits must be within ID region around 2D track to be used as input.
      * Default for axial layers is +- 1 wire,
      * default for stereo layers is region spanned by stereos +- 1 wire. */
    std::vector<float> relevantID;

    /** If true, the event time will be determined
      * from hits within relevantID region, if it is missing.
      * Otherwise, no drift times are used if the event time is missing.
      * Stored here to make sure that the event time definition is the same
      * during training and during execution. */
    bool T0fromHits;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLP, 6);
  };
}
#endif
