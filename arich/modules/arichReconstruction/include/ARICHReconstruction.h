/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRECONSTRUCTION_H
#define ARICHRECONSTRUCTION_H

#include <arich/geoarich/ARICHGeometryPar.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <cmath>
#include <boost/format.hpp>

using namespace std;
using namespace boost;

#define MAX_N_ALAYERS 5

namespace Belle2 {

  //! This Class contains methods needed to obtain the value of identity likelihood function for different particle hypotheses.

  class ARICHReconstruction {

  public:

    //! Constructor
    ARICHReconstruction();

    //! Destructor
    ~ARICHReconstruction() {};

    //! Smeares track parameters ("simulate" the uncertainties of tracking).
    int ReconstructParticles();

    //! Computes the value of identity likelihood function for different particle hypotheses.
    int Likelihood2();

  private:

    TRandom3* m_random; /**< random generator */
    ARICHGeometryPar* _arichgp; /**< holding the parameters of detector */

    //! Returns 1 if vector "a" lies on detector active surface of detector and 0 else.
    int InsideDetector(TVector3 a);

    //! Returns the hit virtual position, assuming that it was reflected from mirror.
    /*!
      \param hitpos vector of hit position
      \param mirrorID id of mirror from which the foton was reflected
     */
    TVector3 HitVirtualPosition(const TVector3 hitpos, int mirrorID);

    //! Calculates the intersection of the Cherenkov photon emitted from point "r" in "dirf" direction with the detector plane.
    /*!
      \param r vector of photon emittion point
      \param dirf direction of photon emission
      \param n number of aerogel layers through which photon passes
      \param refind array of layers refractive indices
      \param z array of z coordinates of borders between layers
     */
    TVector3 FastTracking(TVector3 dirf, TVector3 r,  double *refind, double *z, int n);

    //! Calculates the direction of photon emission.
    /*! Giving the Cherenkov photon emission point "r" and its position on detector plane "rh" (hit position) this methods calculates the direction "dirf" in which photon was emitted, under the assumption that it was reflected from "nmir"-th mirror plate (nmir=-1 for no reflection).

      \param r vector of photon emission point
      \param rh photon hit position
      \param dirf vector of photon emission direction (this is output of method)
      \param rf vector of photon position on aerogel exit
      \param refind array of layers refractive indices
      \param n number of aerogel layers through which photon passes
      \param z array of z coordinates of borders between layers
      \param nmir id of mirror from which the foton was reflected (assuming).

    */
    int  CherenkovPhoton(TVector3 r, TVector3 rh,
                         TVector3 &rf, TVector3 &dirf,
                         double *refind, double *z, int n, int nmir);
  };


} // end of namespace Belle2

#endif // ARICHRECONSTRUCTION_H

