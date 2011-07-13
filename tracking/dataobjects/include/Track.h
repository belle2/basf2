/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACK_H
#define TRACK_H

#include <TObject.h>

namespace Belle2 {

  /** Track parameters. */
  /** This Track class is based on the perigee parametrisation.
   */
  class Track : public TObject {
  public:

    /** Constructor. */
    /** This is as well the parameter free I/O constructor.
     */
    Track() {
    }

    /** Destructor. */
    ~Track() {
    }

    // Getters for all the parameters.
    /** Getter for Impact Parameter.
     */
    float getD0() {
      return m_d0;
    }

    /** Getter for phi
     */
    float getPhi() {
      return m_phi;
    }

    /** Getter for omega.
      */
    float getOmega() {
      return m_omega;
    }

    /** Getter for z0.
     */
    float getZ0() {
      return m_z0;
    }

    /** Getter for cotTheta
    */
    float getCotTheta() {
      return m_cotTheta;
    }

    //Some non-helix parameters.
    /** Getter for Chi2 of track.
     */
    float getChi2() {
      return m_chi2;
    }

    /**Getter for energy loss in the tracking detectors.
     */
    float getdEdx() {
      return m_dEdx;
    }

    /** Getter for uncertainty on energy loss in tracking detectors.
     */
    float getdEdxSigma() {
      return m_dEdxSigma;
    }

    /** Getter for total number of tracker hits.
     */
    int getNHits() {
      return m_nHits;
    }

    /** Getter for radius of innermost hit contributing to the track.
     */
    float getRadiusOfInnermostHit() {
      return m_radiusOfInnermostHit;
    }

    // Setters for all the parameters.
    /** Setter for Impact Parameter.
     */
    void setD0(const float& d0) {
      m_d0 = d0;
    }

    /** Setter for phi
     */
    void setPhi(const float& phi) {
      m_phi = phi;
    }

    /** Setter for omega.
     */
    void setOmega(const float& omega) {
      m_omega = omega;
    }

    /** Setter for z0.
     */
    void setZ0(const float&z0) {
      m_z0 = z0;
    }

    /** Setter for cotTheta
     */
    void setCotTheta(const float& cotTheta) {
      m_cotTheta = cotTheta;
    }

    //Some non-helix parameters.
    /** Setter for Chi2 of track.
     */
    void setChi2(const float& chi2) {
      m_chi2 = chi2;
    }

    /** Setter for energy loss in the tracking detectors.
     */
    void setdEdx(const float& dEdx) {
      m_dEdx = dEdx;
    }

    /** Setter for uncertainty on energy loss in tracking detectors.
     */
    void setdEdxSigma(const float& dEdxSigma) {
      m_dEdxSigma = dEdxSigma;
    }

    /** Setter for number of tracker hits.
     */
    void setNHits(const int& nHits) {
      m_nHits = nHits;
    }

    /** Setter for radius of innermost hit contributing to the track.
     */
    void setRadiusOfInnermostHit(const float& radiusOfInnermostHit) {
      m_radiusOfInnermostHit = radiusOfInnermostHit;
    }

  private:

    /** Impact Parameter.
        The Impact Parameter is the two-dimensional point of closest approach of a track to the origin.
        A sign is assigned such, that if the projection of the momentum on this distance points to the origin,
        d0 is negative.
     */
    float m_d0;

    /** Phi at the perigee [-pi, pi].
        This is the angle of the track momentum at the perigee in the plane perpendicular to the detector axis.
     */
    float m_phi;

    /** Signed curvature of the track.
        The curvature is 1/(R) where R is the radius of the track measured in cm.
        It is negative if we have a negatively charged track leaving the origin.
     */
    float m_omega;

    /** z Position of the perigee.
        z is parallel to the detector axis.
     */
    float m_z0;

    /** Cotangens of polar angle theta.
        This equals dz/ds with s the path length in R-Phi at the perigee.
     */
    float m_cotTheta;

    /** Chi2 of track fit.
        This is a measure for the quality of the track.
     */
    float m_chi2;

    /** dE/dx in the tracking detectors.
        For the moment, this is intended to be used with the CDC.
        We have to see, how to handle the energy loss in the other detectors.
     */
    float m_dEdx;

    /** Uncertainty on energy loss in tracking detectors.
     */
    float m_dEdxSigma;

    /** Total number of tracking detectors (CDC, SVD, PXD) hits used for this track
     */
    int m_nHits;

    /** Radius of innermost hit contributing to the track.
        This gives a hint, if we may have to do it with a track from a V0 particle.
    */
    float m_radiusOfInnermostHit;

    /** ROOT Macro to make Track a ROOT class.*/
    ClassDef(Track, 1);

  }; //class
} // namespace Belle2
#endif // TRACK
