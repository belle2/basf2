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

  //! ECL cluster class for the Level-3 Fast Cluster Finder
  class FCCluster {
  public:
    //! constructor
    FCCluster(const FTList<FCCrystal*>* hits, const double energy,
              const FCCrystal* seed, const TVector3& position);

    //! destructor
    ~FCCluster();

  public:
    //! returns position of most energetic counter
    const TVector3& getPosition(void) const;

    //! returns most energetic counter
    const FCCrystal& getSeed(void) const;

    //! returns cluster energy
    double getEnergy(void) const;

    //! returns list of ehits
    const FTList<FCCrystal*>& getEHits(void) const;

  private:
    //! array of hits
    const FTList<FCCrystal*>& m_hits;

    //! energy deposit
    const double m_energy;

    //! position of the seed
    const TVector3 m_position;

    //! reference of the seed crystal
    const FCCrystal& m_seed;
  };

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

}

#endif /* FCCluster_FLAG_ */

