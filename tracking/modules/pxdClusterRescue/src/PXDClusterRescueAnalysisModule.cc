/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueAnalysisModule.h>

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <tracking/dataobjects/ROIid.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <TObjArray.h>

#include <ctime>

#define DEBUG_LEVEL 99

using namespace Belle2;

REG_MODULE(PXDClusterRescueAnalysis)

PXDClusterRescueAnalysisModule::PXDClusterRescueAnalysisModule() : Module()
{
  // set description
  setDescription("Analysis module for PXDClusterRescueROI module.");

  // set params
  addParam("TracksColName", m_TracksColName, "Name of tracks in DataStore.", std::string("Tracks"));
  addParam("ROIidColName", m_ROIidColName, "Name of ROIids in DataStore.", std::string(""));
  addParam("PXDClusterNNColName", m_PXDClusterNNColName, " Name of PXDCluster from PXDClusterRescueNN in DataStore.",
           std::string("PXDClusterFiltered"));
  addParam("filenameLog", m_filenameLog, "Filename for log file.", std::string("PXDClusterRescueAnalysis.log"));
  addParam("filenameRoot", m_filenameRoot, "Filename for ROOT output file.", std::string("PXDClusterRescueAnalysis.root"));
  addParam("particleInclude", m_particleInclude,
           "List of included particles by PDG number. Relations of these particles to a PXDCluster defines the cluster as signal.",
           std::vector<int>({211, -211}));
  addParam("particlePtThreshold", m_particlePtThreshold,
           "Pt threshold on particles with valid PDG number. Used for determining NN efficiency.", float(0.100));
  addParam("storeLostByROI", m_storeLostByROI, "Store training variables of clusters lost by ROI cluster rescue.", bool(false));
  addParam("filenameLostByROI", m_filenameLostByROI, "Filename to store training variable of cluster lost by ROI cluster rescue.",
           std::string("PXDClusterLostByROI.data"));
}

PXDClusterRescueAnalysisModule::~PXDClusterRescueAnalysisModule()
{
}

