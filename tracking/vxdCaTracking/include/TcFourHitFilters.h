/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef TCFOURHITFILTERS_H
#define TCFOURHITFILTERS_H

#include "TObject.h"
#include <TVector3.h>
#include "FourHitFilters.h"
#include "VXDSector.h"
#include <utility>



namespace Belle2 {

  /** bundles filter methods using 4 hits (using FourHitFilters-class). and compares with entries of sector min and max */
  class TcFourHitFilters : public FourHitFilters {
  public:
    typedef std::pair <int, int> SuccessAndFailCounter; /**<  first entry is for number of times when filter approved valuse, second one is for number of times when filter neglected values */

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues is called at least once */
    TcFourHitFilters():
      FourHitFilters(),
      m_thisSector(NULL),
      m_friendID(0),
      m_deltaPtCtr(std::make_pair(0, 0)),
      m_deltaDistCircleCenterCtr(std::make_pair(0, 0)) {}

    /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available */
    TcFourHitFilters(TVector3& outer, TVector3& outerCenter, TVector3& innerCenter, TVector3& inner, VXDSector* thisSector, unsigned int friendID):
      FourHitFilters(outer, outerCenter, innerCenter, inner),  // calls constructor of base class. Needed since base class does not use standard constructor, therefore we have to carry the hits manually into the base class
      m_thisSector(thisSector),
      m_friendID(friendID),
      m_deltaPtCtr(std::make_pair(0, 0)),
      m_deltaDistCircleCenterCtr(std::make_pair(0, 0)) {}


    /** Destructor. */
    ~TcFourHitFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(TVector3& outer, TVector3& outerCenter, TVector3& innerCenter, TVector3& inner, VXDSector* thisSector, unsigned int friendID) {
      FourHitFilters::resetValues(outer, outerCenter, innerCenter, inner); // resetValues of baseClass
      m_thisSector = thisSector;
      m_friendID = friendID;
    }

    /** simply check whether dpt-value is accepted by the given cutoffs (dpt= difference in transverse momentum of 2 subsets of the hits) */
    bool checkDeltapT(int nameDeltapT);
    /** calc and return dpt-value directly (dpt= difference in transverse momentum of 2 subsets of the hits) */
    double deltapT() { return FourHitFilters::deltapT(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using deltapT */
    SuccessAndFailCounter getAcceptanceRateDeltapT() { return m_deltaPtCtr; }

    /** simply check whether ddist2IP-value is by the given cutoffs (ddist2IP= difference in magnitude of the points of closest approach of two circles calculated using 2 subsets of the hits) */
    bool checkDeltaDistCircleCenter(int nameDeltaDistCircleCenter);
    /** calc and return ddist2IP-value directly (ddist2IP= difference in magnitude of the points of closest approach of two circles calculated using 2 subsets of the hits) */
    double deltaDistCircleCenter() { return FourHitFilters::deltaDistCircleCenter(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using deltaDistCircleCenter */
    SuccessAndFailCounter getAcceptanceRateDeltaDistCircleCenter() { return m_deltaDistCircleCenterCtr; }

    /** returns cutoff-values of given filter */
    std::pair <double, double> getCutoffs(int aFilter);

  protected:

    VXDSector* m_thisSector; /**< contains cutoffs for all filters available in this sector, together with the friendID the return values are unique */
    unsigned int m_friendID; /**< is a key used for determine the currently needed filterSet */
    SuccessAndFailCounter m_deltaPtCtr; /**< counts number of successful (.first) and neglected (.second) tests for deltapT */
    SuccessAndFailCounter m_deltaDistCircleCenterCtr; /**< counts number of successful (.first) and neglected (.second) tests for deltaDistCircleCenter */
  }; //end class TcFourHitFilters
} //end namespace Belle2

#endif //TCFOURHITFILTERS


