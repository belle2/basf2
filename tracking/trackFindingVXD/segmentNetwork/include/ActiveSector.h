/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/FullSecID.h>

// C++-std:
#include <vector>
#include <string>

namespace Belle2 {

  /** The ActiveSector Class.
   *
   * associated with static sector to be able to access filter cutoffs.
   * It contains a vector of hits associated with it and allows to determine the related inner sectors.
   *
   */
  template<class StaticSectorType, class HitType>
  class ActiveSector {
  protected:
    /** ************************* DATA MEMBERS ************************* */

    /** Pointer to real sector after design of SectorMap */
    const StaticSectorType* m_staticSector;

    /** stores indices of all associated Hits */
    std::vector<HitType*> m_hits;


  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** Default constructor for root compatibility */
    ActiveSector(): m_staticSector(NULL) {}

    /** Constructor.
    *      //      * @param staticSector pointer to static sector associated with this one.
     *      //      */
    ActiveSector(const StaticSectorType* staticSector):
      m_staticSector(staticSector) {}


    /** ************************* OPERATORS ************************* */

    /** overloaded '=='-operator for sorting algorithms */
    bool operator==(const ActiveSector& b) const
    {
      return (getFullSecID() == b.getFullSecID());
    }


    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const ActiveSector& b) const
    {
      return (getFullSecID() < b.getFullSecID());
    }


    /** overloaded '>'-operator for sorting algorithms */
    bool operator>(const ActiveSector& b) const
    {
      return (getFullSecID() > b.getFullSecID());
    }


    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const ActiveSector& aSector) { out << aSector.getName(); return out; }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters:

    /** returns secID of this sector */
    std::string getName() const {return getFullSecID().getFullSecString(); }


    /** returns all indices of attached Hits */
    inline const std::vector<HitType*>& getHits() const { return m_hits; }


    /** returns all IDs for inner sectors of two-sector-combinations stored in the static SectorMap*/
    inline const std::vector<FullSecID>& getInner2spSecIDs() const { return m_staticSector->getInner2spSecIDs(); }


    /** returns all IDs for inner sectors of three-sector-combinations stored in the static SectorMap*/
    inline const std::vector<FullSecID>& getInner3spSecIDs() const { return m_staticSector->getInner3spSecIDs(); }


    /** returns all IDs for inner sectors of four-sector-combinations stored in the static SectorMap*/
    inline const std::vector<FullSecID>& getInner4spSecIDs() const { return m_staticSector->getInner4spSecIDs(); }


    /** returns pointer to associated static Sector in StoreArray */
    inline const StaticSectorType* getAttachedStaticSector() const { return m_staticSector; }


    /** returns VxdID of sensor carrying current sector */
    inline FullSecID getFullSecID() const { return m_staticSector->getFullSecID(); }


    /** for given pair of Hits, the static sector will provide the filters for testing and apply them, returns true if it was accepted */
    bool acceptTwoHitCombination(const FullSecID secID,
                                 const HitType& outerHit,
                                 const HitType& innerHit) const
    {
      return m_staticSector->accept(secID, outerHit.getHit(), innerHit.getHit());
    }

    /** for given triple of Hits, the static sector will provide the filters for testing and apply them, returns true if it was accepted */
    bool acceptThreeHitCombination(const FullSecID centerSecID,
                                   const FullSecID innerSecID,
                                   const HitType& outerHit,
                                   const HitType& centerHit,
                                   const HitType& innerHit) const
    {
      return m_staticSector->accept(centerSecID, innerSecID, outerHit.getHit(), centerHit.getHit(), innerHit.getHit());
    }

    /** for given triple of Hits, the static sector will provide the filters for testing and apply them, returns true if it was accepted */
    bool acceptFourHitCombination(const FullSecID outerCenterSecID,
                                  const FullSecID innerCenterSecID,
                                  const FullSecID innerSecID,
                                  const HitType& outerHit,
                                  const HitType& outerCenterHit,
                                  const HitType& innerCenterHit,
                                  const HitType& innerHit) const
    {
      return m_staticSector->accept(outerCenterSecID, innerCenterSecID, innerSecID, outerHit.getHit(), outerCenterHit.getHit(),
                                    innerCenterHit.getHit(), innerHit.getHit());
    }
/// setters:

    /** adds new Hit to vector of Hits */
    inline void addHit(HitType* newHit) { m_hits.push_back(newHit); }


    /** adds new several new hits to vector of hits  */
    inline void addHits(std::vector<HitType*>& newHits)
    {
      for (HitType* aHit : newHits) { addHit(aHit); }
    }
  };
} //Belle2 namespace
