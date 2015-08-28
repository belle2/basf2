/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERRESCUEANALYSISMODULE_H
#define PXDCLUSTERRESCUEANALYSISMODULE_H

#include <framework/core/Module.h>
#include <tracking/modules/pxdClusterRescue/PXDClusterRescueNNModule.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH3F.h>

#include <fstream>

namespace Belle2 {

  /**
   * Analysis module for PXDClusterRescue modules.
   * Main analysis module for cluster rescue modules. Writes information like classification statistics and pixels found by ROIs to ROOT file (e.g. use TBrowser to look at histograms).
   */

  class PXDClusterRescueAnalysisModule : public Module {
  public:
    PXDClusterRescueAnalysisModule();
    virtual ~PXDClusterRescueAnalysisModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void terminate();

    /**
     * worker function
     */
    virtual void event();

  private:
    std::ofstream m_fileLog, m_fileLostByROI;
    TFile* m_fileRoot;
    int m_countEvents;
    PXDClusterRescueNNModule m_PXDClusterRescueNN;

    TH1F* h_pxdClusters;
    TH1F* h_pxdClustersSignal;
    TH1F* h_pxdClustersHasTrack;
    TH1F* h_pxdClustersNN;
    TH1F* h_tracks;
    TH1F* h_ROIids;
    TH1F* h_ROIidsPixels;
    TH1F* h_foundByROI;
    TH1F* h_foundByNN;
    TH1F* h_foundByROIandNN;
    TH1F* h_foundByROIorNN;
    TH1F* h_foundByROIOnly;
    TH1F* h_foundByNNOnly;
    TH1F* h_foundByNNparticlePt;
    TH1F* h_foundByROIparticlePt;
    TH1F* h_foundByNNOnlyparticlePt;
    TH1F* h_foundByROIOnlyparticlePt;
    TH1F* h_foundByNNparticleP;
    TH1F* h_foundByROIparticleP;
    TH1F* h_foundByNNOnlyparticleP;
    TH1F* h_foundByROIOnlyparticleP;
    TH1F* h_foundByNNWithTrack;
    TH1F* h_foundByROIWithTrack;
    TH1F* h_particlePt;
    TH1F* h_particlePtWithTrack;
    TH1F* h_particlePtWithoutTrack;
    TH1F* h_particleP;
    TH1F* h_particlePWithTrack;
    TH1F* h_particlePWithoutTrack;
    TH1F* h_particlesPDG;
    TH1F* h_particlesPDGCluster;
    TH1F* h_particelPtLessThresh;
    TH1F* h_particelPtLessThreshFoundNN;

    int m_h3bins; // number of 3d histo bins
    TH3F* h_positionClusters;
    TH3F* h_positionClustersLostByROI;
    TH3F* h_positionClustersFoundByNN;
    TH3F* h_positionClustersHasNoTrack;
    TH3F* h_positionClustersSignalVsNoTrack;
    TH3F* h_positionClustersSignalVsFoundByNN;
    TH3F* h_positionClustersSignalVsLostByROI;

  protected:
    std::string m_TracksColName; /**< Name of tracks in DataStore */
    std::string m_ROIidColName; /**< Name of ROIids in DataStore */
    std::string m_PXDClusterNNColName; /**< Name of PXDCluster from PXDClusterRescueNN in DataStore */
    std::string m_filenameLog; /**< Filename for log file */
    std::string m_filenameRoot; /**< Filename for ROOT file */
    std::string m_filenameLostByROI; /**< Filename for training variables of clusters lost by ROI cluster rescue */
    std::vector<int> m_particleInclude; /**< List of included particles by PDG number */
    float m_particlePtThreshold; /**< Pt threshold on particles with valid PDG number */
    bool m_storeLostByROI; /**< Store training variables of clusters lost by ROI cluster rescue */
  };

}

#endif
