/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FCFinder_H
#define FCFinder_H

#include <string>

#include "skim/hlt/modules/level3/FTList.h"
#include "skim/hlt/modules/level3/FCCrystal.h"

namespace Belle2 {

  class FCCluster;

  //! The Level-3 Fast Cluster Finder
  class FCFinder {

  public:
    //! returns FCFinder
    static FCFinder& instance(void);

  public:
    //! Constructors and destructor
    FCFinder();

    //! initializer(creates geometry)
    void init();

    //! terminator
    void term();

    //! begin run function(reads constants)
    void beginRun();

    //! energy reconstructor core
    void event(const double seedThreshold, const double clusterECut);

  public:
    //! returns list of eHits
    FTList<FCCrystal*>& getEHits(void) const;

    //! returns reconstructed clusters
    FTList<FCCluster*>& getClusters(void) const;

    //! returns cell Id for given FTCrystal object
    int getCellId(FCCrystal*) const;

  private: // private member functions
    //! clear object
    void clear(void);

    //! create wire hit from ECLDigit
    void updateEcl3(void);

    //! cluster finding
    void clustering(const double seedThreshold, const double clusterECut);

  private:
    //! pointer for FCFinder
    static FCFinder* s_cFinder;

    //! pointer for the array of crystals
    FCCrystal* m_crystal;

    //! list of hits for the event
    FTList<FCCrystal*>& m_ehits;

    //! list of clusters for the event
    FTList<FCCluster*>& m_clusters;
  };

  inline
  FTList<FCCrystal*>&
  FCFinder::getEHits(void) const
  {
    return m_ehits;
  }

  inline
  FTList<FCCluster*>&
  FCFinder::getClusters(void) const
  {
    return m_clusters;
  }

  inline
  int
  FCFinder::getCellId(FCCrystal* c) const
  {
    return (int)(c - m_crystal);
  }

}

#endif /* FCFinder_FLAG_ */

