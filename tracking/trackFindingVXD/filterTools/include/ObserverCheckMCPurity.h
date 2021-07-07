/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/MCVXDPurityInfo.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h> // determines purity
#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <vector>
#include <map>
#include <string>

#include <TTree.h>

namespace Belle2 {

  /** this observer searches for mcParticles attached to the hits given and stores the information found to be retrieved later. */
  class ObserverCheckMCPurity {
  public:

    /** stores the results calculated (->value) for a selectionVariableName (->Key). */
// //   static std::map<std::string, std::unique_ptr<double>> s_results;
    static std::map<std::string, double*> s_results;


    /** stores if hits were accepted (->value) for a selectionVariableName (->Key). */
// //   static std::map<std::string, std::unique_ptr<bool>> s_wasAccepted;
    static std::map<std::string, bool*> s_wasAccepted;


    /** stores if the filter was actually used this time (->value) for a selectionVariableName (->Key). */
// //   static std::map<std::string, std::unique_ptr<bool>> s_wasUsed;
    static std::map<std::string, bool*> s_wasUsed;

    /** a ttree to store all the collected data. */
    static TTree* s_ttree;

    /** stores the outer hit of a two-hit-combination. */
    static SpacePoint  s_outerHitOfTwo;

    /** stores the inner hit of a two-hit-combination. */
    static SpacePoint  s_innerHitOfTwo;

    /** stores the outer hit of a three-hit-combination. */
    static SpacePoint  s_outerHitOfThree;

    /** stores the center hit of a three-hit-combination. */
    static SpacePoint  s_centerHitOfThree;

    /** stores the inner hit of a three-hit-combination. */
    static SpacePoint  s_innerHitOfThree;

    /** dominating mcParticleID. */
    static int s_mainMCParticleID;

    /** purity for the dominating particleID. */
    static double s_mainPurity;


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


    /// empty constructor:
    ObserverCheckMCPurity() {};



    /** _static_ method used by the observed object to initialize the observer.
     *
     * will be called once per Filter containing a single SelectionVariable and its range.
     */
    template <class Var, class Range, typename ... types >
    static bool initialize(Var var, Range, const types& ...)
    {
      auto varName = var.name();

      auto newResult = new double;
      s_results.insert(std::make_pair(varName, newResult));
      if (s_ttree != nullptr) s_ttree->Branch((varName + std::string("_val")).c_str(), newResult, TBranchLeafType(newResult));

      auto newAccepted = new bool(false);
      s_wasAccepted.insert(std::make_pair(varName, newAccepted));
      if (s_ttree != nullptr) s_ttree->Branch((varName + std::string("_accepted")).c_str(), newAccepted, TBranchLeafType(newAccepted));

      auto newWasUsed = new bool;
      s_wasUsed.insert(std::make_pair(varName, newWasUsed));
      if (s_ttree != nullptr) s_ttree->Branch((varName + std::string("_used")).c_str(), newWasUsed, TBranchLeafType(newWasUsed));

      return true;
    }


    /** version for 2-hit-combinations. */
    static bool initialize(TTree* attree)
    {
      s_ttree = attree;

      if (s_ttree != nullptr) {
        // added some protection for creating branches twice, as the observer is initialized recursively
        if (!s_ttree->GetBranch("outerHit")) s_ttree->Branch("outerHit", &s_outerHitOfTwo);
        if (!s_ttree->GetBranch("innerHit")) s_ttree->Branch("innerHit", &s_innerHitOfTwo);

        if (!s_ttree->GetBranch("mcParticleID")) s_ttree->Branch("mcParticleID", &s_mainMCParticleID);
        if (!s_ttree->GetBranch("mcPurity")) s_ttree->Branch("mcPurity", &s_mainPurity);
      } else {
        return false;
      }

      return true;
    }

//  /** _static_ method used by the observed object to initialize the observer (partial template specialization of the general version).
//   */
//  template <class Var, typename ... types >
//  static void initialize( const types& ... args);
//

    /** _static_ method used by the observed object to reset the stored values of the observer.
     */
    static void prepare(const SpacePoint& outerHit,
                        const SpacePoint& innerHit)
    {
      // store the hits:
      s_outerHitOfTwo = outerHit;
      s_innerHitOfTwo = innerHit;

      // collect purity for each particle attached to the hits
      std::vector<const Belle2::SpacePoint*> hits = {&outerHit, &innerHit};
//    hits.push_back(&outerHit);
//    hits.push_back(&innerHit);
      std::vector<Belle2::MCVXDPurityInfo> particlesFound;
      particlesFound = createPurityInfosVec(hits);
      // the dominating-particle is the uppermost one:
      auto purityPack = particlesFound.at(0).getPurity();
      s_mainMCParticleID = purityPack.first;
      s_mainPurity = purityPack.second;

      // reset the wasUsed-flag to catch when filters are not triggered.
      for (auto& entry : s_wasUsed) {
        *(entry.second) = false;
      }
    }


    /** _static_ method used by the observed object to terminate the observer.
     */
    template <  typename ... types >
    static void terminate(const types& ...)
    {
      s_results.clear();
      s_wasAccepted.clear();
      s_wasUsed.clear();
    }


    /** fill the tree. */
    template <  typename ... types >
    static void collect(const types& ...)
    {
      // TODO:
// // //    // collect purity for each particle attached to the hits
// // //    std::vector<Belle2::MCVXDPurityInfo> particlesFound;
// // //    particlesFound = createPurityInfosVec(hits);

      s_ttree->Fill();
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
      auto varName = Var().name();
      *(s_results.at(varName)) = double(fResult);
      *(s_wasUsed.at(varName)) = true;
      *(s_wasAccepted.at(varName)) = range.contains(fResult);
    }
  };


  /** _static_ method used by the observed object to initialize the observer.
   */
//   template <class Var, typename ... types >
//   static void initialize( TFile * rootFile, const types& ... args)
//   {
//  ObserverCheckMCPurity::initialize( args ...);
//
//  for(auto& entry : ObserverCheckMCPurity::s_wasUsed) {
//    *(entry.second) = false;
// //     rootFile->Branch("expNo", &(m_data2Hit.expNo));
//  }
//   }
}


