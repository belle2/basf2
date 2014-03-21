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

#include <arich/geometry/ARICHGeometryPar.h>
#include <arich/modules/arichReconstruction/ARICHTrack.h>

#include <TVector3.h>
#include <TNtuple.h>
#include <cmath>
#include <boost/format.hpp>

namespace Belle2 {
  /** Internal ARICH track reconstruction
    *
    * The class contains objects and methods needed for internal ARICH track
    * reconstruction.
    * The main method is likelihood2, which returns
    * the value of the likelihood function and number of expected photons for every
    * input track and all hypotheses. For that purpose,
    * the Cherenkov photon is generated for every
    * track that hits the aerogel, by calling the CherenkovPhoton method.
    * The photon is propagated (FastTracking) to
    * the active area of the detection inside a HAPD
    * and the intersection point with the active area is determined.
    * Whether the photon was detected or not is determined by numerical simulation
    * of the module's geometric acceptance.
    *
    */

  class ARICHReconstruction {

  public:

    //! Constructor
    ARICHReconstruction(int, int);

    //! Destructor
    ~ARICHReconstruction() {};

    //! Smeares track parameters ("simulate" the uncertainties of tracking).
    int smearTracks(std::vector<ARICHTrack>&);

    //! Computes the value of identity likelihood function for different particle hypotheses.
    int likelihood2(std::vector<ARICHTrack>&);
    //! Sets detector background level (photon hits / m^2)
    void setBackgroundLevel(double nbkg);
    //! Sets track position resolution (from tracking)
    void setTrackPositionResolution(double pRes);
    //! Sets track direction resolution (from tracking)
    void setTrackAngleResolution(double aRes);
    //! Sets single photon emission angle resolution (without detctor pad)
    void setSinglePhotonResolution(double sRes);
    //! Sets aerogel layer figure of merit
    void setAerogelFigureOfMerit(std::vector<double>& merit);

  private:

    static const int c_noOfHypotheses = 5; /**< Number of hypotheses to loop over */
    static const int c_noOfAerogels = 5; /**< Maximal number of aerogel layers to loop over */

    ARICHGeometryPar* m_arichGeoParameters; /**< holding the parameters of detector */
    TNtuple* m_hitstuple;  /**< beamtest/debug output hit-track pairs */
    TNtuple* m_tracktuple; /**< beamtest/debug output tracks */
    int m_storeHist; /**< store individual photon information (cherenkov angle distribution) */
    int m_beamtest; /**< in the case of beamtest analysis >=1, default 0 */
    double m_bkgLevel; /**< detector photon background level */
    double m_trackPosRes; /**< track position resolution (from tracking) */
    double m_trackAngRes; /**< track direction resolution (from tracking) */
    double m_singleRes;   /**< single photon emission angle resolution */
    std::vector<double> m_aeroMerit; /**< aerogel layer figure of merit */
    //! Returns 1 if vector "a" lies on "copyno"-th detector active surface of detector and 0 else.
    int InsideDetector(TVector3 a, int copyno);
    //! Returns the hit virtual position, assuming that it was reflected from mirror.
    /*!
      \param hitpos vector of hit position
      \param mirrorID id of mirror from which the photon was reflected
     */
    TVector3 HitVirtualPosition(const TVector3 hitpos, int mirrorID);

    //! Calculates the intersection of the Cherenkov photon emitted from point "r" in "dirf" direction with the detector plane.
    /*!
      \param r vector of photon emission point
      \param dirf direction of photon emission
      \param n number of aerogel layers through which photon passes
      \param refind array of layers refractive indices
      \param z array of z coordinates of borders between layers
     */
    TVector3 FastTracking(TVector3 dirf, TVector3 r,  double* refind, double* z, int n);

//! Calculates the intersection of the Cherenkov photon emitted from point "r" in "dirf" direction with the detector plane. (For the case of simple (beamtest) geometry.)
    /*!
      \param r vector of photon emission point
      \param dirf direction of photon emission
      \param n number of aerogel layers through which photon passes
      \param refind array of layers refractive indices
      \param z array of z coordinates of borders between layers
     */
    TVector3 FastTrackingSimple(TVector3 dirf, TVector3 r,  double* refind, double* z, int n);

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
                         TVector3& rf, TVector3& dirf,
                         double* refind, double* z, int n);
  };

} // end of namespace Belle2

#endif // ARICHRECONSTRUCTION_H

