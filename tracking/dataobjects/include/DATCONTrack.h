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
  * A DATCONTrack consists of the helix parameters of the track as well as a trackID and a list
  * of hits assinged to the track.
  */

  class DATCONTrack : public TObject {

  public:
    /**
    * Standard constructor
      * @param trackID unique ID of track
      * @param trackRadius radius of track estimated by hough trafo algorithm
      * @param trackPhi phi of track estimated by hough trafo algorithm
      * @param trackZzero vertical coordinate of the theta Hough Space to compute z0
      * @param trackTheta theta of track estimated by hough trafo algorithm
      * @param trackCharge particle charge (-1, 0, +1) of hough track estimated by hough trafo algorithm
      * @param trackCurvsign usual way to define track curvature
      */
    DATCONTrack(): trackID(0), trackRadius(), trackPhi(), trackZzero(), trackTheta(), trackCharge(), trackCurvsign() {}

    /**
      * Constructor with basic values
      * @param trackID unique ID of track
      * @param radius radius of track estimated by hough trafo algorithm
      * @param phi phi of track estimated by hough trafo algorithm
      * @param zzero vertical coordinate of the theta Hough Space to compute z0
      * @param theta theta of track estimated by hough trafo algorithm
      * @param charge particle charge (-1, 0, +1) of hough track estimated by hough trafo algorithm
      * @param curvsign usual way to define track curvature
      */
    DATCONTrack(unsigned int ID, double radius, double phi, double zzero, double theta, int charge, int curvsign):
      trackID(ID), trackRadius(radius), trackPhi(phi), trackZzero(zzero), trackTheta(theta), trackCharge(charge),
      trackCurvsign(curvsign) {}

    /**
      * Constructor with basic values plus hitIDList
      * @param hitIDList list of hits assigned to the track
      * @param trackID unique ID of track
      * @param radius radius of track estimated by hough trafo algorithm
      * @param phi phi of track estimated by hough trafo algorithm
      * @param zzero vertical coordinate of the theta Hough Space to compute z0
      * @param theta theta of track estimated by hough trafo algorithm
      * @param charge particle charge (-1, 0, +1) of hough track estimated by hough trafo algorithm
      * @param curvsign usual way to define track curvature
      */
    DATCONTrack(std::vector<unsigned int> hitIDList, unsigned int ID, double radius, double phi, double zzero, double theta,
                int charge, int curvsign):
      m_HitIDList(hitIDList), trackID(ID), trackRadius(radius), trackPhi(phi), trackZzero(zzero), trackTheta(theta), trackCharge(charge),
      trackCurvsign(curvsign) {}

    /** Standard deconstructor */
    virtual ~DATCONTrack() {}

    /** Get track ID */
    unsigned int getTrackID() const { return trackID; }

    /** Get helix parameters */
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
    /** Get the list of hit assigned to the track */
    std::vector<unsigned int> getHitIDList() const { return m_HitIDList; }

    /** Set functions */
    /** Set list of hits assigned to the track */
    void setHitIDList(std::vector<unsigned int> hitIDList) { m_HitIDList = hitIDList; }


  private:
    /** List of IDs of hits assigned to this track */
    std::vector<unsigned int> m_HitIDList;
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

    ClassDef(DATCONTrack, 1)

  }; // class DATCONTRACK_H

}; // end namespace Belle2

