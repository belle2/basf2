/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FCCluster_H
#define FCCluster_H

#include "skim/hlt/modules/level3/FTList.h"
#include <TVector3.h>

namespace Belle2 {

  // ECL cluster class for the Level-3 Fast Cluster Finder
  class FCCluster {
  public:
    //! constructor
    FCCluster(const FTList<FCCrystal*>* hits, const double energy,
              const FCCrystal* seed, const TVector3& Position);

    //! destructor
    ~FCCluster();

  public: //Selectors

    //! returns position of most energetic counter
    const TVector3& getPosition(void) const;

    //! returns most energetic counter
    const FCCrystal& getSeed(void) const;

    //! returns cluster energy
    double getEnergy(void) const;

    //! returns list of ehits
    const FTList<FCCrystal*>& getEHits(void) const;

  private: //private data members
    const FTList<FCCrystal*>& m_hits; // array of hits
    const double m_energy; // energy deposit
    const TVector3 m_position; // position of the seed
    const FCCrystal& m_seed; // reference of the seed crystal
  };

  //----------------------------------------------
#ifdef FCCluster_NO_INLINE
#define inline
#else
#undef inline
#define FCCluster_INLINE_DEFINE_HERE
#endif

#ifdef FCCluster_INLINE_DEFINE_HERE

  inline
  FCCluster::FCCluster(const FTList<FCCrystal*>* hits, const double energy,
                       const FCCrystal* seed, const TVector3& position)
    : m_hits(*hits),
      m_energy(energy),
      m_position(position),
      m_seed(*seed)
  {
  }

  inline
  FCCluster::~FCCluster()
  {
    delete &m_hits;
  }

  inline
  const TVector3&
  FCCluster::getPosition(void) const
  {
    return m_position;
  }

  inline
  const FCCrystal&
  FCCluster::getSeed(void) const
  {
    return m_seed;
  }

  inline
  double
  FCCluster::getEnergy(void) const
  {
    return m_energy;
  }

  inline
  const FTList<FCCrystal*>&
  FCCluster::getEHits(void) const
  {
    return m_hits;
  }

#endif

#undef inline

}

#endif /* FCCluster_FLAG_ */

