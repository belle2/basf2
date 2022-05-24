/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/eventTimeExtractor/SVDEventT0EstimatorModule.h>
#include <framework/datastore/RelationArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <framework/geometry/B2Vector3.h>
#include <cmath>
using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventT0Estimator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventT0EstimatorModule::SVDEventT0EstimatorModule() : Module()
{
  setDescription("This module estimates the EventT0 as the average of cluster time of SVD clusters associated to tracks. The EventT0 is set to NaN if there are not RecoTracks or there are not SVD clusters associated to tracks or RecoTrack pt < ptMin OR RecoTrack pz < pzMin. The EventT0 estimated is added to the temporaryEventT0s to the StoreObjPtr as EventT0Component that cointains: eventT0, eventT0_error, detector=SVD, algorithm, quality.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //* Definition of input parameters */
  addParam("RecoTracks", m_recoTracksName, "Name of the StoreArray with the input RecoTracks", string(""));
  addParam("EventT0", m_eventT0Name, "Name of the StoreObjPtr with the input EventT0", string(""));
  addParam("ptMinSelection", m_ptSelection, "Cut on minimum transverse momentum pt for RecoTrack selection", m_ptSelection);
  addParam("absPzMinSelection", m_absPzSelection,
           "Cut on minimum absolute value of the longitudinal momentum, abs(pz), for RecoTrack selection",
           m_absPzSelection);
}


SVDEventT0EstimatorModule::~SVDEventT0EstimatorModule()
{
}


void SVDEventT0EstimatorModule::initialize()
{
  B2DEBUG(20, "RecoTracks: " << m_recoTracksName);
  B2DEBUG(20, "EventT0: " << m_eventT0Name);

  /** Register the data object */
  m_eventT0.registerInDataStore();
  m_recoTracks.isRequired(m_recoTracksName);
}


void SVDEventT0EstimatorModule::event()
{

  double evtT0 = NAN;
  double evtT0_err = NAN;
  double clsTime_sum = 0;
  double clsTime_err_sum = 0;
  double quality = NAN;
  int N_cls = 0;

  // loop on recotracks
  for (const auto& recoTrack : m_recoTracks) {
    const B2Vector3D& p = recoTrack.getMomentumSeed();
    if (p.Perp() < m_ptSelection || std::fabs(p.Z()) < m_absPzSelection) continue;
    const vector<SVDCluster* >& svdClusters = recoTrack.getSVDHitList();
    B2DEBUG(20, "FITTED TRACK:   NUMBER OF SVD HITS = " << svdClusters.size());
    for (const SVDCluster* svdCluster : svdClusters) {
      clsTime_sum += svdCluster->getClsTime();
      clsTime_err_sum += (svdCluster->getClsTimeSigma() * svdCluster->getClsTimeSigma());
    }
    N_cls += svdClusters.size();
  }
  if (N_cls > 1) {
    quality = N_cls;
    evtT0 = clsTime_sum / N_cls;
    evtT0_err = std::sqrt(clsTime_err_sum / (N_cls * (N_cls - 1)));
  }
  EventT0::EventT0Component evtT0_comp(evtT0, evtT0_err, Const::SVD, m_algorithm, quality);
  if (m_eventT0.isValid()) {
    m_eventT0->addTemporaryEventT0(evtT0_comp);
    m_eventT0->setEventT0(evtT0_comp);
  }
}
