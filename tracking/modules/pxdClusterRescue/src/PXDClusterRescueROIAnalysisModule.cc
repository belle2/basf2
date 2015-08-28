/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueROIAnalysisModule.h>

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <tracking/dataobjects/ROIid.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <TObjArray.h>

#include <ctime>

#define DEBUG_LEVEL 99

using namespace Belle2;

REG_MODULE(PXDClusterRescueROIAnalysis)

PXDClusterRescueROIAnalysisModule::PXDClusterRescueROIAnalysisModule() : Module()
{
  // set description
  setDescription("Analysis module for PXDClusterRescueROI module.");

  // set params
  addParam("TracksColName", m_TracksColName, "Name of tracks in DataStore.", std::string("Tracks"));
  addParam("ROIidColName", m_ROIidColName, "Name of ROIids in DataStore.", std::string(""));
  addParam("filenameLog", m_filenameLog, "Filename for log file.", std::string("PXDClusterRescueROIAnalysis.log"));
  addParam("filenameRoot", m_filenameRoot, "Filename for ROOT output file.", std::string("PXDClusterRescueROIAnalysis.root"));
}

PXDClusterRescueROIAnalysisModule::~PXDClusterRescueROIAnalysisModule()
{
}

void PXDClusterRescueROIAnalysisModule::initialize()
{
  // set needed data on required
  StoreArray<PXDCluster>::required();
  StoreArray<PXDDigit>::required();
  StoreArray<ROIid>::required(m_ROIidColName);
  StoreArray<Track>::required(m_TracksColName);

  // open log file
  m_fileLog.open(m_filenameLog);

  // write log file: header, filename
  m_fileLog << "// Log file for PXDClusterRescueROIAnalysis module" << std::endl;

  time_t time_now = time(0);
  tm* gmt_time = gmtime(&time_now);
  char* time_string = asctime(gmt_time);
  m_fileLog << "Timestamp (GMT): " << time_string;

  m_fileLog << "Filename log file: " << m_filenameLog << std::endl;
  m_fileLog << "Filename ROOT file: " << m_filenameRoot << std::endl;
  m_fileLog << std::endl;

  // init counter
  m_countEvents = 0;
  m_countClustersAll = 0;
  m_countClustersLost = 0;

  // init root file
  m_fileRoot = new TFile(m_filenameRoot.c_str(), "recreate");
  if (m_fileRoot->IsZombie()) B2ERROR("Could not open ROOT output file. " << m_filenameRoot);

  // init histograms
  h_pxdClusters = new TH1F("h_pxdClusters", "Number of all PXDClusters per track", 100, 0, 10 - 1);
  h_pxdClustersRelated = new TH1F("h_pxdClustersRelated", "Number of related PXDClusters per track", 100, 0, 10 - 1);
  h_ROIids = new TH1F("h_ROIids", "Number of ROIids per track", 100, 0, 10 - 1);
  h_pixelsAll = new TH1F("h_pixelsAll", "Number of cluster pixels (all) per track", 100, 0, 10 - 1);
  h_pixelsROI = new TH1F("h_pixelsROI", "Number of ROI pixels (all) per track", 100, 0, 16000 - 1);
  h_pixelsFoundRate = new TH1F("h_pixelsFoundRate", "Cluster pixel rescue rate", 100, -0.1, 1.1);
  h_pixelsROICluster = new TH1F("h_pixelsROICluster", "Cluster pixels vs. ROI pixels", 100, 0, 0.01);
}

void PXDClusterRescueROIAnalysisModule::beginRun()
{
}

