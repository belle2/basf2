/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERRESCUENNANALYSISMODULE_H
#define PXDCLUSTERRESCUENNANALYSISMODULE_H

#include <framework/core/Module.h>

#include <tracking/modules/pxdClusterRescue/PXDClusterGetNNDataModule.h>

#include <fstream>

namespace Belle2 {

  /**
   * Analyse classification qualtiy of PXDClusterRescueNN module.
   */

  class PXDClusterRescueNNAnalysisModule : public Module {
  public:
    PXDClusterRescueNNAnalysisModule();
    virtual ~PXDClusterRescueNNAnalysisModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

  private:
    PXDClusterGetNNDataModule m_PXDClusterGetNNData;

    int m_countSignal; // count number of signal classified by particle
    int m_countSignalClass; // count number of signal classified by network
    int m_countSignalValid; // count number of signal classified correctly by network
    int m_countBackground;
    int m_countBackgroundClass;
    int m_countBackgroundValid;
    int m_countClusters; // count all clusters

    std::ofstream m_fileLog;

  protected:
    std::string m_filenameLog; /**< Filename of output log file */
    std::string m_dataType; /**< Type of input data (signal or background) */
    float m_maxPt; /**< Maximum pt of particles for being signal*/
    float m_minRelationWeight; /**< Min relation weight to PXDCluster for being signal*/
    std::vector<int> m_particleInclude; /**< Included particles by pdg numbers */
    std::string m_namePXDClusterAll; /**< "Name of input PXD clusters in DataStore with all PXDClusters */
    std::string m_namePXDClusterFiltered; /**< "Name of input PXD clusters in DataStore with filtered PXDClusters */

  };

}

#endif
