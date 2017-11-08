/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/RelationSVDResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParamList.icc.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

RelationSVDResultVarSet::RelationSVDResultVarSet() : TrackFindingCDC::VarSet<RelationSVDResultVarNames>()
{
  addProcessingSignalListener(&m_advancer);
}

void RelationSVDResultVarSet::initialize()
{
  TrackFindingCDC::VarSet<RelationSVDResultVarNames>::initialize();

  ModuleParamList moduleParamList;
  m_advancer.exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<double>("direction").setValue(1);
}

bool RelationSVDResultVarSet::extract(const CKFToSVDResult* result)
{
  const RecoTrack* cdcTrack = result->getSeed();
  const genfit::MeasuredStateOnPlane& cdcMSoP = result->getSeedMSoP();

  const RecoTrack* svdTrack = result->getRelatedSVDRecoTrack();

  B2ASSERT("Should have a related SVD track at this stage;", svdTrack);

  const std::vector<const SpacePoint*> spacePoints = result->getHits();
  genfit::MeasuredStateOnPlane mSoP = result->getSeedMSoP();

  for (const SpacePoint* spacePoint : spacePoints) {
    if (std::isnan(m_advancer.extrapolateToPlane(mSoP, *spacePoint))) {
      return false;
    }
    m_kalmanStepper.kalmanStep(mSoP, *spacePoint);
  }

  const genfit::MeasuredStateOnPlane& firstCDCHit = result->getSeedMSoP();
  m_advancer.extrapolateToPlane(mSoP, firstCDCHit.getPlane());

  var<named("cdc_phi")>() = cdcMSoP.getMom().Phi();
  var<named("svd_phi")>() = mSoP.getMom().Phi();
  var<named("cdc_theta")>() = cdcMSoP.getMom().Theta();
  var<named("svd_theta")>() = mSoP.getMom().Theta();
  var<named("cdc_pt")>() = cdcMSoP.getMom().Pt();
  var<named("svd_pt")>() = mSoP.getMom().Pt();
  var<named("cdc_charge")>() = cdcMSoP.getCharge();
  var<named("svd_charge")>() = mSoP.getCharge();
  var<named("cdc_number_of_hits")>() = cdcTrack->getNumberOfCDCHits();
  var<named("svd_number_of_hits")>() = svdTrack->getNumberOfSVDHits();
  var<named("cdc_lowest_layer")>() = cdcTrack->getSortedCDCHitList().front()->getICLayer();
  var<named("svd_highest_layer")>() = svdTrack->getSortedSVDHitList().back()->getSensorID().getLayerNumber();

  return true;
}
