/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERGETNNDATAMODULE_H
#define PXDCLUSTERGETNNDATAMODULE_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueNNModule.h>

#include <fstream>

namespace Belle2 {

  /**
   * Get training variables for NeuroBayes neural network from given signal or background data
   */

  class PXDClusterGetNNDataModule : public Module {
  public:
    PXDClusterGetNNDataModule();
    virtual ~PXDClusterGetNNDataModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

    /**
     * Classify PXDCluster as signal if at least one particle
     * 1) is on pdg number include list
     * 2) has relations weigth greater than m_minRelationWeight
     * 3) has pt less than m_maxPt
     */
    virtual bool PXDClusterIsType(PXDCluster* pxdCluster, const float& maxPt, const float& minRelationWeight,
                                  const std::vector<int>& particleInclude, const std::string& dataType);

  private:
    PXDClusterRescueNNModule m_PXDClusterRescueNN;
    std::ofstream m_fileData, m_fileLog;

  protected:
    std::string m_filenameData; /**< Filename of output data file */
    std::string m_filenameLog; /**< Filename of output log file */
    std::string m_dataType; /**< Type of input data (signal or background) */
    float m_maxPt; /**< Maximum pt of particles on include list for being signal*/
    float m_minRelationWeight; /**< Min relation weight to PXDCluster for being signal*/
    std::vector<int> m_particleInclude; /**< Included particles by pdg numbers */
  };

}

#endif
