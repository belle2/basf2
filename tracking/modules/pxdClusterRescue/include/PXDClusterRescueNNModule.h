/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERRESCUENNMODULE_H
#define PXDCLUSTERRESCUENNMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <neurobayes/NeuroBayesExpert.hh>

namespace Belle2 {

  /**
   * PXDCluster rescue module with signal/background classification by NeuroBayes neural network
   */

  class PXDClusterRescueNNModule : public Module {
  public:
    PXDClusterRescueNNModule();
    virtual ~PXDClusterRescueNNModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

    /**
     * get training variables from PXDCluster
     * variable 1: sum pixel charge
     * variable 2: minimum pixel charge
     * variable 3: maximum pixel charge
     * variable 4: charge mean
     * variable 5: charge variance
     * variable 6: cluster size
     * variable 7: cluster size in z
     * variable 8: cluster size in rphi
     * variable 9: global cluster coordinate in z
     * variable 10: global cluster coordinate in r
     * variable 11: global cluster coordinate in phi
     */
    virtual void getPXDClusterTrainingVariables(const PXDCluster* pxdCluster, std::vector<float>& trainingVariables);

    /**
     * return number of training variables of neural network
     */
    virtual int getNumTrainingVariables();

  private:
    Expert* m_NBExpert;

  protected:
    std::string m_filenameExpertise; /**< filename of NeuroBayes expertise input */
    float m_classThreshold; /**< classification threshold for NeuroBayes teacher*/
    std::string m_namePXDClusterInput; /**< Name of input PXD clusters in DataStore */
    std::string m_namePXDClusterOutput; /**< Name of output PXD clusters in DataStore */

    SelectSubset<PXDCluster> m_pxdClustersSubset;

    const int m_NumTrainingVariables = 11; /**< number of training variables*/
  };

}

#endif
