/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHReconstructionPar.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHGlobalAlignment.h>
#include <arich/dbobjects/ARICHMirrorAlignment.h>
#include <arich/dbobjects/ARICHAeroTilesAlignment.h>
#include "framework/datastore/StoreArray.h"
#include "arich/dataobjects/ARICHHit.h"
#include "arich/dataobjects/ARICHTrack.h"
#include "arich/dataobjects/ARICHLikelihood.h"
#include <framework/database/DBObjPtr.h>


#include <TVector3.h>

namespace Belle2 {

  /**
   * Internal ARICH track reconstruction.
   *
   * The class contains objects and methods needed for internal ARICH track
   * reconstruction.
   * The main method is likelihood2, which returns the value
   * of the likelihood function and number of expected photons for every
   * input track and all hypotheses. For that purpose,
   * the Cherenkov photon is generated for every
   * track that hits the aerogel, by calling the CherenkovPhoton method.
   * The photon is propagated (FastTracking) to
   * the active area of the detection inside a HAPD
   * and the intersection point with the active area is determined.
   * Whether the photon was detected or not is determined by numerical
   * simulation of the module's geometric acceptance.
   */
  class ARICHReconstruction {

  public:

    /**
     * Constructor.
     */
    explicit ARICHReconstruction(int storePhotons = 0);

    /**
     * Destructor.
     */
    ~ARICHReconstruction() {};

    /**
     * Read geomerty parameters from xml and initialize class members.
     */
    void initialize();

    /**
     * Smears track parameters ("simulate" the uncertainties of tracking).
     */
    int smearTrack(ARICHTrack& arichTrack);

    /**
     * Transforms track parameters from global Belle2 to ARICH local frame.
     */
    void transformTrackToLocal(ARICHTrack& arichTrack, bool align);

    /**
     * Computes the value of identity likelihood function
     * for different particle hypotheses.
     */
    int likelihood2(ARICHTrack& arichTrack, const StoreArray<ARICHHit>& arichHits, ARICHLikelihood& arichLikelihood);

    /**
     * Sets track position resolution (from tracking).
     */
    void setTrackPositionResolution(double pRes);

    /**
     * Sets track direction resolution (from tracking).
     */
    void setTrackAngleResolution(double aRes);

    /**
     * Use mirror alignment or not.
     */
    void useMirrorAlignment(bool align)
    {
      m_alignMirrors = align;
    };

    /**
     * Correct mean emission point z position.
     */
    void correctEmissionPoint(int tileID, double r);

  private:

    static const int c_noOfHypotheses = Const::ChargedStable::c_SetSize; /**< Number of hypotheses to loop over */
    static const int c_noOfAerogels = 5; /**< Maximal number of aerogel layers to loop over */
    double p_mass[c_noOfHypotheses];  /**< particle masses */

    DBObjPtr<ARICHGeometryConfig> m_arichgp; /**< geometry configuration parameters from the DB */
    DBObjPtr<ARICHReconstructionPar> m_recPars; /**< reconstruction parameters from the DB */
    DBObjPtr<ARICHChannelMask> m_chnMask; /**< map of masked channels from the DB */
    DBObjPtr<ARICHChannelMapping> m_chnMap; /**< map x,y channels to asic channels from the DB */
    DBObjPtr<ARICHGlobalAlignment> m_alignp; /**< global alignment parameters from the DB */
    DBObjPtr<ARICHMirrorAlignment> m_mirrAlign; /**< global alignment parameters from the DB */
    OptionalDBObjPtr<ARICHAeroTilesAlignment> m_tileAlign; /**< alignment of aerogel tiles from DB */

    std::vector<TVector3> m_mirrorPoints; /**< vector of points on all mirror plates */
    std::vector<TVector3> m_mirrorNorms;  /**< vector of nomal vectors of all mirror plates */

    double m_trackPosRes; /**< track position resolution (from tracking) */
    double m_trackAngRes; /**< track direction resolution (from tracking) */
    bool   m_alignMirrors; /**< if set to true mirror alignment constants from DB are used*/

