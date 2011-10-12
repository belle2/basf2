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
#include <TVector3.h>

namespace Belle2 {

  /** Track parameters. */
  /** This Track class is based on the track parametrisation used at Belle. It is however has to be checked if it is really consistent with the Belle parametrisation. It is also still under discussion which parametrisation will be the one we want to use.
   */
  class Track : public TObject {
  public:

    /** Constructor. */
    /** This is as well the parameter free I/O constructor.
     */

    Track() {
      //set all member variables to some starting values
      m_d0 = -999;
      m_phi = -999;
      m_kappa = -999;
      m_z0 = -999;
      m_tanLambda = -999;

      m_nHits = -999;
      m_chi2 = -999;
      m_pValue = -999;
      m_mcId = -999;
      m_pdgHypo = -999;
      m_purity = -999;
      m_fitFailed = false;
      m_extrapFailed = false;

      m_pErrors.SetX(-999);
      m_pErrors.SetY(-999);
      m_pErrors.SetZ(-999);

      m_vertexErrors.SetX(-999);
      m_vertexErrors.SetY(-999);
      m_vertexErrors.SetZ(-999);

      m_dEdx = -999;
      m_dEdxSigma = -999;
      m_radiusOfInnermostHit = -999;
    }

    /** Destructor. */
    ~Track() {
    }

    // Getters for all the parameters.
    //helix parameters
    /** Getter for impact parameter.
     */
    const float getD0() { return m_d0; }

    /** Getter for phi.
     */
    const float getPhi() { return m_phi; }

    /** Getter for kappa.
      */
    const float getKappa() {return m_kappa; }

    /** Getter for z0.
     */
    const float getZ0() {return m_z0; }

    /** Getter for tanLambda.
    */
    const float getTanLambda() { return m_tanLambda;}

    //non-helix parameters.
    /** Getter for Chi2 of the track fit.
     */
    const float getChi2() {return m_chi2;}

    /** Getter for the probability value of the track fit.
     */
    const float getPValue() {return m_pValue;}
    /** Getter for total number of tracker hits.
     */
    const int getNHits() {return m_nHits;}

    /** Getter of the MCParticle Id.
     */
    const int getMCId() { return m_mcId; }

    /** Getter for the pdg hypothesis under which this track was fitted.
     */
    const int getPDG() {return m_pdgHypo;}

    /** Getter for the purity of the track.
     */
    const float getPurity() { return m_purity;}

    /** Returns true if the fit of this track failed.
     */
    const bool getFitFailed() {return m_fitFailed;}

    /** Return true if the extrapolation of this track failed.
     */
    const bool getExtrapFailed() {return m_extrapFailed;}

    /** Getter for momentum errors.
     */
    const TVector3 getPErrors() {return m_pErrors;}
    /** Getter for vertex errors.
     */
    const TVector3 getVertexErrors() {return m_vertexErrors;}

    /**Getter for energy loss in the tracking detectors.
     */
    const float getdEdx() { return m_dEdx;}

    /** Getter for uncertainty on energy loss in tracking detectors.
     */
    const float getdEdxSigma() { return m_dEdxSigma;}

    /** Getter for radius of innermost hit contributing to the track.
     */
    const float getRadiusOfInnermostHit() { return m_radiusOfInnermostHit;}


    // Setters for all the parameters.
    //helix parameters
    /** Setter for impact parameter.
     */
    void setD0(const float& d0) { m_d0 = d0;}

    /** Setter for phi
     */
    void setPhi(const float& phi) {m_phi = phi;}

    /** Setter for kappa.
     */
    void setKappa(const float& kappa) {m_kappa = kappa;}

    /** Setter for z0.
     */
    void setZ0(const float& z0) {m_z0 = z0;}

    /** Setter for tanLambda.
     */
    void setTanLambda(const float& tanLambda) {m_tanLambda = tanLambda;}

    //non-helix parameters.
    /** Setter for Chi2 of the track fit.
     */
    void setChi2(const float& chi2) {m_chi2 = chi2;}

    /** Setter for the probabity value of the track fit.
     */
    void setPValue(const float& pValue) {m_pValue = pValue;};

    /** Setter for number of tracker hits.
     */
    void setNHits(const int& nHits) {
      m_nHits = nHits;
    }
    /** Setter for the MCParticle Id.
     */
    void setMCId(const int& mcId) {m_mcId = mcId;};

    /** Setter for the pdg hypothesis.
     */
    void setPDG(const int& pdg) { m_pdgHypo = pdg;};

