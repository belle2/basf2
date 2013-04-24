/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SEGFINDERFILTERS_H
#define SEGFINDERFILTERS_H

#include "TObject.h"
#include <TVector3.h>
#include <string>
#include <utility>
#include "TwoHitFilters.h"
#include "VXDSector.h"



namespace Belle2 {

  /** bundles filter methods using 2 hits (using TwhoHitFilter-class). and compares with entries of sector min and max */
  class SegFinderFilters : public TwoHitFilters {
  public:
    typedef std::pair <int, int> SuccessAndFailCounter; /**<  first entry is for number of times when filter approved valuse, second one is for number of times when filter neglected values */

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues is called at least once */
    SegFinderFilters():
      TwoHitFilters(),
      m_thisSector(NULL),
      m_friendID(""),
      m_dist3DCtr(std::make_pair(0, 0)),
      m_distXYCtr(std::make_pair(0, 0)),
      m_distZCtr(std::make_pair(0, 0)),
      m_normDist3DCtr(std::make_pair(0, 0)),
      m_slopeRZCtr(std::make_pair(0, 0)) {}

    /** Constructor. use this one, when having a sectormap (e.g. during track finding), use TwoHitFilters when no sectormap is available */
    SegFinderFilters(TVector3 outerHit, TVector3 innerHit, VXDSector* thisSector, std::string friendID):
      TwoHitFilters(outerHit, innerHit), // calls constructor of base class. Needed since base class does not use standard constructor, therefore we have to carry the hits manually into the base class
      m_thisSector(thisSector),
      m_friendID(friendID),
      m_dist3DCtr(std::make_pair(0, 0)),
      m_distXYCtr(std::make_pair(0, 0)),
      m_distZCtr(std::make_pair(0, 0)),
      m_normDist3DCtr(std::make_pair(0, 0)),
      m_slopeRZCtr(std::make_pair(0, 0)) {}


    /** Destructor. */
    ~SegFinderFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(TVector3 outerHit, TVector3 innerHit, VXDSector* thisSector, std::string friendID) {
      TwoHitFilters::resetValues(outerHit, innerHit);
      m_thisSector = thisSector;
      m_friendID = friendID;
    }

    /** simply checks whether dist3D-value is accepted by the given cutoffs */
    bool checkDist3D(std::string nameDist3D);
    /** calculates the distance between the hits (3D), returning unit: cm^2 for speed optimization */
    double calcDist3D() { return TwoHitFilters::calcDist3D(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
    SuccessAndFailCounter getAcceptanceRateDist3D() { return m_dist3DCtr; }

    /** simply checks whether distXY-value is accepted by the given cutoffs */
    bool checkDistXY(std::string nameDistXY);
    /** calculates the distance between the hits (XY), returning unit: cm^2 for speed optimization */
    double calcDistXY() { return TwoHitFilters::calcDistXY(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
    SuccessAndFailCounter getAcceptanceRateDistXY() { return m_distXYCtr; }

    /** simply checks whether distZ-value is accepted by the given cutoffs */
    bool checkDistZ(std::string nameDistZ);
    /** calculates the distance between the hits (Z only), returning unit: cm */
    double calcDistZ() { return TwoHitFilters::calcDistZ(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
    SuccessAndFailCounter getAcceptanceRateDistZ() { return m_distZCtr; }

    /** simply checks whether normedDist3D-value is accepted by the given cutoffs */
    bool checkNormedDist3D(std::string nameNormDist3D);
    /** calculates the normed distance between the hits (3D), return unit: none */
    double calcNormedDist3D() { return TwoHitFilters::calcNormedDist3D(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
    SuccessAndFailCounter getAcceptanceRateNormDist3D() { return m_normDist3DCtr; }

    /** imply checks whether the slope in RZ is accepted by the given cutoffs */
    bool checkSlopeRZ(std::string nameSlopeRZ);
    /** calculates the slope of the hits in RZ, return unit: cm (cm^2/cm = cm) */
    double calcSlopeRZ() { return TwoHitFilters::calcSlopeRZ(); }
    /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
    SuccessAndFailCounter getAcceptanceRateSlopeRZ() { return m_slopeRZCtr; }

    /** returns cutoff-values of given filter */
    std::pair <double, double> getCutoffs(std::string aFilter); // one method to read them all...


  protected:

    VXDSector* m_thisSector; /**< contains cutoffs for all filters available in this sector, together with the friendID the return values are unique */
    std::string m_friendID; /**< is a key used for determine the currently needed filterSet */
    SuccessAndFailCounter m_dist3DCtr; /**< counts number of successful (.first) and neglected (.second) tests for distance3D */
    SuccessAndFailCounter m_distXYCtr; /**< counts number of successful (.first) and neglected (.second) tests for distanceXY */
    SuccessAndFailCounter m_distZCtr; /**< counts number of successful (.first) and neglected (.second) tests for distanceZ */
    SuccessAndFailCounter m_normDist3DCtr; /**< counts number of successful (.first) and neglected (.second) tests for normedDistance3D */
    SuccessAndFailCounter m_slopeRZCtr; /**< counts number of successful (.first) and neglected (.second) tests for slopeRZ */
  }; //end class SegFinderFilters
} //end namespace Belle2

#endif //SEGFINDERFILTERS


