/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDQMBowingModule.h>

#include <alignment/dbobjects/VXDAlignment.h>
#include <framework/database/DBObjPtr.h>
#include <genfit/Track.h>
#include <mdst/dataobjects/Track.h>
#include <pxd/reconstruction/PXDRecoHit.h>
//#include <tracking/dbobjects/ROICalculationParameters.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <vxd/geometry/GeoCache.h>
#include "TDirectory.h"


using namespace Belle2;

REG_MODULE(PXDDQMBowing);

PXDDQMBowingModule::PXDDQMBowingModule() : HistoModule()
{
  // Set module properties
  setDescription(R"DOC(Create basic histograms for PXD bowing monitoring)DOC");

  /// tips from xWiki DQM manual for development
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("particleList", m_particleListName, "Name of the particle list to which the module is applied", m_particleListName);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where the histogram will be placed",
           std::string("PXDBow"));

  addParam("outputRecoTracksArrayName", m_outputRecoTracksArrayName, "Name of the StoreArray with the recoTracks without PXD hits",
           m_outputRecoTracksArrayName);

  addParam("cutResU", m_cutResU, "Cut value on |res u| (upper limit)", 0.04);
  addParam("cutP", m_cutP, "Cut on the track momentum (lower limit)", 1.0);
  addParam("cutD0", m_cutD0, "Cut on |d0| (upper limit)", 0.1);
  addParam("cutZ0", m_cutZ0, "Cut on |z0| (upper limit)", 0.1);

  addParam("rangeV", m_rangeV, "Upper edge of the res v histogram symmetrical range", 0.1);
  addParam("rangeS", m_rangeS, "Upper edge of the sagitta histogram symmetrical range", 0.1);
  addParam("binsV", m_binsV, "Number of bins of the res v histogram", 100);
  addParam("binsS", m_binsS, "Number of bins of the sagitta histogram", 100);
}

void PXDDQMBowingModule::initialize()
{
  /// call the define histogram function
  REG_HISTOGRAM;

  /// register the required array as optional so validation succeeds also for cases where they are not available, however the module will not do anything without them
  m_recoTracks.isOptional();
  m_ParticleList.isOptional(m_particleListName);

  if (m_recoTracks.isValid()) {
    m_outputRecoTracks.registerInDataStore(m_outputRecoTracksArrayName);
    RecoTrack::registerRequiredRelations(m_outputRecoTracks);
  }

  /// get the bowing amplitude from the aligment
  DBObjPtr<VXDAlignment> alignment;
  VXD::GeoCache& geometry(VXD::GeoCache::getInstance());
  std::vector<VxdID> sensors = geometry.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geometry.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD || aVxdID.getSensorNumber() != 1) continue;
    double sensor_length = info.getLength();
    double sensor_alpha = alignment->get(aVxdID, VXDAlignment::dAlpha);
    m_dwAlignment[aVxdID] = sensor_alpha * sensor_length;
  }
}

void PXDDQMBowingModule::beginRun()
{
  ///reset histograms at the beginning of each run
  for (auto& h : m_hResV) if (h.second) h.second->Reset();
  for (auto& h : m_hSagitta) if (h.second) h.second->Reset();

}

