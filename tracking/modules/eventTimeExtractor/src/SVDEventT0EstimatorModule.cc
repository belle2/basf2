/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/eventTimeExtractor/SVDEventT0EstimatorModule.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventT0Estimator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventT0EstimatorModule::SVDEventT0EstimatorModule() : Module()
{
  setDescription("This module estimates the EventT0 as the average of cluster time of SVD clusters associated to tracks. The EventT0 is set to NaN if there are not RecoTracks or there are not SVD clusters associated to tracks or RecoTrack pt < ptMin OR RecoTrack pz < pzMin. The EventT0 estimated is added to the temporaryEventT0s to the StoreObjPtr as EventT0Component that cointains: eventT0, eventT0_error, detector=SVD, algorithm, quality.");
  //* Definition of input parameters */
  addParam("RecoTracks", m_recoTracks, "StoreArray with the input RecoTracks", string(""));
  addParam("EventT0", m_eventT0, "StoreObjPtr with the input EventT0", string(""));
  addParam("ptMin", m_pt, "Cut on minimum transverse momentum pt for RecoTrack selection", m_pt);
  addParam("pzMin", m_pz, "Cut on minimum longitudinal momentum pz for RecoTrack selection", m_pz);
}


SVDEventT0EstimatorModule::~SVDEventT0EstimatorModule()
{
}


void SVDEventT0EstimatorModule::initialize()
{
  B2DEBUG(10, "RecoTracks: " << m_recoTracks);
  B2DEBUG(10, "EventT0: " << m_eventT0);

  StoreArray<RecoTrack> rTracks(m_recoTracks);
  StoreObjPtr<EventT0> eT0(m_eventT0);
  rTracks.isRequired();
  eT0.isRequired();
}

void SVDEventT0EstimatorModule::beginRun()
{
}


void SVDEventT0EstimatorModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracks);
  StoreObjPtr<EventT0> eventT0(m_eventT0);

  m_evtT0 = NAN;
  m_evtT0_err = NAN;
  double clsTime_sum = 0;
  double clsTime_err_sum = 0;
  m_quality = NAN;
  int N_cls = 0;

  // loop on recotracks
  for (const auto& recoTrack : recoTracks) {
    if (! recoTrack.wasFitSuccessful()) continue;
    TVector3 p = recoTrack.getMomentumSeed();
    double pt = p.Perp();
    double pz = p[2];
    const vector<SVDCluster* > svdClusters = recoTrack.getSVDHitList();
    if (svdClusters.size() == 0) continue;
    B2DEBUG(40, "FITTED TRACK:   NUMBER OF SVD HITS = " << svdClusters.size());
    if (pt < m_pt || abs(pz) < m_pz) continue;
    for (unsigned int i = 0; i < svdClusters.size(); i++) {
      double clsTime = svdClusters[i]->getClsTime();
      double clsTime_err = svdClusters[i]->getClsTimeSigma();
      clsTime_sum += clsTime;
      clsTime_err_sum += clsTime_err * clsTime_err;
    }
    N_cls += svdClusters.size();
  }
  if (N_cls > 0) {
    m_quality = N_cls;
    m_evtT0 = clsTime_sum / N_cls;
    m_evtT0_err = std::sqrt(clsTime_err_sum / (N_cls * (N_cls - 1)));
  }
  EventT0::EventT0Component evtT0_comp(m_evtT0, m_evtT0_err, Const::SVD, m_algorithm, m_quality);
  eventT0->addTemporaryEventT0(evtT0_comp);
}


void SVDEventT0EstimatorModule::endRun()
{
}


void SVDEventT0EstimatorModule::terminate()
{
}

