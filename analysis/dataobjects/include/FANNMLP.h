/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FANNMLP_H
#define FANNMLP_H

#include <framework/datastore/RelationsObject.h>

#include <vector>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class FANNMLP : public RelationsObject {
  public:
    /** Default constructor. */
    FANNMLP();
//
//     /** Constructor to set all parameters (not weights and relevantID ranges). */
    FANNMLP(std::string arrayName,
            std::vector<std::string> variableNames,
            std::string targetName,
            std::vector<unsigned short> nNodes,
            std::string neuronType,
            std::string trainingMethod,
            int nCycles,
            float validationFraction,
            int randomSeeds,
            int testRate,
            int nThreads,
            bool epochMonitoring);

    /** Destructor, empty because we don't allocate memory anywhere. */
    ~FANNMLP() { }

    /** Set the weights */
    void setWeights(std::vector<float>& w) { weights = w; }

    /** Get number of layers */
    unsigned nLayers() const { return nNodes.size(); }
    /** Get number of nodes in a layer */
    unsigned nNodesLayer(unsigned iLayer) const { return nNodes[iLayer]; }
    /** Get number of weights from length of weights vector */
    unsigned nWeights() const { return weights.size(); }
    /** Calculate number of weights from number of nodes */
    unsigned nWeightsCal() const;

    /** Get MLP array name */
    std::string getArrayName() const {return arrayName; }
    /** Get list of input variable names. */
    std::vector<std::string> getVariableNames() const {return variableNames;};
    /** Get target variable name. */
    std::string getTargetName() const {return targetName;};
    /** Get weights vector */
    std::vector<float> getWeights() const { return weights; }
    /** Get neuron type */
    std::string getNeuronType() const { return neuronType; }
    /** Get training algorithm */
    std::string getTrainingMethod() const { return trainingMethod; }
    /** Get maximum number of training epochs */
    int getNCycles() const { return nCycles;};
    /** Get fraction of events used for testing and validation  */
    float getValidationFraction() const { return validationFraction;};
    /** Get number of training runs with different random start weights.*/
    int getRandomSeeds() const { return randomSeeds;};
    /** Get the ith epoch to  test for overtraining*/
    int getTestRate() const { return testRate;};
    /** Number of threads for training. */
    int getNThreads() const { return nThreads;};
    /** Get if FANNlogFile is produced or not. */
    bool getEpochMonitoring() const { return epochMonitoring;};

    /** Sets two vectors containing the list of minimal and maximal input values to be used for scaling. */
    void setMinAndMaxInputSample(std::vector<std::vector<float>> vector) {minAndMaxInputSample = vector;}
    /** Sets two vectors containing the list of minimal and maximal target values to be used for scaling. */
    void setMinAndMaxTargetSample(std::vector<std::vector<float>> vector) { minAndMaxTargetSample = vector;}

    /** Gets two vectors containing the list of minimal and maximal input values to be used for scaling. */
    std::vector<std::vector<float>> getMinAndMaxInputSample() const {return minAndMaxInputSample;};
    /** Gets two vectors containing the list of minimal and maximal target values to be used for scaling. */
    std::vector<std::vector<float>> getMinAndMaxTargetSample() const {return minAndMaxTargetSample;};

    /** Scale input values to [-1, 1] */
    std::vector<float> scaleInput(std::vector<float> target) const;
    /** Unscale input values from [-1, 1] to the normal scale */
    std::vector<float> unscaleInput(std::vector<float> target) const;

    /** Scale target value to [-1, 1] */
    std::vector<float> scaleTarget(std::vector<float> target) const;
    /** Scale target value from [-1, 1] to the normal scale */
    std::vector<float> unscaleTarget(std::vector<float> target) const;

    /** Run an expert MLP.
     * @param input vector of input values
     * @return unscaled output values (z vertex in cm and/or theta in radian) */
    std::vector<float> runMLP(std::vector<float> input);

  private:
    /** MLP array name. */
    std::string arrayName;

    /** List of input variable names. */
    std::vector<std::string> variableNames;

    /** Target variable name. */
    std::string targetName;

    /** Number of nodes in each layer, not including bias nodes. */
    std::vector<unsigned short> nNodes;

    /** Weights of the network. */
    std::vector<float> weights;

    /** Activation Function for all neurons in the network. */
    std::string neuronType;

    /** Applied Training Algorithm. */
    std::string trainingMethod;

    /** Maximum number of training epochs. */
    int nCycles;

    /** Fraction of events used for testing and validation */
    float validationFraction;

    /** Number of training runs with different random start weights. */
    int randomSeeds; //.

    /** Training is stopped if validation error is higher than checkInterval epochs ago, i.e. either the validation error is increasing or the gain is less than the fluctuations. */
    int testRate;

    /** Number of threads for training. */
    int nThreads; //

    /** Produce FANNlogFile or not. */
    bool epochMonitoring;

    /** Two vectors containing the list of minimal and maximal input values to be used for scaling. */
    std::vector<std::vector<float>> minAndMaxInputSample;

    /** Two vectors containing the list of minimal and maximal target values to be used for scaling. */
    std::vector<std::vector<float>> minAndMaxTargetSample;

    //! Needed to make the ROOT object storable
    ClassDef(FANNMLP, 3);
  };
}
#endif
