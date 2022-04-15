/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDEventT0EstimatorModule.h>

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
  setDescription("");
  //* Definition of input parameters */
  addParam("RecoTracks", m_recoTracks, "StoreArray with the input recotracks", string("RecoTracks"));
  addParam("EventT0", m_eventT0, "StoreObjPtr with the input EventT0", string("EventT0"));
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

  double evtT0 = 0;
  double evtT0_err = NAN;
  double quality = -2;
  const string& algorithm = "cls_time_average";
  for (const auto& recoTrack : recoTracks) {
    RelationVector<Track> trk = DataStore::getRelationsWithObj<Track>(&recoTrack);
    if (trk.size() == 0) evtT0 = NAN;
    else {
      const TrackFitResult*  tfr = trk[0]->getTrackFitResultWithClosestMass(Const::pion);
      double pt = tfr->getMomentum().Perp();
      TVector3 p = tfr->getMomentum();
      double pz = p[2];
      const vector<SVDCluster* > svdClusters = recoTrack.getSVDHitList();
      if (svdClusters.size() == 0 || (pt < 0.25 && pz < 0.1)) evtT0 = NAN;
      else {
        evtT0 = eventT0Estimator(svdClusters);
        quality = svdClusters.size();
      }
    }
  }
  EventT0::EventT0Component evtT0_comp(evtT0, evtT0_err, Const::SVD, algorithm, quality);
  eventT0->addTemporaryEventT0(evtT0_comp);
}


void SVDEventT0EstimatorModule::endRun()
{
}


void SVDEventT0EstimatorModule::terminate()
{
}

double SVDEventT0EstimatorModule::eventT0Estimator(const vector<SVDCluster* > svdClusters)
{
  double clsTime_sum = 0;
  for (unsigned int i = 0; i < svdClusters.size(); i++) {
    double clsTime = svdClusters[i]->getClsTime();
    clsTime_sum += clsTime;
  }
  double evtT0 = clsTime_sum / svdClusters.size();
  return evtT0;
}






























