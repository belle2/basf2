#ifndef GRLMLP_H
#define GRLMLP_H

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class GRLMLP : public TObject {

    // weights etc. are set only by the trainer
    friend class GRLNeuroTrainerModule;

  public:
    /** default constructor. */
    GRLMLP();

    /** constructor to set all parameters (not weights and relevantID ranges). */
    GRLMLP(std::vector<unsigned short>& nodes, unsigned short targets, std::vector<float>& outputscale);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~GRLMLP() { }

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

    //! Needed to make the ROOT object storable
    ClassDef(GRLMLP, 2);
  };
}
#endif
