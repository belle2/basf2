/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
    GRLMLP(std::vector<unsigned short>& nodes, unsigned short targets, const std::vector<float>& outputscale);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~GRLMLP() { }

    /** check if weights are default values or set by some trainer */
    bool isTrained() const { return m_trained; }
    /** get number of layers */
    unsigned getNumberOfLayers() const { return m_nNodes.size(); }
    /** get number of nodes in a layer */
    unsigned getNumberOfNodesLayer(unsigned iLayer) const { return m_nNodes[iLayer]; }
    /** get number of weights from length of weights vector */
    unsigned getNumberOfWeights() const { return m_weights.size(); }
    /** calculate number of weights from number of nodes */
    unsigned nWeightsCal() const;
    /** get weights vector */
    std::vector<float> getWeights() const { return m_weights; }

    /** check if weights are default values or set by some trainer */
    void Trained(bool trained) { m_trained = trained; }

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

    //! Needed to make the ROOT object storable
    ClassDef(GRLMLP, 2);
  };
}
#endif
