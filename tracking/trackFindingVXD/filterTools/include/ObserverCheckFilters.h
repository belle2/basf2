/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * this is a modified copy of                                             *
 * tracking/trackFindingVXD/filterTools/include/ObserverCheckMCPurity.h   *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

// contains also the helper struct SpacePointInfo
#include <tracking/dataobjects/ObserverInfo.h>

#include <tracking/spacePointCreation/SpacePoint.h>

// #include <memory> // unique_ptr


namespace Belle2 {

  /** this observer searches logs the response for each of SelectionVariables used in the filters
    If the pointer to the StoreArray is set the results will be put into the datastore
   */
  class ObserverCheckFilters {
  public:

    /** container that stores the results calculated for a selectionVariableName, has to be static due to the way the
      observers is called*/
    static ObserverInfo s_observerInfo;

    /** hold a storearray to have access to the datastore */
    static StoreArray<ObserverInfo> s_storeArray;

    /// empty constructor:
    ObserverCheckFilters() {};



    /** get a copy of a storearray */
    static bool initialize(const StoreArray<ObserverInfo>& aStoreArray)
    {
      ObserverCheckFilters::s_storeArray = aStoreArray;
      return true;
    }

    /** this function is needed by the Filters.h but has no task for this observer */
    template <class Var, class Range, typename ... types >
    static bool initialize(Var , Range, const types& ...)
    {
      return true;
    }


    /** convert a SpacePiont into a version that can be stored in the datastore */
    static SpacePointInfo convertSpacePoint(const SpacePoint& aSpacePoint)
    {
      SpacePointInfo spInfo;
      spInfo.setPosition(aSpacePoint.getPosition());
      spInfo.setPositionError(aSpacePoint.getPositionError());
      spInfo.setNormalizedLocalU(aSpacePoint.getNormalizedLocalU());
      spInfo.setNormalizedLocalV(aSpacePoint.getNormalizedLocalV());
      spInfo.setClustersAssignedU(aSpacePoint.getIfClustersAssigned().first);
      spInfo.setClustersAssignedV(aSpacePoint.getIfClustersAssigned().second);
      spInfo.setVxdID(aSpacePoint.getVxdID());
      spInfo.setSensorType((int)aSpacePoint.getType());
      spInfo.setQualityIndicator(aSpacePoint.getQualityEstimation());
      spInfo.setIsAssigned(aSpacePoint.getAssignmentState());
      return spInfo;
    }


    /** _static_ method used by the observed object to reset the stored values of the observer.
     */
    static void prepare(const SpacePoint& outerHit,
                        const SpacePoint& innerHit)
    {
      // discard all previous data
      s_observerInfo.clear();
      std::vector<SpacePointInfo> hitsinfo = { ObserverCheckFilters::convertSpacePoint(outerHit),
                                               ObserverCheckFilters::convertSpacePoint(innerHit)
                                             };
      s_observerInfo.setHits(hitsinfo);


      // collect purity for each particle attached to the hits
      // TODO: this is code used in the ObserverCheckMCPurity check if the following code
      /*
         std::vector<const Belle2::SpacePoint*> hits = {&outerHit, &innerHit};
         std::vector<Belle2::MCVXDPurityInfo> particlesFound;
      // TODO: check if it can cope with real data (no MC particles)!
      particlesFound = createPurityInfosVec(hits);
      // the dominating-particle is the uppermost one:
      auto purityPack = particlesFound.at(0).getPurity(); //that part does not look safe to be used on data!
      s_mainMCParticleID = purityPack.first;
      s_mainPurity = purityPack.second;
       */
    }

    /** _static_ method used by the observed object to terminate the observer.
     */
    template <  typename ... types >
    static void terminate(const types& ...)
    {
    }


    /** fill the storearray  */
    template <  typename ... types >
    static void collect(const types& ...)
    {
      // append a copy to the storearray
      s_storeArray.appendNew(s_observerInfo);
    }


    /** notifier which finds the mcParticles attached to given triplet of spacePoints and determines the purities for them. */
    template<class Var, class RangeType>
    static void notify(const Var&,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType&,
                       const typename Var::argumentType&,
                       const typename Var::argumentType&)
    {
      // create input-container for purity-check:
      //    std::vector<const Belle2::SpacePoint*> spacePoints = {&outerHit, &centerHit, &innerHit};

      generalNotify<Var, RangeType>(fResult, range);
    }


    /** notifier which finds the mcParticles attached to given pair of spacePoints and determines the purities for them. */
    template<class Var, class RangeType>
    static void notify(const Var&,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType&,
                       const typename Var::argumentType&)
    {
      //    // create input-container for purity-check:
      //    std::vector<const Belle2::SpacePoint*> spacePoints = {&outerHit, &innerHit};

      generalNotify<Var, RangeType>(fResult, range);
    }






  protected:

    /** unified part of the notifier function.
     *
     * Stores filter-specific things. */
    template<class Var, class RangeType>
    static void generalNotify(typename Var::variableType fResult,
                              const RangeType& range)
    {
      // store the data retrieved:
      FilterInfo info(Var().name() , double(fResult), range.contains(fResult), true);
      s_observerInfo.addFilterInfo(info);
    }
  };


}