void PXDDQMBowingModule::event()
{
  if (!m_recoTracks.isValid()) {
    B2INFO("RecoTrack array is missing, no residuals");
    return;
  }
  if (!m_ParticleList.isValid()) {
    B2INFO("Particle array is missing, no residuals");
    return;
  }

  /// loop on particle list
  auto nParticles = m_ParticleList->getListSize();
  for (unsigned int iParticle = 0; iParticle < nParticles; ++iParticle) {
    try {
      auto particle = m_ParticleList->getParticle(iParticle);
      auto b2track = particle->getTrack();
      if (!b2track) {
        B2ERROR("No Track for particle.");
        continue;
      }

      /// selection: high momentum track from IP
      const TrackFitResult* fitResult = b2track->getTrackFitResultWithClosestMass(Const::pion);
      ROOT::Math::XYZVector mom = fitResult->getMomentum();
      auto px = mom.X();
      auto py = mom.Y();
      auto pz = mom.Z();
      auto p = TMath::Sqrt(px * px + py * py + pz * pz);
      if (p < m_cutP) continue;
      Helix helix = fitResult->getHelix();
      auto d0 = helix.getD0();
      auto z0 = helix.getZ0();
      if (TMath::Abs(d0) > m_cutD0 || TMath::Abs(z0) > m_cutZ0) continue;

      auto recoTrack = b2track->getRelatedTo<RecoTrack>();
      if (!recoTrack) {
        B2ERROR("No RecoTrack for Track");
        continue;
      }
      /// copy track without hits
      auto noPXDTrack = recoTrack->copyToStoreArray(m_outputRecoTracks);
      int sortingPar = 0;
      /// add SVD hits
      for (auto hit : recoTrack->getSortedSVDHitList()) {
        noPXDTrack->addSVDHit(hit, sortingPar);
        sortingPar++;
      }
      /// add CDC hits
      for (auto hit : recoTrack->getSortedCDCHitList()) {
        noPXDTrack->addCDCHit(hit, sortingPar);
        sortingPar++;
      }
      /// refit the track without PXD hits
      Belle2::TrackFitter fitter;
      fitter.fit(*noPXDTrack);
      /// get the original genfit track to get the PXDhits
      genfit::Track& track = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);
      for (unsigned int i = 0; i < track.getNumPoints() - 1; ++i) {
        if (!track.getPoint(i)->hasRawMeasurements()) continue;

        Belle2::PXDRecoHit* hit = dynamic_cast<Belle2::PXDRecoHit*>(track.getPoint(i)->getRawMeasurement(0));
        if (!hit) continue;

        auto fitterInfo = track.getPoint(i)->getFitterInfo();
        if (fitterInfo) {
          auto vxdid = Belle2::VxdID(hit->getPlaneId());
          if (vxdid.getSensorNumber() != 1) continue;
          auto plane = fitterInfo->getPlane();
          bool biased = true;
          auto state = fitterInfo->getFittedState(biased).getState();
          auto residual = fitterInfo->getResidual(0, biased).getState();

          auto hitposU = state[3] + residual[0];
          auto hitposV = state[4] + residual[1];

          auto noPXDState = genfit::StateOnPlane(noPXDTrack->getMeasuredStateOnPlaneFromFirstHit());
          noPXDState.extrapolateToPlane(plane);

          auto noPXDhitPredU = noPXDState.getState()[3];
          auto noPXDhitPredV = noPXDState.getState()[4];

          auto noPXDhitPredVp = noPXDState.getState()[2];

          auto residualU = hitposU - noPXDhitPredU;

          if (TMath::Abs(residualU) > m_cutResU) continue;
          auto residualV = hitposV - noPXDhitPredV;
          auto residualW = residualV / noPXDhitPredVp;
          auto sagitta = residualW + m_dwAlignment[vxdid];

          if (m_hResV[vxdid]) m_hResV[vxdid]->Fill(residualV);
          if (m_hSagitta[vxdid] && hitposV < -2) m_hSagitta[vxdid]->Fill(sagitta);
        }
      }
    }/// end try
    catch (...) {
      B2ERROR("Some error in Residual Collector");
    }
  }/// end loop on particle
}

void PXDDQMBowingModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  VXD::GeoCache& vxdGeometry(VXD::GeoCache::getInstance());
  //  auto vxdGeometry = VXD::GeoCache::getInstance();
  std::vector<VxdID> sensors = vxdGeometry.getListOfSensors();
  std::sort(sensors.begin(), sensors.end());  // make sure it is our natural order
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD
        || avxdid.getSensorNumber() != 1) continue;  /** we are only interested in forward PXD sensor */

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    m_hResV[avxdid] = new TH1F("resV_" + buff, "resV " + buff, m_binsV, -m_rangeV, m_rangeV);
    m_hSagitta[avxdid] = new TH1F("sagitta_" + buff, "sagitta " + buff, m_binsS, -m_rangeS, m_rangeS);
  }
  oldDir->cd();
}


