/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERRESCUEROIANALYSISMODULE_H
#define PXDCLUSTERRESCUEROIANALYSISMODULE_H

#include <framework/core/Module.h>

#include <TFile.h>
#include <TH1F.h>

#include <fstream>

namespace Belle2 {

  /**
   * Analysis module for PXDClusterRescueROI module.
   */

  class PXDClusterRescueROIAnalysisModule : public Module {
  public:
    PXDClusterRescueROIAnalysisModule();
    virtual ~PXDClusterRescueROIAnalysisModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

  private:
    std::ofstream m_fileLog;
    TFile* m_fileRoot;
    int m_countEvents;
    int m_countClustersAll;
    int m_countClustersLost;

    TH1F* h_pxdClusters;
    TH1F* h_pxdClustersRelated;
    TH1F* h_ROIids;
    TH1F* h_pixelsAll;
    TH1F* h_pixelsROI;
    TH1F* h_pixelsFoundRate;
    TH1F* h_pixelsROICluster;

  protected:
    std::string m_TracksColName; /**< Name of tracks in DataStore */
    std::string m_ROIidColName; /**< Name of ROIids in DataStore */
    std::string m_filenameLog; /**< Filename for log file */
    std::string m_filenameRoot; /**< Filename for ROOT file */
  };

}

#endif