void PXDClusterRescueAnalysisModule::initialize()
{
  // set needed data on required
  StoreArray<PXDCluster>::required();
  StoreArray<MCParticle>::required();
  StoreArray<PXDCluster>::required(m_PXDClusterNNColName);
  StoreArray<PXDDigit>::required();
  StoreArray<ROIid>::required(m_ROIidColName);
  StoreArray<Track>::required(m_TracksColName);

  // open files
  m_fileLog.open(m_filenameLog);
  m_fileLostByROI.open(m_filenameLostByROI);

  // write log file: header, filename
  m_fileLog << "// Log file for PXDClusterRescueAnalysis module" << std::endl;

  time_t time_now = time(0);
  tm* gmt_time = gmtime(&time_now);
  char* time_string = asctime(gmt_time);
  m_fileLog << "Timestamp (GMT): " << time_string;

  m_fileLog << "Filename log file: " << m_filenameLog << std::endl;
  m_fileLog << "Filename ROOT file: " << m_filenameRoot << std::endl;
  m_fileLog << std::endl;

  // init counter
  m_countEvents = 0;

  // init root file
  m_fileRoot = new TFile(m_filenameRoot.c_str(), "recreate");
  if (m_fileRoot->IsZombie()) B2ERROR("Could not open ROOT output file. " << m_filenameRoot);

  // init histograms
  int numBinsEntries = 100;
  int numBinsEnergy = 60;
  float maxEnergy = 1;

  h_pxdClusters = new TH1F("h_pxdClusters", "Number of all PXDClusters", numBinsEntries, 0, numBinsEntries - 1);
  h_pxdClustersSignal = new TH1F("h_pxdClustersSignal", "Number of signal PXDClusters", numBinsEntries, 0, numBinsEntries - 1);
  h_pxdClustersHasTrack = new TH1F("h_pxdClustersHasTrack", "Number of signal PXDClusters with related track", numBinsEntries, 0,
                                   numBinsEntries - 1);
  h_pxdClustersNN = new TH1F("h_pxdClustersNN", "Number of PXDClusters from NN", numBinsEntries, 0, numBinsEntries - 1);
  h_tracks = new TH1F("h_tracks", "Number of tracks", numBinsEntries, 0, numBinsEntries - 1);
  h_ROIids = new TH1F("h_ROIids", "Number of ROIids", numBinsEntries, 0, numBinsEntries - 1);
  h_ROIidsPixels = new TH1F("h_ROIidsPixels", "Number of ROIid pixels", numBinsEntries, 0, 200000 - 1);
  h_foundByROI = new TH1F("h_foundByROI", "Number of PXDClusters found by ROI", numBinsEntries, 0, numBinsEntries - 1);
  h_foundByNN = new TH1F("h_foundByNN", "Number of PXDClusters found by NN", numBinsEntries, 0, numBinsEntries - 1);
  h_foundByROIandNN = new TH1F("h_foundByROIandNN", "Number of PXDClusters found by NN and ROI", numBinsEntries, 0,
                               numBinsEntries - 1);
  h_foundByROIorNN = new TH1F("h_foundByROIorNN", "Number of PXDClusters found by NN or ROI", numBinsEntries, 0, numBinsEntries - 1);
  h_foundByROIOnly = new TH1F("h_foundByROIOnly", "Number of PXDClusters found by ROI only", numBinsEntries, 0, numBinsEntries - 1);
  h_foundByNNOnly = new TH1F("h_foundByNNOnly", "Number of PXDClusters found by NN only", numBinsEntries, 0, numBinsEntries - 1);
  h_foundByNNWithTrack = new TH1F("h_foundByNNWithTrack", "Number of PXDClusters found by NN with related track", numBinsEntries, 0,
                                  numBinsEntries - 1);
  h_foundByROIWithTrack = new TH1F("h_foundByROIWithTrack", "Number of PXDClusters found by ROI with related track", numBinsEntries,
                                   0, numBinsEntries - 1);

  h_particlesPDG = new TH1F("h_particlesPDG", "PDG numbers of primary particles", 6000, -3000, 3000);
  h_particlesPDGCluster = new TH1F("h_particlesPDGCluster", "PDG numbers of primary particles related to cluster", 6000, -3000, 3000);

  h_particlePt = new TH1F("h_particlePt", "Pt of particles with valid PDG number", numBinsEnergy, 0, maxEnergy);
  h_particlePtWithTrack = new TH1F("h_particlePtWithTrack", "Pt of valid particles with track", numBinsEnergy, 0, maxEnergy);
  h_particlePtWithoutTrack = new TH1F("h_particlePtWithoutTrack", "Pt of valid particles without track", numBinsEnergy, 0, maxEnergy);
  h_foundByNNparticlePt = new TH1F("h_foundByNNparticlePt", "Pt of particle related to cluster found by NN", numBinsEnergy, 0,
                                   maxEnergy);
  h_foundByROIparticlePt = new TH1F("h_foundByROIparticlePt", "Pt of particle related to cluster found by ROI", numBinsEnergy, 0,
                                    maxEnergy);
  h_foundByNNOnlyparticlePt = new TH1F("h_foundByNNOnlyparticlePt", "Pt of particle related to cluster found by NN only",
                                       numBinsEnergy, 0, maxEnergy);
  h_foundByROIOnlyparticlePt = new TH1F("h_foundByROIOnlyparticlePt", "Pt of particle related to cluster found by ROI only",
                                        numBinsEnergy, 0, maxEnergy);

  h_particelPtLessThresh = new TH1F("h_particelPtLessThresh", "Number of valid particles with Pt less than threshold", numBinsEntries,
                                    0, numBinsEntries - 1);
  h_particelPtLessThreshFoundNN = new TH1F("h_particelPtLessThreshFoundNN",
                                           "Number of valid particles with Pt less than threshold found by NN", numBinsEntries, 0, numBinsEntries - 1);

  h_particleP = new TH1F("h_particleP", "P of particles with valid PDG number", numBinsEnergy, 0, maxEnergy);
  h_particlePWithTrack = new TH1F("h_particlePWithTrack", "P of valid particles with track", numBinsEnergy, 0, maxEnergy);
  h_particlePWithoutTrack = new TH1F("h_particlePWithoutTrack", "P of valid particles without track", numBinsEnergy, 0, maxEnergy);
  h_foundByNNparticleP = new TH1F("h_foundByNNparticleP", "P of particle related to cluster found by NN", numBinsEnergy, 0,
                                  maxEnergy);
  h_foundByROIparticleP = new TH1F("h_foundByROIparticleP", "P of particle related to cluster found by ROI", numBinsEnergy, 0,
                                   maxEnergy);
  h_foundByNNOnlyparticleP = new TH1F("h_foundByNNOnlyparticleP", "P of particle related to cluster found by NN only", numBinsEnergy,
                                      0, maxEnergy);
  h_foundByROIOnlyparticleP = new TH1F("h_foundByROIOnlyparticleP", "P of particle related to cluster found by ROI only",
                                       numBinsEnergy, 0, maxEnergy);

  int h3max = 10;
  m_h3bins = h3max * 10;
  h_positionClusters = new TH3F("h_positionClusters", "Signal cluster positions", m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max,
                                m_h3bins, -h3max, h3max);
  h_positionClusters->SetMarkerColor(kRed);
  h_positionClustersLostByROI = new TH3F("h_positionClustersLostByROI", "Signal cluster positions lost by ROI", m_h3bins, -h3max,
                                         h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersLostByROI->SetMarkerColor(kRed);
  h_positionClustersFoundByNN = new TH3F("h_positionClustersFoundByNN", "Signal cluster positions found by NN", m_h3bins, -h3max,
                                         h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersFoundByNN->SetMarkerColor(kRed);
  h_positionClustersHasNoTrack = new TH3F("h_positionClustersHasNoTrack", "Signal cluster positions has no track", m_h3bins, -h3max,
                                          h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersHasNoTrack->SetMarkerColor(kRed);
  h_positionClustersSignalVsNoTrack = new TH3F("h_positionClustersSignalVsNoTrack",
                                               "Signal cluster positions vs cluster has no track", m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersSignalVsNoTrack->SetMarkerColor(kRed);
  h_positionClustersSignalVsFoundByNN = new TH3F("h_positionClustersSignalVsFoundByNN",
                                                 "Signal cluster positions vs cluster found by NN", m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersSignalVsFoundByNN->SetMarkerColor(kRed);
  h_positionClustersSignalVsLostByROI = new TH3F("h_positionClustersSignalVsLostByROI",
                                                 "Signal cluster positions vs cluster lost by ROI", m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max, m_h3bins, -h3max, h3max);
  h_positionClustersSignalVsLostByROI->SetMarkerColor(kRed);
}

void PXDClusterRescueAnalysisModule::beginRun()
{
}

void PXDClusterRescueAnalysisModule::event()
{
  B2DEBUG(DEBUG_LEVEL, "##########");
  B2DEBUG(DEBUG_LEVEL, "Event: " << m_countEvents);
  B2DEBUG(DEBUG_LEVEL, "##########");

  // load required data
  StoreArray<ROIid> ROIids(m_ROIidColName);
  StoreArray<Track> Tracks(m_TracksColName);
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<PXDCluster> pxdClustersNN(m_PXDClusterNNColName);
  StoreArray<PXDDigit> pxdDigits;
  StoreArray<MCParticle> mcParticles;

  // load vxd geometry
  VXD::GeoCache& vxdGeometry = VXD::GeoCache::getInstance();

  // setup counters for found clusters by NN and/or ROI with attributes
  int foundByROI = 0;
  int foundByROIOnly = 0;
  int foundByNN = 0;
  int foundByNNOnly = 0;
  int foundByROIorNN = 0;
  int foundByROIandNN = 0;
  int countIsSignal = 0;
  int countHasTrack = 0;
  int foundByNNWithTrack = 0;
  int foundByROIWithTrack = 0;
  int countPtThresh = 0;
  int countPtThreshFoundNN = 0;

  // go through clusters and check whether they are found by ROI and/or NN cluster rescue
  for (int k = 0; k < pxdClusters.getEntries(); k++) {
    // check whether cluster has relation to valid mcparticle and is signal
    RelationVector<MCParticle> particleRelated = pxdClusters[k]->getRelationsTo<MCParticle>();
    bool clusterIsSignal = false;
    bool particleHasTrack = false;
    float particlePt = 0;
    float particleP = 0;
    for (unsigned int p = 0; p < particleRelated.size(); p++) {
      for (unsigned int pp = 0; pp < m_particleInclude.size(); pp++)
        if (particleRelated[p]->getPDG() == m_particleInclude[pp]) {
          // set signal flag on true and get impulse
          clusterIsSignal = true;
          particlePt = particleRelated[p]->getMomentum().Pt();
          particleP = particleRelated[p]->getMomentum().Mag();

          // check whether particle has track and should be found by ROI
          RelationVector<Track> trackRelated = particleRelated[p]->getRelationsFrom<Track>(m_TracksColName);
          if (trackRelated.size() > 0) particleHasTrack = true;
        }
    }

    if (clusterIsSignal) {
      countIsSignal++;
      if (particleHasTrack) countHasTrack++;
    } else {
      continue; // do not evaluate cluster which is not signal
    }

    // check relation to NN cluster
    bool relationToNN = false;
    RelationVector<PXDCluster> relatedClusters = pxdClusters[k]->getRelationsTo<PXDCluster>(m_PXDClusterNNColName);
    if (relatedClusters.size() == 1) relationToNN = true;
    else if (relatedClusters.size() > 1) B2ERROR("Found more than 1 relation to NN PXDClusters.");

    // check relation to ROI with going through ROIids and check whether all pixels of clusters are in a ROI
    bool relationToROI = false;
    RelationVector<PXDDigit> pxdDigitsRelated = pxdClusters[k]->getRelationsTo<PXDDigit>();

    for (int p = 0; p < ROIids.getEntries(); p++) {
      bool foundAllPixels = false;

      if (pxdClusters[k]->getSensorID() == ROIids[p]->getSensorID()) {
        foundAllPixels = true;

        for (unsigned int m = 0; m < pxdDigitsRelated.size(); m++) {
          if (!ROIids[p]->Contains(*pxdDigitsRelated[m])) {
            foundAllPixels = false;
          }
        }
      }

      if (foundAllPixels) {
        relationToROI = true;
      }
    }

    // get global cluster position
    VXD::SensorInfoBase pxdSensorInfo = vxdGeometry.getSensorInfo(pxdClusters[k]->getSensorID());
    TVector3 localPoint(pxdClusters[k]->getU(), pxdClusters[k]->getV(), 0);
    TVector3 globalPoint = pxdSensorInfo.pointToGlobal(localPoint);

    // fill global position to histogram
    h_positionClusters->Fill(globalPoint.X(), globalPoint.Y(), globalPoint.Z());
    if (not(particleHasTrack)) h_positionClustersHasNoTrack->Fill(globalPoint.X(), globalPoint.Y(), globalPoint.Z());

    // increment counters for histograms outside loop and fill pt and position histograms
    if (particlePt < m_particlePtThreshold) {
      countPtThresh++;
      if (relationToNN) countPtThreshFoundNN++;
    }

    if (relationToROI) {
      foundByROI++;
      if (particleHasTrack) foundByROIWithTrack++;
      h_foundByROIparticlePt->Fill(particlePt);
      h_foundByROIparticleP->Fill(particleP);
    }
    if (relationToNN) {
      foundByNN++;
      if (particleHasTrack) foundByNNWithTrack++;
      h_foundByNNparticlePt->Fill(particlePt);
      h_foundByNNparticleP->Fill(particleP);
      h_positionClustersFoundByNN->Fill(globalPoint.X(), globalPoint.Y(), globalPoint.Z());
    }
    if (not(relationToROI)) {
      h_positionClustersLostByROI->Fill(globalPoint.X(), globalPoint.Y(), globalPoint.Z());

      // store training variables to file
      int NumTrainingVariables = m_PXDClusterRescueNN.getNumTrainingVariables();
      std::vector<float> pxdClusterVariables(NumTrainingVariables);
      m_PXDClusterRescueNN.getPXDClusterTrainingVariables(pxdClusters[k], pxdClusterVariables);

      for (int p = 0; p < NumTrainingVariables; p++) {
        m_fileLostByROI << pxdClusterVariables[p];
        if (p == NumTrainingVariables - 1) m_fileLostByROI << std::endl;
        else m_fileLostByROI << " ";
      }
    }
    if (relationToNN && !relationToROI) {
      foundByNNOnly++;
      h_foundByNNOnlyparticlePt->Fill(particlePt);
      h_foundByNNOnlyparticleP->Fill(particleP);
    }
    if (!relationToNN && relationToROI) {
      foundByROIOnly++;
      h_foundByROIOnlyparticlePt->Fill(particlePt);
      h_foundByROIOnlyparticleP->Fill(particleP);
    }
    if (relationToNN && relationToROI)foundByROIandNN++;
    if (relationToNN || relationToROI) foundByROIorNN++;
  }

  // get sum of ROI pixels
  int countPixelsROI = 0;
  for (int k = 0; k < ROIids.getEntries(); k++) {
    countPixelsROI += (ROIids[k]->getMaxUid() - ROIids[k]->getMinUid() + 1) * (ROIids[k]->getMaxVid() - ROIids[k]->getMinVid() +
                      1); // +1 cause boundries of ROI are included
  }

  // get p, pt and pdg number of valid particles with and without tracks
  for (int k = 0; k < mcParticles.getEntries(); k++) {
    if (mcParticles[k]->hasStatus(MCParticle::c_PrimaryParticle)) { // if particle is primary, get pdg number
      h_particlesPDG->Fill(mcParticles[k]->getPDG());

      RelationVector<PXDCluster> clusterRelated = mcParticles[k]->getRelationsFrom<PXDCluster>();
      if (clusterRelated.size() > 0) { // if particle has relation to cluster, get pdg number
        h_particlesPDGCluster->Fill(mcParticles[k]->getPDG());
      }
    }

    for (unsigned int p = 0; p < m_particleInclude.size(); p++) {
      if (mcParticles[k]->getPDG() == m_particleInclude[p]) {
        // fill p and pt (all valid particles)
        float particlePt = mcParticles[k]->getMomentum().Pt();
        float particleP = mcParticles[k]->getMomentum().Mag();
        h_particlePt->Fill(particlePt);
        h_particleP->Fill(particleP);

        // fill p and pt based on related track
        RelationVector<Track> trackRelated = mcParticles[k]->getRelationsFrom<Track>(m_TracksColName);
        if (trackRelated.size() > 0) {
          h_particlePtWithTrack->Fill(particlePt);
          h_particlePWithTrack->Fill(particleP);
        } else {
          h_particlePtWithoutTrack->Fill(particlePt);
          h_particlePWithoutTrack->Fill(particleP);
        }
      }
    }
  }

  // histogram: number of data entries
  h_pxdClusters->Fill(pxdClusters.getEntries());
  h_pxdClustersSignal->Fill(countIsSignal);
  h_pxdClustersHasTrack->Fill(countHasTrack);
  h_pxdClustersNN->Fill(pxdClustersNN.getEntries());
  h_tracks->Fill(Tracks.getEntries());
  h_ROIids->Fill(ROIids.getEntries());
  h_ROIidsPixels->Fill(countPixelsROI);

  // histogram: clusters found by NN and/or ROI and relation to signal clusters
  h_foundByROI->Fill(foundByROI);
  h_foundByNN->Fill(foundByNN);
  h_foundByROIandNN->Fill(foundByROIandNN);
  h_foundByROIorNN->Fill(foundByROIorNN);
  h_foundByROIOnly->Fill(foundByROIOnly);
  h_foundByNNOnly->Fill(foundByNNOnly);
  h_foundByNNWithTrack->Fill(foundByNNWithTrack);
  h_foundByROIWithTrack->Fill(foundByROIWithTrack);

  h_particelPtLessThresh->Fill(countPtThresh);
  h_particelPtLessThreshFoundNN->Fill(countPtThreshFoundNN);

  // increment counters
  m_countEvents++;
}

void PXDClusterRescueAnalysisModule::endRun()
{
}

void PXDClusterRescueAnalysisModule::terminate()
{
  // write counter to file
  m_fileLog << "Count events: " << m_countEvents << std::endl;

  // process cluster position vs sth 3d histos
  float binContentSignal, binContentVs;
  for (int x = 0; x < m_h3bins + 2; x++) {
    for (int y = 0; y < m_h3bins + 2; y++) {
      for (int z = 0; z < m_h3bins + 2; z++) {
        // get bin contents
        binContentSignal = h_positionClusters->GetBinContent(x, y, z);

        // signal clusters vs no track
        binContentVs = h_positionClustersHasNoTrack->GetBinContent(x, y, z);
        if (isnan(binContentVs / binContentSignal)) h_positionClustersSignalVsNoTrack->SetBinContent(x, y, z, 0);
        else h_positionClustersSignalVsNoTrack->SetBinContent(x, y, z, binContentVs / binContentSignal);

        // signal clusters vs found by NN
        binContentVs = h_positionClustersFoundByNN->GetBinContent(x, y, z);
        if (isnan(binContentVs / binContentSignal)) h_positionClustersSignalVsFoundByNN->SetBinContent(x, y, z, 0);
        else h_positionClustersSignalVsFoundByNN->SetBinContent(x, y, z, binContentVs / binContentSignal);

        // signal clusters vs lost by ROI
        binContentVs = h_positionClustersLostByROI->GetBinContent(x, y, z);
        if (isnan(binContentVs / binContentSignal)) h_positionClustersSignalVsLostByROI->SetBinContent(x, y, z, 0);
        else h_positionClustersSignalVsLostByROI->SetBinContent(x, y, z, binContentVs / binContentSignal);
      }
    }
  }

  // write histograms to file
  TObjArray Hlist(0);
  Hlist.Add(h_pxdClusters);
  Hlist.Add(h_pxdClustersSignal);
  Hlist.Add(h_pxdClustersHasTrack);
  Hlist.Add(h_pxdClustersNN);
  Hlist.Add(h_tracks);
  Hlist.Add(h_ROIids);
  Hlist.Add(h_ROIidsPixels);

  Hlist.Add(h_foundByROI);
  Hlist.Add(h_foundByROIOnly);
  Hlist.Add(h_foundByROIWithTrack);

  Hlist.Add(h_foundByNN);
  Hlist.Add(h_foundByNNOnly);
  Hlist.Add(h_foundByNNWithTrack);
  Hlist.Add(h_foundByROIandNN);
  Hlist.Add(h_foundByROIorNN);

  Hlist.Add(h_particlesPDG);
  Hlist.Add(h_particlesPDGCluster);

  Hlist.Add(h_particlePt);
  Hlist.Add(h_particlePtWithTrack);
  Hlist.Add(h_particlePtWithoutTrack);
  Hlist.Add(h_foundByNNparticlePt);
  Hlist.Add(h_foundByROIparticlePt);
  Hlist.Add(h_foundByNNOnlyparticlePt);
  Hlist.Add(h_foundByROIOnlyparticlePt);

  Hlist.Add(h_particelPtLessThresh);
  Hlist.Add(h_particelPtLessThreshFoundNN);

  Hlist.Add(h_particleP);
  Hlist.Add(h_particlePWithTrack);
  Hlist.Add(h_particlePWithoutTrack);
  Hlist.Add(h_foundByNNparticleP);
  Hlist.Add(h_foundByROIparticleP);
  Hlist.Add(h_foundByNNOnlyparticleP);
  Hlist.Add(h_foundByROIOnlyparticleP);

  Hlist.Add(h_positionClusters);
  Hlist.Add(h_positionClustersLostByROI);
  Hlist.Add(h_positionClustersFoundByNN);
  Hlist.Add(h_positionClustersHasNoTrack);
  Hlist.Add(h_positionClustersSignalVsLostByROI);
  Hlist.Add(h_positionClustersSignalVsFoundByNN);
  Hlist.Add(h_positionClustersSignalVsNoTrack);

  Hlist.Write();

  // close files
  m_fileRoot->Close();
  m_fileLog.close();
  m_fileLostByROI.close();
}
