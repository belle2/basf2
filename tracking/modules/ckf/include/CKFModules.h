/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/FindletModule.h>
#include <tracking/ckf/svd/findlets/CKFToSVDFindlet.h>
#include <tracking/ckf/pxd/findlets/CKFToPXDFindlet.h>
#include <tracking/ckf/cdc/findlets/CKFToCDCFindlet.h>
#include <tracking/ckf/svd/findlets/CKFToSVDSeedFindlet.h>
#include <tracking/ckf/cdc/findlets/CKFToCDCFromEclFindlet.h>

#include <tracking/dbobjects/SVDToCDCCKFParameters.h>

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>

#include <boost/variant/variant.hpp>


namespace Belle2 {
  /**
   * Combinatorial Kalman Filter used for extrapolating CDC tracks into SVD and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToSVDSpacePointCKFModule : public TrackingUtilities::FindletModule<CKFToSVDFindlet> {

  public:
    /// Set description
    CDCToSVDSpacePointCKFModule()
    {
      setDescription("Combinatorial Kalman Filter used for extrapolating CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

  /**
   * Seed-finding combinatorial Kalman Filter that combines every RecoTrack with every
   * SVD track, then filters the combinations.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToSVDSeedCKFModule : public TrackingUtilities::FindletModule<CKFToSVDSeedFindlet> {

  public:
    /// Set description
    CDCToSVDSeedCKFModule()
    {
      setDescription("Combinatorial Kalman Filter used for merging existing CDC tracks and SVD tracks.");
    }
  };

  /**
   * Combinatorial Kalman Filter that extrapolates every RecoTrack into the PXD
   * and collects space points.
   * All implementation is done in the corresponding findlet.
   */
  class ToPXDCKFModule : public TrackingUtilities::FindletModule<CKFToPXDFindlet> {

  public:
    /// Set description
    ToPXDCKFModule()
    {
      setDescription("Combinatorial Kalman Filter used for extrapolating SVD/CDC tracks into "
                     "PXD and create merged tracks.");
    }
  };

  /**
   * Combinatorial Kalman Filter that extrapolates every RecoTrack into the CDC
   * and collects wire hits.
   * All implementation is done in the corresponding findlet.
   */
  class ToCDCCKFModule : public TrackingUtilities::FindletModule<CKFToCDCFindlet> {

  public:
    /// Set description
    ToCDCCKFModule() : TrackingUtilities::FindletModule<CKFToCDCFindlet>({"CDCWireHitVector"})
    {
      setDescription("Combinatorial Kalman Filter used for extrapolating SVD tracks into "
                     "CDC and create merged tracks.");

      // add warnings for all parameters which will be overridden by the payload in the beginRun() function
      std::vector<std::string> payloadVarNames = {"maximalDeltaPhi", "firstActiveCDCLayer", "maximalLayerJump", "maximalLayerJumpBackwardSeed", "pathMaximalCandidatesInFlight", "stateMaximalHitCandidates", "stateBasicFilterParameters"};

      for (const auto& varName : payloadVarNames) {
        auto typeInfo = getParamList().getParameterTypeInfo(varName);

        ModuleParamBase* p  = nullptr;
        if (typeInfo == "int") {
          p = &(getParamList().getParameter<int>(varName));
        } else if (typeInfo == "double") {
          p = &(getParamList().getParameter<double>(varName));
        } else if (typeInfo == "float") {
          p = &(getParamList().getParameter<float>(varName));
        } else if (typeInfo == "unsigned long int") {
          p = &(getParamList().getParameter<unsigned long int>(varName));
        } else if (typeInfo == "dict(str -> variant(bool, int, float, str, list(str)))") {
          using variantType = boost::variant<bool, int, double, std::string, std::vector<std::string> >;
          p = &(getParamList().getParameter< std::map<std::string, variantType> >(varName));
        } else {
          B2FATAL("Type " << typeInfo << " not supported ");
        }
        // the pointer should be safe as the getParameter<>() function throws an exception if parameter is not present (not sure if should be caught in constructor)
        p->setDescription("[WARNING: may be overridden by payload] " + p->getDescription());

      }

    }



    // override module parameter
    void beginRun()
    {

      DBObjPtr<SVDToCDCCKFParameters> payload;

      if (!payload.isValid()) {
        B2FATAL("ToCDCCKFModule: DB payload 'SVDToCDCCKFParameters' not found or not valid for current run.");
      }

      auto& p = Belle2::Module::getParam<int>("firstActiveCDCLayer");
      p.setValue(payload->getStateCreatorFirstCDCLayer());

      // need to be set before TrackingUtilities::FindletModule<CKFToCDCFindlet>::beginRun()
      using variantType = boost::variant<bool, int, double, std::string, std::vector<std::string> >;
      std::map<std::string, variantType>& value =
        Belle2::Module::getParam< std::map<std::string, variantType> >("stateBasicFilterParameters").getValue();
      value["maximalArcLengthDistance"] = payload->getMaxArcLengthRoughCDCStateFilter();


      TrackingUtilities::FindletModule<CKFToCDCFindlet>::beginRun();

    }
  };




  /**
   * Combinatorial Kalman Filter that extrapolates every ECLShower into the CDC
   * and collects wire hits.
   * All implementation is done in the corresponding findlet.
   */
  class ToCDCFromEclCKFModule : public TrackingUtilities::FindletModule<CKFToCDCFromEclFindlet> {

  public:
    /// Set description
    ToCDCFromEclCKFModule() : TrackingUtilities::FindletModule<CKFToCDCFromEclFindlet>({"CDCWireHitVector"})
    {
      setDescription("Combinatorial Kalman Filter used for extrapolating ECL showers into "
                     "CDC and create merged tracks.");
    }
  };
}
