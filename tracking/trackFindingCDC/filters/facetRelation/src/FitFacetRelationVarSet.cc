/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/FitFacetRelationVarSet.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

FitFacetRelationVarSet::FitFacetRelationVarSet(const std::string& prefix)
  : Super(prefix)
{
}

bool FitFacetRelationVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  bool extracted = Super::extract(ptrFacetRelation);
  if (not extracted or not ptrFacetRelation) return false;

  const CDCFacet* fromFacet = ptrFacetRelation->first;
  const CDCFacet* toFacet = ptrFacetRelation->second;

  const Vector2D& fromTangential = fromFacet->getStartToEndLine().tangential();
  const Vector2D& toTangential = toFacet->getStartToEndLine().tangential();

  Vector2D tangential = Vector2D::average(fromTangential, toTangential);

  double fromMiddleCos = fromFacet->getStartToMiddleLine().tangential().cosWith(toTangential);
  double toMiddleCos = fromTangential.cosWith(toFacet->getMiddleToEndLine().tangential());

  var<named("cos_delta")>() = fromTangential.cosWith(toTangential);

  var<named("from_middle_cos_delta")>() = fromMiddleCos;
  var<named("to_middle_cos_delta")>() = toMiddleCos;

  {
    int nSteps = 0;
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet, nSteps);
    var<named("chi2_0")>() = fitLine.chi2();
    var<named("erf_0")>() = std::erf(fitLine.chi2() / 800);
    var<named("fit_0_phi0")>() = fitLine->tangential().phi();
    var<named("fit_0_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }

  {
    int nSteps = 1;
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet, nSteps);
    var<named("chi2_1")>() = fitLine.chi2();
    var<named("fit_1_phi0")>() = fitLine->tangential().phi();
    var<named("fit_1_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }

  {
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet);
    var<named("chi2")>() = fitLine.chi2();
    var<named("fit_phi0")>() = fitLine->tangential().phi();
    var<named("fit_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }


  // Fitter
  {
    CDCObservations2D observations2D(EFitPos::c_RecoPos, EFitVariance::c_Unit);
    observations2D.append(*fromFacet);
    observations2D.append(*toFacet);

    CDCTrajectory2D fittedTrajectory = CDCKarimakiFitter::getFitter().fit(std::move(observations2D));
    var<named("chi2_kari_unit")>() = fittedTrajectory.getChi2();
    var<named("abs_curv_unit")>() = std::fabs(fittedTrajectory.getCurvature());
  }

  {
    CDCObservations2D observations2D(EFitPos::c_RecoPos, EFitVariance::c_DriftLength);
    observations2D.append(*fromFacet);
    observations2D.append(*toFacet);

    CDCTrajectory2D fittedTrajectory = CDCKarimakiFitter::getFitter().fit(std::move(observations2D));
    var<named("chi2_kari_l")>() = fittedTrajectory.getChi2();
    var<named("abs_curv_l")>() = std::fabs(fittedTrajectory.getCurvature());
  }

  {
    CDCObservations2D observations2D(EFitPos::c_RecoPos, EFitVariance::c_Pseudo);
    observations2D.append(*fromFacet);
    observations2D.append(*toFacet);

    CDCTrajectory2D fittedTrajectory = CDCKarimakiFitter::getFitter().fit(std::move(observations2D));
    var<named("chi2_kari_pseudo")>() = fittedTrajectory.getChi2();
    var<named("abs_curv_pseudo")>() = std::fabs(fittedTrajectory.getCurvature());
  }

  {
    CDCObservations2D observations2D(EFitPos::c_RecoPos, EFitVariance::c_Proper);
    observations2D.append(*fromFacet);
    observations2D.append(*toFacet);

    CDCTrajectory2D fittedTrajectory = CDCKarimakiFitter::getFitter().fit(std::move(observations2D));
    var<named("chi2_kari_proper")>() = fittedTrajectory.getChi2();
    var<named("abs_curv_proper")>() = std::fabs(fittedTrajectory.getCurvature());
  }

  return true;
}
