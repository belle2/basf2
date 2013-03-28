/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#ifndef NBFINDERFILTERS_H
#define NBFINDERFILTERS_H

#include "TObject.h"
#include <TVector3.h>
#include <string>
#include <utility>
#include "ThreeHitFilters.h"
#include "VXDSector.h"



namespace Belle2 {

  /** bundles filter methods using 3 hits (using ThreeHitFilter-class). and compares with entries of sector min and max */
  class NbFinderFilters : public ThreeHitFilters {
  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues is called at least once */
    NbFinderFilters():
      ThreeHitFilters() {
      m_friendID = "";
      m_thisSector = NULL;
    }

    /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available */
    NbFinderFilters(TVector3 outerHit, TVector3 centerHit, TVector3 innerHit, VXDSector* thisSector, std::string friendID):
      ThreeHitFilters(outerHit, centerHit, innerHit),  // calls constructor of base class. Needed since base class does not use standard constructor, therefore we have to carry the hits manually into the base class
      m_thisSector(thisSector),
      m_friendID(friendID) {}


    /** Destructor. */
    ~NbFinderFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(TVector3 outerHit, TVector3 centerHit, TVector3 innerHit, VXDSector* thisSector, std::string friendID) {
      ThreeHitFilters::resetValues(outerHit, centerHit, innerHit); // resetValues of baseClass
      m_thisSector = thisSector;
      m_friendID = friendID;
    }

    /** simply checks whether angle3D-value is accepted by the given cutoffs */
    bool checkAngle3D(std::string nameAngle3D);
    /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngle3D instead) */
    double calcAngle3D() { return ThreeHitFilters::calcAngle3D(); }
    /** calculates the angle between the hits/vectors (3D), returning unit: angle in radians */
    double fullAngle3D() { return ThreeHitFilters::fullAngle3D(); }

    /** simply checks whether angleXY-value is accepted by the given cutoffs */
    bool checkAngleXY(std::string nameAngleXY);
    /** calculates the angle between the hits/vectors (XY), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleXY instead) */
    double calcAngleXY() { return ThreeHitFilters::calcAngleXY(); }
    /** calculates the angle between the hits/vectors (XY), returning unit: angle in radians */
    double fullAngleXY() { return ThreeHitFilters::fullAngleXY(); }

    /** simply checks whether angleRZ-value is accepted by the given cutoffs */
    bool checkAngleRZ(std::string nameAngleRZ);
    /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleRZ instead) */
    double calcAngleRZ() { return ThreeHitFilters::calcAngleRZ(); }
    /** calculates the angle between the hits/vectors (RZ), returning unit: angle in radians */
    double fullAngleRZ() { return ThreeHitFilters::fullAngleRZ(); }

    /** simply checks whether angleRZ-value is accepted by the given cutoffs */
    bool checkCircleDist2IP(std::string nameCircleDist2IP);
    /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
    double calcCircleDist2IP() { return ThreeHitFilters::calcCircleDist2IP(); }

    /** simply checks whether the difference of the slope in RZ-value is accepted by the given cutoffs */
    bool checkDeltaSlopeRZ(std::string nameDeltaSlopeRZ);
    /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none */
    double calcDeltaSlopeRZ() { return ThreeHitFilters::calcDeltaSlopeRZ(); }

    /** simply checks whether pT-value is accepted by the given cutoffs */
    bool checkPt(std::string namePt);
    /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
    double calcPt() { return ThreeHitFilters::calcPt(); }

    /** simply checks whether helixparameter-value is accepted by the given cutoffs */
    bool checkHelixFit(std::string nameHelixFit);
    /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
    double calcHelixFit() { return ThreeHitFilters::calcHelixFit(); }

    /** returns cutoff-values of given filter */
    std::pair <double, double> getCutoffs(std::string aFilter); // one method to read them all...


  protected:

    VXDSector* m_thisSector; /** contains cutoffs for all filters available in this sector, together with the friendID the return values are unique */
    std::string m_friendID; /** is a key used for determine the currently needed filterSet */
  }; //end class NbFinderFilters
} //end namespace Belle2

#endif //NBFINDERFILTERS