    /** Setter for the purity of the track.
     */
    void setPurity(const float& purity) { m_purity = purity;};

    /** Set if the fit was successful or failed.
     */
    void setFitFailed(const bool& fitFailed) { m_fitFailed = fitFailed;};

    /** Set if the extrapolation was successful or failed.
     */
    void setExtrapFailed(const bool& extrapFailed) {m_extrapFailed = extrapFailed;};

    /** Setter for momentum errors.
     */
    void setPErrors(const TVector3& pErrors) { m_pErrors = pErrors  ;};

    /** Setter for momentum errors.
     */
    void setPErrors(const double& px, const double& py, const double& pz) {
      m_pErrors.SetX(px);
      m_pErrors.SetY(py);
      m_pErrors.SetZ(pz);
    }

    /** Setter for vertex errors.
    */
    void setVertexErrors(const TVector3& vertexErrors) { m_vertexErrors = vertexErrors  ;}
    /** Setter for vertex errors.
     */
    void setVertexErrors(const double& x, const double& y, const double& z) {
      m_vertexErrors.SetX(x);
      m_vertexErrors.SetY(y);
      m_vertexErrors.SetZ(z);
    }

    /** Setter for energy loss in the tracking detectors.
     */
    void setdEdx(const float& dEdx) { m_dEdx = dEdx;}

    /** Setter for uncertainty on energy loss in tracking detectors.
     */
    void setdEdxSigma(const float& dEdxSigma) { m_dEdxSigma = dEdxSigma; }

    /** Setter for radius of innermost hit contributing to the track.
     */
    void setRadiusOfInnermostHit(const float& radiusOfInnermostHit) {m_radiusOfInnermostHit = radiusOfInnermostHit;}











  private:

    //helix parameters

    /** Impact Parameter.
        The Impact Parameter is the two-dimensional point of closest approach of a track to the origin.
        A sign is assigned such, that if the projection of the momentum on this distance points to the origin,
        d0 is negative.
     */
    float m_d0;

    /** Phi at the perigee [0, 2pi].
        This is the angle of the track momentum at the perigee in the plane perpendicular to the detector axis.
     */
    float m_phi;

    /** Signed reciprocal transvers momentum of the track (1/Pt).
        Pt is measured in GeV.
        It is negative if we have a negatively charged track leaving the origin.
     */
    float m_kappa;

    /** z Position of the perigee.
        z is parallel to the detector axis.
     */
    float m_z0;

    /** Tangens of the dip angle lambda.
        This equals dz/ds with s the path length in R-Phi at the perigee.
     */
    float m_tanLambda;

    //non-helix parameters

    /** Total number of tracking detectors (CDC, SVD, PXD) hits used for this track
     */
    int m_nHits;

    /** Chi2 of track fit.
        This is a measure for the quality of the track.
     */
    float m_chi2;

    /** Probability value of track fit.
        This is a measure for the quality of the track.
     */
    float m_pValue;

    /** ID of the MCParticle which created this tracks.
     *
     */
    int m_mcId;

    /** PDG hypothesis under which this track was fitted.
     *
     */
    int m_pdgHypo;

    /** Purity of the track (number of hits coming from original MCParticle/total number of hits  * 100)
     *  Is always 100 for MCTracks, but is useful to evaluate the performance of pattern recognition.
     */
    float m_purity;

    /** Mark is the fit of this track was successful;
     *  True if the fit failed.
     */
    bool m_fitFailed;

    /** Mark is the extrapolation of the fit results was successful;
     *  True if the extrapolation failed.
     */
    bool m_extrapFailed;

    /** Variances on momentum.
     *
     */
    TVector3 m_pErrors;

    /** Variances on vertex position.
     *
     */
    TVector3 m_vertexErrors;


    //these parameters are not used yet ...

    /** dE/dx in the tracking detectors.
        For the moment, this is intended to be used with the CDC.
        We have to see, how to handle the energy loss in the other detectors.
     */
    float m_dEdx;

    /** Uncertainty on energy loss in tracking detectors.
     */
    float m_dEdxSigma;

    /** Radius of innermost hit contributing to the track.
        This gives a hint, if we may have to do it with a track from a V0 particle.
    */
    float m_radiusOfInnermostHit;


    /** ROOT Macro to make Track a ROOT class.*/
    ClassDef(Track, 1);

  }; //class
} // namespace Belle2
#endif // TRACK
