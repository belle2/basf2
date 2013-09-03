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
#include <utility>
#include "ThreeHitFilters.h"
#include "VXDSector.h"



namespace Belle2 {
  namespace Tracking {
    /** bundles filter methods using 3 hits (using ThreeHitFilter-class). and compares with entries of sector min and max */
    class NbFinderFilters : public ThreeHitFilters {
    public:
      typedef std::pair <int, int> SuccessAndFailCounter; /**< first entry is for number of times when filter approved valuse, second one is for number of times when filter neglected values */

      /** Empty constructor. For initialisation only, an object generated this way is useless unless resetValues is called at least once */
      NbFinderFilters():
        ThreeHitFilters(),
        m_thisSector(NULL),
        m_friendID(0),
        m_angle3DCtr(std::make_pair(0, 0)),
        m_angleXYCtr(std::make_pair(0, 0)),
        m_angleRZCtr(std::make_pair(0, 0)),
        m_circleDist2IPCtr(std::make_pair(0, 0)),
        m_deltaSlopeRZCtr(std::make_pair(0, 0)),
        m_pTCtr(std::make_pair(0, 0)),
        m_helixFitCtr(std::make_pair(0, 0)) {}

      /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available, optional parameter sets strength of magnetic field (standard is 1.5T) */
      NbFinderFilters(TVector3 outerHit, TVector3 centerHit, TVector3 innerHit, VXDSector* thisSector, unsigned int friendID, double magneticFieldStrength = 1.5):
        ThreeHitFilters(outerHit, centerHit, innerHit, magneticFieldStrength),  // calls constructor of base class. Needed since base class does not use standard constructor, therefore we have to carry the hits manually into the base class
        m_thisSector(thisSector),
        m_friendID(friendID),
        m_angle3DCtr(std::make_pair(0, 0)),
        m_angleXYCtr(std::make_pair(0, 0)),
        m_angleRZCtr(std::make_pair(0, 0)),
        m_circleDist2IPCtr(std::make_pair(0, 0)),
        m_deltaSlopeRZCtr(std::make_pair(0, 0)),
        m_pTCtr(std::make_pair(0, 0)),
        m_helixFitCtr(std::make_pair(0, 0)) {}


      /** Destructor. */
      ~NbFinderFilters() {}

      /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
      void resetValues(TVector3 outerHit, TVector3 centerHit, TVector3 innerHit, VXDSector* thisSector, unsigned int friendID) {
        ThreeHitFilters::resetValues(outerHit, centerHit, innerHit); // resetValues of baseClass
        m_thisSector = thisSector;
        m_friendID = friendID;
      }

      /** Overrides Constructor-Setup for magnetic field. if no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T */
      void resetMagneticField(double magneticFieldStrength = 1.5) { ThreeHitFilters::resetMagneticField(magneticFieldStrength); }


      /** simply checks whether angle3D-value is accepted by the given cutoffs */
      bool checkAngle3D(int nameAngle3D);


      /** calculates the angle between the hits/vectors (3D), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngle3D instead) */
      double calcAngle3D() { return ThreeHitFilters::calcAngle3D(); }


      /** calculates the angle between the hits/vectors (3D), returning unit: angle in radians */
      double fullAngle3D() { return ThreeHitFilters::fullAngle3D(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateAngle3D() { return m_angle3DCtr; }


      /** simply checks whether angleXY-value is accepted by the given cutoffs */
      bool checkAngleXY(int nameAngleXY);


      /** calculates the angle between the hits/vectors (XY), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleXY instead) */
      double calcAngleXY() { return ThreeHitFilters::calcAngleXY(); }


      /** calculates the angle between the hits/vectors (XY), returning unit: angle in radians */
      double fullAngleXY() { return ThreeHitFilters::fullAngleXY(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateAngleXY() { return m_angleXYCtr; }


      /** simply checks whether angleRZ-value is accepted by the given cutoffs */
      bool checkAngleRZ(int nameAngleRZ);


      /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use fullAngleRZ instead) */
      double calcAngleRZ() { return ThreeHitFilters::calcAngleRZ(); }


      /** calculates the angle between the hits/vectors (RZ), returning unit: angle in radians */
      double fullAngleRZ() { return ThreeHitFilters::fullAngleRZ(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateAngleRZ() { return m_angleRZCtr; }


      /** simply checks whether angleRZ-value is accepted by the given cutoffs */
      bool checkCircleDist2IP(int nameCircleDist2IP);


      /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
      double calcCircleDist2IP() { return ThreeHitFilters::calcCircleDist2IP(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateCircleDist2IP() { return m_circleDist2IPCtr; }


      /** simply checks whether the difference of the slope in RZ-value is accepted by the given cutoffs */
      bool checkDeltaSlopeRZ(int nameDeltaSlopeRZ);


      /** calculates deviations in the slope of the inner segment and the outer segment, returning unit: none */
      double calcDeltaSlopeRZ() { return ThreeHitFilters::calcDeltaSlopeRZ(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateDeltaSlopeRZ() { return m_deltaSlopeRZCtr; }


      /** simply checks whether pT-value is accepted by the given cutoffs */
      bool checkPt(int namePt);


      /** calculates the estimation of the transverse momentum of the 3-hit-tracklet, returning unit: GeV/c */
      double calcPt() { return ThreeHitFilters::calcPt(); }


      /** calculates the estimation of the transverse momentum of a given particle track circle radius, returning unit: GeV/c */
      double calcPt(double& pT) { return ThreeHitFilters::calcPt(pT); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRatePt() { return m_pTCtr; }


      /** simply checks whether helixparameter-value is accepted by the given cutoffs */
      bool checkHelixFit(int nameHelixFit);


      /** calculates the helixparameter describing the deviation in z per unit angle, returning unit: none */
      double calcHelixFit() { return ThreeHitFilters::calcHelixFit(); }


      /** returns number of accepted (.first) and neglected (.second) filter tests using dist3D */
      SuccessAndFailCounter getAcceptanceRateHelixFit() { return m_helixFitCtr; }


      /** returns cutoff-values of given filter */
      std::pair <double, double> getCutoffs(int aFilter) {
        const Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
        if (pCutoff != NULL) return std::make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
        return std::make_pair(0., 0.);
      }// one method to read them all...


    protected:

      VXDSector* m_thisSector; /**< contains cutoffs for all filters available in this sector, together with the friendID the return values are unique */
      unsigned int m_friendID; /**< is a key used for determine the currently needed filterSet */
      SuccessAndFailCounter m_angle3DCtr; /**< counts number of successful (.first) and neglected (.second) tests for angle3D */
      SuccessAndFailCounter m_angleXYCtr; /**< counts number of successful (.first) and neglected (.second) tests for angleXY */
      SuccessAndFailCounter m_angleRZCtr; /**< counts number of successful (.first) and neglected (.second) tests for angleRZ */
      SuccessAndFailCounter m_circleDist2IPCtr; /**< counts number of successful (.first) and neglected (.second) tests for circleDist2IP */
      SuccessAndFailCounter m_deltaSlopeRZCtr; /**< counts number of successful (.first) and neglected (.second) tests for deltaSlopeRZ */
      SuccessAndFailCounter m_pTCtr; /**< counts number of successful (.first) and neglected (.second) tests for pT */
      SuccessAndFailCounter m_helixFitCtr; /**< counts number of successful (.first) and neglected (.second) tests for helixFit */
    }; //end class NbFinderFilters
  } // end namespace Tracking
} //end namespace Belle2

#endif //NBFINDERFILTERS


