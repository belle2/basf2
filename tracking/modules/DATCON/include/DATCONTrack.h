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
    //DATCONTrack(): trackID(0), r(), phi(), theta(), charge(), curvsign(), left(false) {}
    //     DATCONTrack(): trackID(0), r(), phi(), theta(), charge(), curvsign() {}
    DATCONTrack(): trackID(0), r(), phi(), d(), theta(), charge(), curvsign() {}

    /**
    * Constructor if all values are given
    */
    DATCONTrack(unsigned int ID, double in_r, double in_phi, double in_d, double in_theta, int in_charge, int in_curvsign):
      trackID(ID), r(in_r), phi(in_phi), d(in_d), theta(in_theta), charge(in_charge), curvsign(in_curvsign) {}


    DATCONTrack(std::vector<unsigned int> hitIDList, unsigned int ID, double in_r, double in_phi, double in_d, double in_theta,
                int in_charge, int in_curvsign):
      m_HitIDList(hitIDList), trackID(ID), r(in_r), phi(in_phi), d(in_d), theta(in_theta), charge(in_charge), curvsign(in_curvsign) {}

    /** Standard deconstructor */
    virtual ~DATCONTrack() {}

    /** Get track ID */
    unsigned int getTrackID() const { return trackID; }

    /** Get Hit position (or vertex) if existing */
    TVector3 getHitPos() const { return hitPos; }

    /** Get helix parameters */
    /** Get radius of track */
    double getTrackR() const { return r; }
    /** Get radius of track */
    double getTrackRadius() const { return r; }
    /** Get phi of track */
    double getTrackPhi() const { return phi; }
    /** Get d of track */
    double getTrackd() const { return d; }
    /** Get theta of track */
    double getTrackTheta() const { return theta; }
    /** Get (hough) charge of track */
    int getTrackCharge() const { return charge; }
    /** Get curvature of track as integer */
    int getTrackCurvature() const { return curvsign; }

    std::vector<unsigned int> getHitIDList() const { return m_HitIDList; }

    /** Setter functions */
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
    void setTrackParam(double in_r, double in_phi, double in_d, double in_theta, int in_charge, int in_curvsign)
    {
      r = in_r; phi = in_phi; d = in_d; theta = in_theta; charge = in_charge; curvsign = in_curvsign;
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
    double r = TMath::QuietNaN();
    /** phi of track */
    double phi = TMath::QuietNaN();
    /** vertical coordinate of the theta Hough Space to compute z0 */
    double d = TMath::QuietNaN();
    /** theta of track */
    double theta = TMath::QuietNaN();
    /** charge of track */
    int charge = TMath::QuietNaN();
    /** curvature of track as integer */
    int curvsign = TMath::QuietNaN();


    /** position of hit */
    TVector3 hitPos;


    ClassDef(DATCONTrack, 1)

  }; // class DATCONTRACK_H

}; // end namespace Belle2
