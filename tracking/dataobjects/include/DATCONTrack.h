/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <math.h>

#include <root/TObject.h>
#include <root/TVector3.h>
#include <root/TMath.h>


namespace Belle2 {
  /**
  * The DATCON Track class.
  *
  * Contains helix and straight track parameters for the DATCON
  * tracking algorithm.
  */

  class DATCONTrack : public TObject {

  public:
    /**
    * Standard constructor
    * @param trackID unique ID of track
    * @param r radius of track estimated by hough trafo algorithm
    * @param phi phi of track estimated by hough trafo algorithm
    * @param d vertical coordinate of the theta Hough Space to compute z0
    * @param theta theta of track estimated by hough trafo algorithm
    * @param charge particle charge (-1, 0, +1) of hough track estimated by hough trafo algorithm
    * @param curvsign usual way to define track curvature
    * @param left redundant information with curvsign, initialized as false
    */
    DATCONTrack(): trackID(0), trackRadius(), trackPhi(), trackZzero(), trackTheta(), trackCharge(), trackCurvsign() {}

    /**
    * Constructor if all values are given
    */
    DATCONTrack(unsigned int ID, double r, double phi, double zzero, double theta, int charge, int curvsign):
      trackID(ID), trackRadius(r), trackPhi(phi), trackZzero(zzero), trackTheta(theta), trackCharge(charge), trackCurvsign(curvsign) {}


    DATCONTrack(std::vector<unsigned int> hitIDList, unsigned int ID, double r, double phi, double zzero, double theta,
                int charge, int curvsign):
      m_HitIDList(hitIDList), trackID(ID), trackRadius(r), trackPhi(phi), trackZzero(zzero), trackTheta(theta), trackCharge(charge),
      trackCurvsign(curvsign) {}

    /** Standard deconstructor */
    virtual ~DATCONTrack() {}

    /** Get track ID */
    unsigned int getTrackID() const { return trackID; }

    /** Get Hit position (or vertex) if existing */
    TVector3 getHitPos() const { return hitPos; }

    /** Get helix parameters */
    /** Get radius of track */
//     double getTrackR() const { return trackRadius; }
    /** Get radius of track */
    double getTrackRadius() const { return trackRadius; }
    /** Get phi of track */
    double getTrackPhi() const { return trackPhi; }
    /** Get d of track */
    double getTrackZzero() const { return trackZzero; }
    /** Get theta of track */
    double getTrackTheta() const { return trackTheta; }
    /** Get (hough) charge of track */
    int getTrackCharge() const { return trackCharge; }
    /** Get curvature of track as integer */
    int getTrackCurvature() const { return trackCurvsign; }

    std::vector<unsigned int> getHitIDList() const { return m_HitIDList; }

    /** Set functions */
    /** Set track ID */
    void setTrackID(unsigned int id) { trackID = id; }

    void setHitIDList(std::vector<unsigned int> hitIDList) { m_HitIDList = hitIDList; }

    /** Set track parameter */
    /**
    * Set track parameter estimated by hough trafo algorithm
    * @param in_r radius of track
    * @param in_phi phi of track
    * @param in_theta theta of track
    * @param in_charge charge of track
    * @param in_curvsign left or right curved track as integer
    */
    void setTrackParam(double r, double phi, double zzero, double theta, int charge, int curvsign)
    {
      trackRadius = r; trackPhi = phi; trackZzero = zzero; trackTheta = theta; trackCharge = charge; trackCurvsign = curvsign;
    }


    /**
    * Set hit position
    */
    void setHitPos(TVector3 pos) { hitPos = pos; }

  private:
    /** List of IDs of hits assigned to this track */
    std::vector<unsigned int> m_HitIDList;
    /** is track straight? */
    bool straight = false;
    /** track ID */
    unsigned int trackID = -1;
    /** radius of track */
    double trackRadius = TMath::QuietNaN();
    /** phi of track */
    double trackPhi = TMath::QuietNaN();
    /** vertical coordinate of the theta Hough Space to compute z0 */
    double trackZzero = TMath::QuietNaN();
    /** theta of track */
    double trackTheta = TMath::QuietNaN();
    /** charge of track */
    int trackCharge = TMath::QuietNaN();
    /** curvature of track as integer */
    int trackCurvsign = TMath::QuietNaN();

    /** position of hit */
    TVector3 hitPos;


    ClassDef(DATCONTrack, 1)

  }; // class DATCONTRACK_H

}; // end namespace Belle2