void PXDClusterRescueROIAnalysisModule::event()
{
  B2DEBUG(DEBUG_LEVEL, "##########");
  B2DEBUG(DEBUG_LEVEL, "Event: " << m_countEvents);
  B2DEBUG(DEBUG_LEVEL, "##########");

  // load required data
  StoreArray<ROIid> ROIids(m_ROIidColName);
  StoreArray<Track> Tracks(m_TracksColName);
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<PXDDigit> pxdDigits;

  // go through tracks, get related particle and look for related PXD clusters
  int countRelatedClusters = 0;
  int countPixelsFound = 0;
  int countPixelsAll = 0;
  int TracksEntries = Tracks.getEntries();

  B2DEBUG(DEBUG_LEVEL, "-> Tracks in event: " << TracksEntries);

  for (int k = 0; k < TracksEntries; k++) {
    B2DEBUG(DEBUG_LEVEL, "##########");
    B2DEBUG(DEBUG_LEVEL, "Track #: " << k);
    B2DEBUG(DEBUG_LEVEL, "##########");

    // get related particles
    RelationVector<MCParticle> particles = Tracks[k]->getRelationsTo<MCParticle>();

    if (particles.size() != 1) B2ERROR("Found track without particle.");

    // get related PXD clusters
    RelationVector<PXDCluster> pxdClustersRelated = particles[0]->getRelationsFrom<PXDCluster>();
    countRelatedClusters += pxdClustersRelated.size();
    B2DEBUG(DEBUG_LEVEL, "-> Found related clusters: " << pxdClustersRelated.size());

    // go through clusters
    for (unsigned int p = 0; p < pxdClustersRelated.size(); p++) {
      RelationVector<PXDDigit> pxdDigitsRelated = pxdClustersRelated[p]->getRelationsTo<PXDDigit>();

      B2DEBUG(DEBUG_LEVEL, "Related cluster #: " << p);
      B2DEBUG(DEBUG_LEVEL, "-> Start cluster (u/v): " << pxdClustersRelated[p]->getUStart() << "/" << pxdClustersRelated[p]->getVStart());
      B2DEBUG(DEBUG_LEVEL, "-> Size cluster (u/v): " << pxdClustersRelated[p]->getUSize() << "/" << pxdClustersRelated[p]->getVSize());

      // check whether there are ROIids on the same sensor
      B2DEBUG(DEBUG_LEVEL, "ROI on same sensor (" << pxdClustersRelated[p]->getSensorID() << "): ");
      bool foundSensor = false;
      for (int i = 0; i < ROIids.getEntries(); i++) {
        if (pxdClustersRelated[p]->getSensorID() == ROIids[i]->getSensorID()) {
          foundSensor = true;
          B2DEBUG(DEBUG_LEVEL, "-> ROI position (u min max v min max): " << ROIids[i]->getMinUid() << "/" << ROIids[i]->getMaxUid() << "/" <<
                  ROIids[i]->getMinVid() << "/" << ROIids[i]->getMaxVid());
          B2DEBUG(DEBUG_LEVEL, "-> ROI size: " << (ROIids[i]->getMaxUid() - ROIids[i]->getMinUid() + 1) *
                  (ROIids[i]->getMaxVid() - ROIids[i]->getMinVid() + 1));
          // count pixels of cluster in this ROI
          int countPixelsThisROI = 0;
          bool foundAll = true;
          for (unsigned int ii = 0; ii < pxdDigitsRelated.size(); ii++) {
            if (ROIids[i]->Contains(*pxdDigitsRelated[ii])) {
              countPixelsThisROI++;
            } else {
              foundAll = false;
            }
          }
          B2DEBUG(DEBUG_LEVEL, "-> Pixels of related clusters in this ROI: " << countPixelsThisROI);
          B2DEBUG(DEBUG_LEVEL, "-> Found all pixels: " << foundAll);
        }
      }

      // count lost and all clusters
      if (!foundSensor) m_countClustersLost++;
      m_countClustersAll++;

      // check pixels of cluster whether they are in ROI or not
      for (unsigned int i = 0; i < pxdDigitsRelated.size(); i++) {
        for (int j = 0; j < ROIids.getEntries(); j++) {
          if (ROIids[j]->Contains(*pxdDigitsRelated[i])) {
            countPixelsFound++;
            break; // jump to next pixel if this one is found (avoid double findings)
          }
        }

        countPixelsAll++;
      }
    }
  }

  // go through ROI and count pixels
  int countPixelsROI = 0;
  for (int k = 0; k < ROIids.getEntries(); k++) {
    countPixelsROI += (ROIids[k]->getMaxUid() - ROIids[k]->getMinUid() + 1) * (ROIids[k]->getMaxVid() - ROIids[k]->getMinVid() + 1);
  }

  // histogram: number of all PXDClusters per track
  h_pxdClusters->Fill(pxdClusters.getEntries() / float(TracksEntries));

  // histogram: number of ROIids per track
  h_ROIids->Fill(ROIids.getEntries() / float(TracksEntries));

  // histogram: number of related clusters per track
  h_pxdClustersRelated->Fill(countRelatedClusters / float(TracksEntries));

  // histogram: cluster pixels per track
  h_pixelsAll->Fill(countPixelsAll / float(TracksEntries));

  // histogram: ROI pixels per track
  h_pixelsROI->Fill(countPixelsROI / float(TracksEntries));

  // histogram: found cluster pixels/all cluster pixels per track
  h_pixelsFoundRate->Fill(countPixelsFound / float(countPixelsAll));

  // histogram: cluster pixels/ROI pixels per track
  h_pixelsROICluster->Fill(countPixelsAll / float(countPixelsROI));

  // increment counters
  m_countEvents++;
}

void PXDClusterRescueROIAnalysisModule::endRun()
{
}

void PXDClusterRescueROIAnalysisModule::terminate()
{
  // write counter to file
  m_fileLog << "Count events: " << m_countEvents << std::endl;
  m_fileLog << "Count clusters all/lost: " << m_countClustersAll << "/" << m_countClustersLost << std::endl;

  // write histograms to file
  TObjArray Hlist(0);
  Hlist.Add(h_pxdClusters);
  Hlist.Add(h_pxdClustersRelated);
  Hlist.Add(h_ROIids);
  Hlist.Add(h_pixelsAll);
  Hlist.Add(h_pixelsROI);
  Hlist.Add(h_pixelsFoundRate);
  Hlist.Add(h_pixelsROICluster);
  Hlist.Write();

  // close file
  m_fileRoot->Close();
}
