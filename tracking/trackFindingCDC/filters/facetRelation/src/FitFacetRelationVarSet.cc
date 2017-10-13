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
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/LineParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitFacetRelationVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  if (not ptrFacetRelation) return false;

  const CDCFacet* fromFacet = ptrFacetRelation->first;
  const CDCFacet* toFacet   = ptrFacetRelation->second;

  const UncertainParameterLine2D& fromFitLine = fromFacet->getFitLine();
  const UncertainParameterLine2D& toFitLine   = toFacet->getFitLine();

  LineCovariance fromCov = fromFitLine.lineCovariance();
  LineParameters fromPar = fromFitLine.lineParameters();

  LineCovariance toCov   = toFitLine.lineCovariance();
  LineParameters toPar   = toFitLine.lineParameters();

  Vector2D fromTangential = fromFacet->getStartToEndLine().tangential();
  Vector2D toTangential   = toFacet->getStartToEndLine().tangential();

  Vector2D tangential = Vector2D::average(fromTangential, toTangential);

  double fromMiddleCos = fromFacet->getStartToMiddleLine().tangential().cosWith(toTangential);
  double toMiddleCos = fromTangential.cosWith(toFacet->getMiddleToEndLine().tangential());

  var<named("cos_delta")>() = fromTangential.cosWith(toTangential);

  var<named("from_middle_cos_delta")>() = fromMiddleCos;
  var<named("to_middle_cos_delta")>() = toMiddleCos;

  Vector2D frontWirePos2D = fromFacet->getStartWireHit().getRefPos2D();
  Vector2D backWirePos2D  = toFacet->getEndWireHit().getRefPos2D();
  {
    int nSteps = 0;
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet, nSteps);
    double s = fitLine->lengthOnCurve(frontWirePos2D, backWirePos2D);
    double alpha = fitLine->support().angleWith(fitLine->tangential());
    var<named("alpha_0")>() = alpha;
    var<named("chi2_0")>() = fitLine.chi2();
    var<named("chi2_0_per_s")>() = fitLine.chi2() / s;
    var<named("erf_0")>() = std::erf(fitLine.chi2() / 800);
    var<named("fit_0_phi0")>() = fitLine->tangential().phi();
    var<named("fit_0_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }

  {
    int nSteps = 1;
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet, nSteps);
    double s = fitLine->lengthOnCurve(frontWirePos2D, backWirePos2D);
    var<named("chi2_1")>() = fitLine.chi2();
    var<named("chi2_1_per_s")>() = fitLine.chi2() / s;
    var<named("fit_1_phi0")>() = fitLine->tangential().phi();
    var<named("fit_1_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }

  {
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet);
    double s = fitLine->lengthOnCurve(frontWirePos2D, backWirePos2D);
    var<named("chi2")>() = fitLine.chi2();
    var<named("chi2_per_s")>() = fitLine.chi2() / s;
    var<named("fit_phi0")>() = fitLine->tangential().phi();
    var<named("fit_cos_delta")>() = fitLine->tangential().cosWith(tangential);
  }

  // Combination fit
  {
    using namespace NLineParameterIndices;
    {
      double phi0_var = fromFacet->getFitLine().lineCovariance()(c_Phi0, c_Phi0);
      if (not std::isfinite(phi0_var)) {
        B2INFO("from addr " << *fromFacet);
        B2INFO("From cov " << std::endl << fromFacet->getFitLine().lineCovariance());
        B2INFO("From cov " << std::endl << fromFitLine.lineCovariance());
      }
    }
    {
      double phi0_var = toFacet->getFitLine().lineCovariance()(c_Phi0, c_Phi0);
      if (not std::isfinite(phi0_var)) {
        B2INFO("to addr " << *toFacet);
        B2INFO("To cov " << std::endl << toFacet->getFitLine().lineCovariance());
        B2INFO("To cov " << std::endl << toFitLine.lineCovariance());
      }
    }

    var<named("phi0_from_sigma")>() = std::sqrt(fromCov(c_Phi0, c_Phi0));
    var<named("phi0_to_sigma")>() = std::sqrt(toCov(c_Phi0, c_Phi0));
    var<named("phi0_ref_sigma")>() = std::sqrt(fromCov(c_Phi0, c_Phi0) + toCov(c_Phi0, c_Phi0));
    var<named("phi0_ref_diff")>() = AngleUtil::normalised(toPar(c_Phi0) - fromPar(c_Phi0));
    var<named("phi0_ref_pull")>() =
      std::fabs(AngleUtil::normalised(toPar(c_Phi0) - fromPar(c_Phi0)) /
                std::sqrt((toCov(c_Phi0, c_Phi0) + fromCov(c_Phi0, c_Phi0))));

    LineParameters avgPar;
    LineCovariance avgCov;
    double chi2 = LineUtil::average(fromPar, fromCov, toPar, toCov, avgPar, avgCov);

    LineParameters meanPar = (fromPar + toPar) / 2.0;
    meanPar(c_Phi0) = AngleUtil::average(fromPar(c_Phi0), toPar(c_Phi0));

    LineParameters relAvgPar = avgPar - meanPar;
    relAvgPar(c_Phi0) = AngleUtil::normalised(relAvgPar(c_Phi0));

    var<named("chi2_comb")>() = chi2;
    var<named("phi0_comb_pull")>() =
      std::fabs(relAvgPar(c_Phi0) / std::sqrt(avgCov(c_Phi0, c_Phi0)));
    var<named("phi0_comb_diff")>() = relAvgPar(c_Phi0);
    var<named("phi0_comb_sigma")>() = std::sqrt(avgCov(c_Phi0, c_Phi0));
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
