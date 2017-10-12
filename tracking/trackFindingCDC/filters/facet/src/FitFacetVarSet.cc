/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/FitFacetVarSet.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/numerics/Quadratic.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  if (not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  const Vector2D startWirePos2D = facet.getStartWireHit().getRefPos2D();
  const Vector2D middleWirePos2D = facet.getMiddleWireHit().getRefPos2D();
  const Vector2D endWirePos2D = facet.getEndWireHit().getRefPos2D();

  // 0 step fit
  {
    constexpr const int nSteps = 0;
    const double chi2_0 = FacetFitter::fit(facet, nSteps);
    const UncertainParameterLine2D& fitLine = facet.getFitLine();
    const double s = fitLine->lengthOnCurve(startWirePos2D, endWirePos2D);

    var<named("chi2_0")>() = chi2_0;
    var<named("chi2_0_per_s")>() = chi2_0 / s;
    var<named("fit_0_phi0")>() = fitLine->tangential().phi();
    var<named("fit_0_phi0_sigma")>() = std::sqrt(fitLine.variance(ELineParameter::c_Phi0));
  }

  // 1 step fit
  {
    constexpr const int nSteps = 1;
    const double chi2_1 = FacetFitter::fit(facet, nSteps);
    const UncertainParameterLine2D& fitLine = facet.getFitLine();
    const double s = fitLine->lengthOnCurve(startWirePos2D, endWirePos2D);

    var<named("chi2_1")>() = chi2_1;
    var<named("chi2_1_per_s")>() = chi2_1 / s;
    var<named("fit_1_phi0")>() = fitLine->tangential().phi();
    var<named("fit_1_phi0_sigma")>() = std::sqrt(fitLine.variance(ELineParameter::c_Phi0));
  }

  // N step fit
  {
    const double chi2 = FacetFitter::fit(facet);
    const UncertainParameterLine2D& fitLine = facet.getFitLine();
    const double s = fitLine->lengthOnCurve(startWirePos2D, endWirePos2D);

    var<named("chi2")>() = chi2;
    var<named("chi2_per_s")>() = chi2 / s;

    // Heuristic flattening functions. Factor is chosen by hand for some experimentation here.
    constexpr const double erfWidth = 120.0;
    constexpr const double tanhWidth = 1.64 * erfWidth;

    var<named("erf")>() = std::erf(chi2 / erfWidth);
    var<named("tanh")>() = std::tanh(chi2 / tanhWidth);
    var<named("fit_phi0")>() = fitLine->tangential().phi();
    var<named("fit_phi0_sigma")>() = std::sqrt(fitLine.variance(ELineParameter::c_Phi0));

    const CDCRLWireHit& startRLWireHit = facet.getStartRLWireHit();
    const CDCRLWireHit& middleRLWireHit = facet.getMiddleRLWireHit();
    const CDCRLWireHit& endRLWireHit = facet.getEndRLWireHit();

    const double startL = startRLWireHit.getSignedRefDriftLength();
    const double middleL = middleRLWireHit.getSignedRefDriftLength();
    const double endL = endRLWireHit.getSignedRefDriftLength();

    const double startDistance = fitLine->distance(startWirePos2D) - startL;
    const double middleDistance = fitLine->distance(middleWirePos2D) - middleL;
    const double endDistance = fitLine->distance(endWirePos2D) - endL;

    var<named("start_distance")>() = startDistance;
    var<named("middle_distance")>() = middleDistance;
    var<named("end_distance")>() = endDistance;
    var<named("d2")>() = square(startDistance) + square(middleDistance) + square(endDistance);
  }
  return true;
}
