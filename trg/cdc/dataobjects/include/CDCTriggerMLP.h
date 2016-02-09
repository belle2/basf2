#ifndef CDCTRIGGERMLP_H
#define CDCTRIGGERMLP_H

#include <TObject.h>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class CDCTriggerMLP : public TObject {
  public:
    /** default constructor. */
    CDCTriggerMLP();

    /** constructor to set all parameters (not weights and relevantID ranges). */
    CDCTriggerMLP(std::vector<unsigned short> nNodes,
                  unsigned short targetVars,
                  std::vector<float> outputScale,
                  std::vector<float> phiRange,
                  std::vector<float> invptRange,
                  std::vector<float> thetaRange,
                  unsigned short SLpattern,
                  unsigned short tMax);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerMLP() { }

    /** set the relevant Id range */
    void setRelevantID(std::vector<float>& relId) { relevantID = relId; }
    /** set the weights */
    void setWeights(std::vector<float>& w) { weights = w; }
    /** set phi range for which this expert is trained */
    void setPhiRange(std::vector<float>& phi) { phiRange = phi; }
    /** set charge / Pt range for which this expert is trained */
    void setInvptRange(std::vector<float>& invpt) { invptRange = invpt; }
    /** set theta range for which this expert is trained */
    void setThetaRange(std::vector<float>& theta) { thetaRange = theta; }

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
    /** get phi range for which this expert is trained */
    std::vector<float> getPhiRange() const { return phiRange; }
    /** get charge / Pt range for which this expert is trained */
    std::vector<float> getInvptRange() const { return invptRange; }
    /** get theta range for which this expert is trained */
    std::vector<float> getThetaRange() const { return thetaRange; }
    /** get super layer pattern */
    unsigned short getSLpattern() const { return SLpattern; }
    /** get maximal drift time */
    unsigned short getTMax() const { return tMax; }

    /** check whether given phi value is in sector */
    bool inPhiRange(float phi) const;
    /** check whether given pt value is in sector */
    bool inPtRange(float pt) const;
    /** check whether given theta value is in sector */
    bool inThetaRange(float theta) const;
    /** check whether given relative TS ID is in relevant range */
    bool isRelevant(float relId, unsigned iSL) const;

    /** scale relative TS ID from relevant range to [-1, 1] */
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

    /** Super layer pattern for which this expert is trained.
      * Binary pattern of 9 bits (on/off for each SL).
      * 0 means no restriction rather than no inputs. */
    unsigned short SLpattern;

    /** Maximal drift time (for scaling), hits with larger values are ignored. */
    unsigned short tMax;
    /** Hits must be within ID region around 2D track to be used as input.
      * Default for axial layers is +- 1 wire,
      * default for stereo layers is region spanned by stereos +- 1 wire. */
    std::vector<float> relevantID;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLP, 3);
  };
}
#endif
