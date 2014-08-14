/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#ifndef SVD_HOUGHTRACK_H
//#define SVD_HOUGHTRACK_H

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
    SVDHoughTrack(): trackID(0), r(), phi(), theta() {}

    SVDHoughTrack(unsigned int ID, double in_r, double in_phi, double in_theta):
      straight(false), trackID(ID), r(in_r), phi(in_phi), theta(in_theta) {}

    SVDHoughTrack(double m1, double a1, double m2, double a2):
      straight(true), m_1(m1), a_1(a1), m_2(m2), a_2(a2) {}

    /** Get track ID */
    unsigned int getTrackID() { return trackID; }

    /** Get Hit position (or vertex) if existing */
    TVector3 getHitPos() { return hitPos; }

    /** Get type of track */
    bool getTrackType() { return straight; }

    /** Get helix parameters */
    double getTrackR() { return r; }
    double getTrackPhi() { return phi; }
    double getTrackTheta() { return theta; }

    /** Get Hough bounding box */
    double getTrackM1() { return m_1; }
    double getTrackA1() { return a_1; }
    double getTrackM2() { return m_2; }
    double getTrackA2() { return a_2; }

    /** Set functions */
    void setTrackID(unsigned int id) { trackID = id; }

    void setTrackParam(double in_r, double in_phi, double in_theta) {
      r = in_r; phi = in_phi; theta = in_theta;
    }

    void setHitPos(TVector3 pos) { hitPos = pos; }

    void setStraightParam(double m1, double m2, double a1, double a2) {
      straight = true;
      m_1 = m1;
      a_1 = a1;
      m_2 = m2;
      a_2 = a2;
    }

  private:
    bool straight = false;
    unsigned int trackID;
    double r, phi, theta;
    TVector3 hitPos;
    double m_1, a_1, m_2, a_2;

    ClassDef(SVDHoughTrack, 1)

  }; // class SVDHoughTrack


} // end namespace Belle2

//#endif