    unsigned int m_nAerogelLayers; /**< number of aerogel layers */
    double  m_refractiveInd[c_noOfAerogels]; /**< refractive indices of aerogel layers */
    double  m_zaero[c_noOfAerogels]; /**< z-positions of aerogel layers */
    double  m_thickness[c_noOfAerogels]; /**< thicknesses of areogel layers */
    double  m_transmissionLen[c_noOfAerogels]; /**< transmission lengths of aerogel layers */
    double  m_n0[c_noOfAerogels];  /**< number of emmited photons per unit length */
    TVector3 m_anorm[c_noOfAerogels]; /**< normal vector of the aerogle plane */
    int m_storePhot; /**< set to 1 to store individual reconstructed photon information */
    double m_tilePars[124][2] = {{0}};

    /**
     * Returns 1 if vector "a" lies on "copyno"-th detector active surface
     * of detector and 0 else.
     */
    int InsideDetector(TVector3 a, int copyno);

    /**
     * Returns the hit virtual position, assuming that it was reflected
     * from mirror.
     * @param[in] hitpos   Vector of hit positions.
     * @param[in] mirrorID Id of mirror from which the photon was reflected.
     */
    TVector3 HitVirtualPosition(const TVector3& hitpos, int mirrorID);

    /**
     * Calculates the intersection of the Cherenkov photon emitted
     * from point "r" in "dirf" direction with the detector plane.
     * @param[in] r      Vector of photon emission point.
     * @param[in] dirf   Direction of photon emission.
     * @param[in] refind Array of layers refractive indices.
     * @param[in] z      Array of z coordinates of borders between layers.
     * @param[in] n      Number of aerogel layers through which photon passes.
     * @param[in] opt    Parameter can be set to 1 to return empty TVector3 in case of errors
     */
    TVector3 FastTracking(TVector3 dirf, TVector3 r,  double* refind, double* z, int n, int opt);

    /**
     * Calculates the intersection of the Cherenkov photon emitted
     * from point "r" in "dirf" direction with the detector plane.
     * (For the case of simple (beamtest) geometry.)
     * @param[in] r      Vector of photon emission point.
     * @param[in] dirf   Direction of photon emission.
     * @param[in] refind Array of layers refractive indices.
     * @param[in] z      Array of z coordinates of borders between layers.
     * @param[in] n      Number of aerogel layers through which photon passes.
     */
    TVector3 FastTrackingSimple(TVector3 dirf, TVector3 r,  double* refind, double* z, int n);

    /**
     * Calculates the direction of photon emission.
     * Given the Cherenkov photon emission point "r" and its position
     * on detector plane "rh" (hit position) this methods calculates
     * the direction "dirf" in which photon was emitted, under the assumption
     * that it was reflected from "mirrorID"-th mirror plate
     * (mirrorID=-1 for no reflection).
     *
     * @param[in] r
     * Vector of photon emission point.
     *
     * @param[in] rh
     * Photon hit position.
     *
     * @param[in] dirf
     * Vector of photon emission direction (this is output of method).
     *
     * @param[in] rf
     * Vector of photon position on aerogel exit.
     *
     * @param[in] refind
     * Array of layers refractive indices.
     *
     * @param[in] z
     * Array of z coordinates of borders between layers.
     *
     * @param[in] n
     * Number of aerogel layers through which photon passes.
     *
     * @param[in] mirrorID
     * Id of mirror from which the photon was reflected (assumption).
     */
    int  CherenkovPhoton(TVector3 r, TVector3 rh,
                         TVector3& rf, TVector3& dirf,
                         double* refind, double* z, int n, int mirrorID = 0);

    /**
     * Returns true if photon at position pos with direction dir hits mirror
     * plate with ID mirrorID.
     * @param[in] pos      Photon position.
     * @param[in] dir      Photon direction.
     * @param[in] mirrorID ID of mirror plate.
     */
    bool HitsMirror(const TVector3& pos, const TVector3& dir, int mirrorID);

    /**
     * Returns mean emission position of Cherenkov photons
     * from i-th aerogel layer.
     */
    TVector3 getTrackMeanEmissionPosition(const ARICHTrack& track, int iAero);

    /**
     * Returns track direction at point with z coordinate "zout"
     * (assumes straight track).
     */
    TVector3 getTrackPositionAtZ(const ARICHTrack& track, double zout);

    /**
     * Returns point on the mirror plate with id mirrorID.
     */
    TVector3 getMirrorPoint(int mirrorID);

    /**
     * Returns normal vector of the mirror plate with id mirrorID.
     */
    TVector3 getMirrorNorm(int mirrorID);

  };

} // end of namespace Belle2
