/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_HOUGHTRACK_H
#define SVD_HOUGHTRACK_H

#include <math.h>

#include <root/TObject.h>
#include <root/TVector3.h>
#include <root/TMath.h>


namespace Belle2 {

  /**
   * The SVD Hough Track class.
   *
   * Contains helix and straight track parameters for the SVD Hough
   * tracking algorithm.
   */

  class SVDHoughTrack : public TObject {

  public:
    /**
     * Standard constructor
     * @param trackID unique ID of track
     * @param r radius of track estimated by hough trafo algorithm
     * @param phi phi of track estimated by hough trafo algorithm
     * @param theta theta of track estimated by hough trafo algorithm
     * @param charge particle charge (-1, 0, +1) of hough track estimated by hough trafo algorithm
     * @param curvsign usual way to define track curvature
     * @param left redundant information with curvsign, initialized as false
     */
    //SVDHoughTrack(): trackID(0), r(), phi(), theta(), charge(), curvsign(), left(false) {}
    SVDHoughTrack(): trackID(0), r(), phi(), theta(), charge(), curvsign() {}

    /**
     * Constructor if all values are given
     */
    //SVDHoughTrack(unsigned int ID, double in_r, double in_phi, double in_theta, int in_charge, int in_curvsign, bool in_left = false):
    //  straight(false), trackID(ID), r(in_r), phi(in_phi), theta(in_theta), charge(in_charge), curvsign(in_curvsign), left(in_left) {}
    SVDHoughTrack(unsigned int ID, double in_r, double in_phi, double in_theta, int in_charge, int in_curvsign):
      straight(false), trackID(ID), r(in_r), phi(in_phi), theta(in_theta), charge(in_charge), curvsign(in_curvsign) {}

    /**
     * Constructor for straight track
     * @param m1 slope of track in rphi direction
     * @param a1 intercept of track in rphi direction
     * @param m2 slope of track in theta direction
     * @param a2 intercept of track in theta direction
     */
    SVDHoughTrack(double m1, double a1, double m2, double a2):
      straight(true), m_1(m1), a_1(a1), m_2(m2), a_2(a2) {}

    /** Standard deconstructor */
    virtual ~SVDHoughTrack() {}

    /** Get track ID */
    unsigned int getTrackID() const { return trackID; }

    /** Get Hit position (or vertex) if existing */
    TVector3 getHitPos() const { return hitPos; }

    /** Get type of track */
    bool getTrackType() const { return straight; }

    /** Get helix parameters */
    /** Get radius of track */
    double getTrackR() const { return r; }
    /** Get phi of track */
    double getTrackPhi() const { return phi; }
    /** Get theta of track */
    double getTrackTheta() const { return theta; }
    /** Get (hough) charge of track */
    int getTrackCharge() const { return charge; }
    /** Get curvature of track as integer */
    int getTrackCurvature() const { return curvsign; }
    /** Get curvature of track as bool */
    bool getTrackOrientation() const { return left; }

    /** Get Hough bounding box */
    /** Get slope of track in rphi direction */
    double getTrackM1() const { return m_1; }
    /** Get intercept of track in rphi direction */
    double getTrackA1() const { return a_1; }
    /** Get slope of track in theta direction */
    double getTrackM2() const { return m_2; }
    /** Get intercept of track in theta direction */
    double getTrackA2() const { return a_2; }

    /** Set functions */
    /** Set track ID */
    void setTrackID(unsigned int id) { trackID = id; }

    /** Set track parameter */
    /**
     * Set track parameter estimated by hough trafo algorithm
     * @param in_r radius of track
     * @param in_phi phi of track
     * @param in_theta theta of track
     * @param in_charge charge of track
     * @param in_curvsign left or right curved track as integer
     */
    void setTrackParam(double in_r, double in_phi, double in_theta, int in_charge, int in_curvsign)
    {
      r = in_r; phi = in_phi; theta = in_theta; charge = in_charge; curvsign = in_curvsign;
    }

    /**
     * Set track parameter estimated by hough trafo algorithm
     * @param in_r radius of track
     * @param in_phi phi of track
     * @param in_theta theta of track
     * @param in_charge charge of track
     * @param in_left left or right curved track as bool
     */
    /*
    void setTrackParam(double in_r, double in_phi, double in_theta, int in_charge, bool in_left)
    {
      r = in_r; phi = in_phi; theta = in_theta; charge = in_charge; left = in_left;
    }
    */

    /**
     * Set track parameter estimated by hough trafo algorithm
     * @param in_r radius of track
     * @param in_phi phi of track
     * @param in_theta theta of track
     * @param in_charge charge of track
     * @param in_curvsign left or right curved track as integer
     * @param in_left left or right curved track as bool
     */
    /*
    void setTrackParam(double in_r, double in_phi, double in_theta, int in_charge, int in_curvsign, bool in_left)
    {
      r = in_r; phi = in_phi; theta = in_theta; charge = in_charge; curvsign = in_curvsign; left = in_left;
    }
    */

    /**
     * Set hit position
     */
    void setHitPos(TVector3 pos) { hitPos = pos; }

    /**
     * Set parameter for straight track
     * @param m1 slope of track in rphi direction
     * @param a1 intercept of track in rphi direction
     * @param m2 slope of track in theta direction
     * @param a2 intercept of track in theta direction
     */
    void setStraightParam(double m1, double m2, double a1, double a2)
    {
      straight = true;
      m_1 = m1;
      a_1 = a1;
      m_2 = m2;
      a_2 = a2;
    }

  private:
    /** is track straight? */
    bool straight = false;
    /** track ID */
    unsigned int trackID = -1;
    /** radius of track */
    double r = TMath::QuietNaN();
    /** phi of track */
    double phi = TMath::QuietNaN();
    /** theta of track */
    double theta = TMath::QuietNaN();
    /** charge of track */
    int charge = TMath::QuietNaN();
    /** curvature of track as integer */
    int curvsign = TMath::QuietNaN();
    /** curvature of track as bool */
    bool left;

    /** position of hit */
    TVector3 hitPos;
    /** slope of track in rphi direction */
    double m_1 = TMath::QuietNaN();
    /** intercept of track in rphi direction */
    double a_1 = TMath::QuietNaN();
    /** slope of track in theta direction */
    double m_2 = TMath::QuietNaN();
    /** intercept of track in theta direction */
    double a_2 = TMath::QuietNaN();

    ClassDef(SVDHoughTrack, 1)

  }; // class SVDHoughTrack


} // end namespace Belle2

#endif
