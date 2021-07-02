/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TMatrixDSym.h>
#include <string>

// DataObjects
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {



  /**
   * TagVertex data object: contains Btag Vertex and DeltaT
   */

  class TagVertex : public RelationsObject {

  public:

    /**
      * Default constructor.
      * All private members are set to 0 (all vectors are empty).
      */
    TagVertex()
    {
      m_tagVertex(0) = 0; m_tagVertex(1) = 0; m_tagVertex(2) = 0;
      m_tagVertexPval = 0;
      m_deltaT = 0;
      m_deltaTErr = 0;
      m_mcDeltaTau = 0;
      m_mcDeltaT = 0;
      m_mcTagV(0) = 0; m_mcTagV(1) = 0; m_mcTagV(2) = 0;
      m_mcPDG = 0;
      resetTagVertexErrorMatrix();
      resetConstraintCov();
      m_FitType = 0;
      m_NTracks = -1;
      m_tagVl = 0;
      m_tagVlErr = 0;
      m_tagVol = 0;
      m_tagVolErr = 0;
      m_truthTagVl = 0;
      m_truthTagVol = 0;
      m_tagVNDF = 0;
      m_tagVChi2 = 0;
      m_tagVChi2IP = 0;
      m_NFitTracks = 0;
      m_constraintType = "";
      m_constraintCenter(0) = 0; m_constraintCenter(1) = 0, m_constraintCenter(2) = 0;
      m_fitTruthStatus = 0;
      m_rollbackStatus = 0;
    }

    // get methods

    /**
     * Returns BTag Vertex
     */
    TVector3 getTagVertex() const;

    /**
     * Returns BTag Vertex (3x3) error matrix
     */
    TMatrixDSym getTagVertexErrMatrix() const;

    /**
     * Returns BTag Vertex P value
     */
    float getTagVertexPval() const;

    /**
     * Returns a ptr to the particle constructed from the tag vtx track indexed by trackIndex
     */
    const Particle* getVtxFitParticle(unsigned int trackIndex) const;

    /**
     * Returns a ptr to the MC particle matched to the tag vtx track indexed by trackIndex
     */
    const MCParticle* getVtxFitMCParticle(unsigned int trackIndex) const;

    /**
     * Returns DeltaT
     */
    float getDeltaT() const;

    /**
     * Returns DeltaTErr
     */
    float getDeltaTErr() const;

    /**
     * Returns generated BTag Vertex
     */
    TVector3 getMCTagVertex() const;

    /**
     * Returns generated Btag PDG code
     */
    int getMCTagBFlavor() const;

    /**
     * Returns generated DeltaTau
     */
    float getMCDeltaTau() const;

    /**
     * Returns mc DeltaT (in kin. approximation)
     */
    float getMCDeltaT() const;

    /**
     * Returns fit algo type
     */
    int getFitType() const;

    /**
     * get the constraint type used in the tag fit
     */

    std::string getConstraintType() const;

    /**
     * Returns number of tracks used in the fit
     */
    int getNTracks() const;

    /**
     * Returns number of tracks used in the fit (not counting the ones removed because they come from Kshorts)
     */
    int getNFitTracks() const;

    /**
     * Returns the tagV component in the boost direction
     */
    float getTagVl() const;

    /**
     * Returns the MC tagV component in the boost direction
     */
    float getTruthTagVl() const;

    /**
     * Returns the error of the tagV component in the boost direction
     */
    float getTagVlErr() const;

    /**
     * Returns the tagV component in the direction orthogonal to the boost
     */
    float getTagVol() const;

    /**
     * Returns the MC tagV component in the direction orthogonal to the boost
     */
    float getTruthTagVol() const;

    /**
     * Returns the error of the tagV component in the direction orthogonal to the boost
     */
    float getTagVolErr() const;

    /**
     * Returns the number of degrees of freedom in the tag vertex fit
     */
    float getTagVNDF() const;

    /**
     * Returns the chi^2 value of the tag vertex fit result
     */
    float getTagVChi2() const;

    /**
     * Returns the IP component of the chi^2 value of the tag vertex fit result
     */
    float getTagVChi2IP() const;

    /**
     *
     *  Returns the position of the constraint, ie centre of the constraint ellipse
     */

    TVector3 getConstraintCenter() const;

    /**
     * Get the covariance matrix of the constraint for the tag fit
     */

    TMatrixDSym getConstraintCov() const;

    /**
     * Returns the position vector (X, Y, Z) of the tag track indexed by trackindex
     */

    TVector3 getVtxFitTrackPosition(unsigned int trackIndex) const;

    /**
     * Returns the momentum vector of the tag track indexed by trackindex
     */

    TVector3 getVtxFitTrackP(unsigned int trackIndex) const;

    /**
     * Returns one of the 3 components of the momentum of tag track indexed by trackindex
     */
    double getVtxFitTrackPComponent(unsigned int trackIndex, unsigned int component) const;

    /**
     * Returns the longitudinal distance from the IP to the POCA of the tag track indexed by trackIndex
     */

    double getVtxFitTrackZ0(unsigned int trackIndex) const;

    /**
     * Returns the radial distance from the IP to the POCA of the tag track indexed by trackIndex
     */

    double getVtxFitTrackD0(unsigned int trackIndex) const;

    /**
     * Returns the weight assigned by Rave to the track indexed by trackIndex
     */

    double getRaveWeight(unsigned int trackIndex) const;

    /**
     * Get the status of the fit performed with the truth info of the tracks
     */
    int getFitTruthStatus() const;

    /**
     * Get the status of the fit performed with the rolled back tracks
     */
    int getRollBackStatus() const;

    // set methods

    /**
     * Set BTag Vertex
     */
    void setTagVertex(const TVector3& TagVertex);

    /**
     *  Set BTag Vertex (3x3) error matrix
     */
    void setTagVertexErrMatrix(const TMatrixDSym& TagVertexErrMatrix);

    /**
     * Set BTag Vertex P value
     */
    void setTagVertexPval(float TagVertexPval);

    /**
     * Set DeltaT
     */
    void setDeltaT(float DeltaT);

    /**
     * Set DeltaTErr
     */
    void setDeltaTErr(float DeltaTErr);

    /**
     * Set generated BTag Vertex
     */
    void setMCTagVertex(const TVector3& mcTagVertex);

    /**
     * Set generated Btag PDG code
     */
    void setMCTagBFlavor(int mcTagBFlavor);

    /**
     * Set generated DeltaT
     */
    void setMCDeltaTau(float mcDeltaTau);

    /**
     * Set generated DeltaT (in kin. approx.)
     */
    void setMCDeltaT(float mcDeltaT);

    /**
     * Set fit algo type
     */
    void setFitType(float FitType) ;

    /**
     * Set number of tracks used in the fit
     */
    void setNTracks(int nTracks) ;

    /**
     * Set the tagV component in the boost direction
     */
    void setTagVl(float TagVl) ;

    /**
     * Set the MC tagV component in the boost direction
     */
    void setTruthTagVl(float TruthTagVl) ;

    /**
     * Set the error of the tagV component in the boost direction
     */
    void setTagVlErr(float TagVlErr) ;

    /**
     * Set the tagV component in the direction orthogonal to the boost
     */
    void setTagVol(float TagVol) ;

    /**
     * Set the tagV component in the direction orthogonal to the boost
     */
    void setTruthTagVol(float TruthTagVol) ;

    /**
     * Set the error of the tagV component in the direction orthogonal to the boost
     */
    void setTagVolErr(float TagVolErr) ;

    /**
     * Set the number of degrees of freedom in the tag vertex fit
     */
    void setTagVNDF(float TagVNDF) ;

    /**
     * Set the chi^2 value of the tag vertex fit result
     */
    void setTagVChi2(float TagVChi2) ;

    /**
     * Set the IP component of the chi^2 value of the tag vertex fit result
     */
    void setTagVChi2IP(float TagVChi2IP) ;

    /**
     * Set a vector of pointers to the tracks used in the tag vtx fit
     */
    void setVertexFitParticles(const std::vector<const Particle*>& vtxFitParticles);

    /**
     * Set a vector of pointers to the MC p'cles corresponding to the tracks in the tag vtx fit
     */
    void setVertexFitMCParticles(const std::vector<const MCParticle*>& vtxFitMCParticles);

    /**
     * Set the weights used by Rave in the tag vtx fit
     */
    void setRaveWeights(const std::vector<double>& raveWeights);

    /**
     * Set the centre of the constraint for the tag fit
     */

    void setConstraintCenter(const TVector3& constraintCenter);

    /**
     * Set the covariance matrix of the constraint for the tag fit
     */

    void setConstraintCov(const TMatrixDSym& constraintCov);

    /**
     * Set the type of the constraint for the tag fit
     */

    void setConstraintType(const std::string& constraintType);

    /**
     * Set the status of the fit performed with the truth info of the tracks
     */
    void setFitTruthStatus(int truthStatus);

    /**
     * Set the status of the fit performed with the rolled back tracks
     */
    void setRollBackStatus(int backStatus);

  private:
    TVector3 m_tagVertex;               /**< Btag vertex */
    TMatrixDSym m_tagVertexErrMatrix;   /**< Btag vertex (3x3) error matrix */
    float m_tagVertexPval;              /**< Btag vertex P value */
    float m_deltaT;                     /**< Delta t */
    float m_deltaTErr;                  /**< Delta t error */
    TVector3 m_mcTagV;                  /**< generated Btag vertex */
    int m_mcPDG;                        /**< generated tag side B flavor (PDG code) */
    float m_mcDeltaTau;                 /**< generated Delta t: difference between signal and tag flight times */
    float m_mcDeltaT;                   /**< generated Delta t approximated: true Delta L divided by Upsilon(4S)'s boost */
    int m_FitType;                      /**< Fit algo used */
    int m_NTracks;                      /**< Number of tracks used in the fit */
    float m_tagVl;                      /**< tagV component in the boost direction  */
    float m_truthTagVl;                 /**< MC tagV component in the boost direction  */
    float m_tagVlErr;                   /**< Error of the tagV component in the boost direction  */
    float m_tagVol;                     /**< tagV component in the direction orthogonal to the boost */
    float m_truthTagVol;                /**< MC tagV component in the direction orthogonal to the boost */
    float m_tagVolErr;                  /**< Error of the tagV component in the direction orthogonal to the boost */
    float m_tagVNDF;                    /**< Number of degrees of freedom in the tag vertex fit */
    float m_tagVChi2;                   /**< chi^2 value of the tag vertex fit result */
    float m_tagVChi2IP;                 /**< IP component of chi^2 value of the tag vertex fit result */
    std::vector<const Particle*> m_vtxFitParticles; /**< pointers to the tracks used by rave to fit the vertex */
    std::vector<const MCParticle*> m_vtxFitMCParticles; /**< pointers to the MC p'cles corresponding to the tracks in the tag vtx fit */
    int m_NFitTracks;                   /**< Number of tracks used by Rave to fit the vertex */
    std::vector<double> m_raveWeights;  /**< weights of each track in the Rave tag vtx fit */
    std::string m_constraintType;       /**< Type of the constraint used for the tag vertex fit (noConstraint, IP, Boost, Tube) */
    TVector3 m_constraintCenter;        /**< centre of the constraint */
    TMatrixDSym m_constraintCov;        /**< covariance matrix associated to the constraint, ie size of the constraint */
    int m_fitTruthStatus;               /**< status of the fit when fitted with the truth info of the tracks */
    int m_rollbackStatus;               /**< status of the fit when fitted with rolled back tracks */


    /**
     * Resets 3x3 tag vertex error matrix
     * All elements are set to 0.0
     */
    void resetTagVertexErrorMatrix();

    /**
     * Resets 3x3 constraint error matrix
     * All elements are set to 0.0
     */
    void resetConstraintCov();

    ClassDef(TagVertex, 5) /**<
           5. Renamed variables related to DeltaZ and DeltaT
           4. Add info related to fit tracks, rave weights
           3. Add NDF, Chi2, Chi2IP
           2. Name to contain "Tag"
           1. class definition
                             */

  };

} // end namespace Belle2
