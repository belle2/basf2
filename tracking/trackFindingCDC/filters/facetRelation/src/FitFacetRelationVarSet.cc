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
#include <tracking/trackFindingCDC/numerics/Angle.h>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

FitFacetRelationVarSet::FitFacetRelationVarSet()
  : Super()
{
}

bool FitFacetRelationVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  bool extracted = Super::extract(ptrFacetRelation);
  if (not extracted or not ptrFacetRelation) return false;

  const CDCFacet* fromFacet = ptrFacetRelation->first;
  const CDCFacet* toFacet   = ptrFacetRelation->second;

  const UncertainParameterLine2D& fromFitLine = fromFacet->getFitLine();
  const UncertainParameterLine2D& toFitLine   = toFacet->getFitLine();

  LineCovariance fromCovariance = fromFitLine.lineCovariance();
  LineParameters fromParameters = fromFitLine.lineParameters();

  LineCovariance toCovariance   = toFitLine.lineCovariance();
  LineParameters toParameters   = toFitLine.lineParameters();

  Vector2D fromTangential = fromFacet->getStartToEndLine().tangential();
  Vector2D toTangential   = toFacet->getStartToEndLine().tangential();

  Vector2D tangential = Vector2D::average(fromTangential, toTangential);

  double fromMiddleCos = fromFacet->getStartToMiddleLine().tangential().cosWith(toTangential);
  double toMiddleCos   = fromTangential.cosWith(toFacet->getMiddleToEndLine().tangential());

  var<named("cos_delta")>() = fromTangential.cosWith(toTangential);

  var<named("from_middle_cos_delta")>() = fromMiddleCos;
  var<named("to_middle_cos_delta")>() = toMiddleCos;

  Vector2D frontWirePos2D = fromFacet->getStartWire().getRefPos2D();
  Vector2D backWirePos2D  = toFacet->getEndWire().getRefPos2D();
  {
    int nSteps = 0;
    UncertainParameterLine2D fitLine = FacetFitter::fit(*fromFacet, *toFacet, nSteps);
    double s = fitLine->lengthOnCurve(frontWirePos2D, backWirePos2D);
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
        B2INFO("from addr " << fromFacet);
        B2INFO("From cov " << std::endl << fromFacet->getFitLine().lineCovariance());
        B2INFO("From cov " << std::endl << fromFitLine.lineCovariance());
      }
    }
    {
      double phi0_var = toFacet->getFitLine().lineCovariance()(c_Phi0, c_Phi0);
      if (not std::isfinite(phi0_var)) {
        B2INFO("to addr " << toFacet);
        B2INFO("To cov " << std::endl << toFacet->getFitLine().lineCovariance());
        B2INFO("To cov " << std::endl << toFitLine.lineCovariance());
      }
    }

    var<named("phi0_from_sigma")>() = std::sqrt(fromCovariance(c_Phi0, c_Phi0));
    var<named("phi0_to_sigma")>() = std::sqrt(toCovariance(c_Phi0, c_Phi0));
    var<named("phi0_ref_sigma")>() = std::sqrt(fromCovariance(c_Phi0, c_Phi0) + toCovariance(c_Phi0, c_Phi0));
    var<named("phi0_ref_diff")>() = AngleUtil::normalised(toParameters(c_Phi0) - fromParameters(c_Phi0));
    var<named("phi0_ref_pull")>() =
      std::fabs(AngleUtil::normalised(toParameters(c_Phi0) - fromParameters(c_Phi0)) /
                std::sqrt((toCovariance(c_Phi0, c_Phi0) + fromCovariance(c_Phi0, c_Phi0))));

    LineParameters refParameters;
    refParameters(c_I) = (fromParameters(c_I) + toParameters(c_I)) / 2;
    refParameters(c_Phi0) = AngleUtil::average(fromParameters(c_Phi0), toParameters(c_Phi0));

    LineParameters relFromParameters;
    relFromParameters(c_I)    = fromParameters(c_I) - refParameters(c_I);
    relFromParameters(c_Phi0) = AngleUtil::normalised(fromParameters(c_Phi0) - refParameters(c_Phi0));

    LineParameters relToParameters;
    relToParameters(c_I)    = toParameters(c_I) - refParameters(c_I);
    relToParameters(c_Phi0) =  AngleUtil::normalised(toParameters(c_Phi0) - refParameters(c_Phi0));

    LineParameters relAvgParameters;
    LineCovariance avgCovariance;

    double chi2 = CovarianceMatrixUtil::average(relFromParameters, fromCovariance,
                                                relToParameters, toCovariance,
                                                relAvgParameters, avgCovariance);

    LineParameters avgParameters;

    avgParameters(c_I) = relAvgParameters(c_I) + refParameters(c_I);
    avgParameters(c_Phi0) = AngleUtil::normalised(relAvgParameters(c_Phi0) + refParameters(c_Phi0));

    var<named("chi2_comb")>() = chi2;
    var<named("phi0_comb_pull")>() = std::fabs(relAvgParameters(c_Phi0) /
                                               std::sqrt(avgCovariance(c_Phi0, c_Phi0)));
    var<named("phi0_comb_diff")>() = relAvgParameters(c_Phi0);
    var<named("phi0_comb_sigma")>() = std::sqrt(avgCovariance(c_Phi0, c_Phi0));

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
