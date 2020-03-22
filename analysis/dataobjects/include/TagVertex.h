/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Stefano Lacaprara, Thibaud Humair         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TMatrixDSym.h>
#include <string>

// DataObjects
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

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
      m_MCdeltaT = 0;
      m_MCdeltaTapprox = 0;
      m_MCtagV(0) = 0; m_MCtagV(1) = 0; m_MCtagV(2) = 0;
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
      m_constraintType = "";
      m_constraintCenter(0) = 0; m_constraintCenter(1) = 0, m_constraintCenter(2) = 0;
    }

    // get methods

    /**
     * Returns BTag Vertex
     */
    TVector3 getTagVertex();

    /**
     * Returns BTag Vertex (3x3) error matrix
     */
    TMatrixDSym getTagVertexErrMatrix();

    /**
     * Returns BTag Vertex P value
     */
    float getTagVertexPval();

    /**
     * Returns a ptr to the tag vtx track indexed by trackIndex
     */
    const TrackFitResult* getVtxFitTrackResultPtr(unsigned int trackIndex);

    /**
     * Returns a ptr to the MC particle matched to the tag vtx track indexed by trackIndex
     */
    const MCParticle* getVtxFitTrackMCParticle(unsigned int trackIndex);

    /**
     * Returns DeltaT
     */
    float getDeltaT();

    /**
     * Returns DeltaTErr
     */
    float getDeltaTErr();

    /**
     * Returns generated BTag Vertex
     */
    TVector3 getMCTagVertex();

    /**
     * Returns generated Btag PDG code
     */
    int getMCTagBFlavor();

    /**
     * Returns generated DeltaT
     */
    float getMCDeltaT();

    /**
     * Returns generated DeltaT
     */
    float getMCDeltaTapprox();

    /**
     * Returns fit algo type
     */
    int getFitType() ;

    /**
     * get the constraint type used in the tag fit
     */

    std::string getConstraintType();

    /**
     * Returns number of tracks used in the fit
     */
    int getNTracks() ;

    /**
     * Returns number of tracks used in the fit (not counting the ones removed because they come from Kshorts)
     */
    int getNFitTracks() ;

    /**
     * Returns the tagV component in the boost direction
     */
    float getTagVl() ;

    /**
     * Returns the MC tagV component in the boost direction
     */
    float getTruthTagVl() ;

    /**
     * Returns the error of the tagV component in the boost direction
     */
    float getTagVlErr() ;

    /**
     * Returns the tagV component in the direction orthogonal to the boost
     */
    float getTagVol() ;

    /**
     * Returns the MC tagV component in the direction orthogonal to the boost
     */
    float getTruthTagVol() ;

    /**
     * Returns the error of the tagV component in the direction orthogonal to the boost
     */
    float getTagVolErr() ;

    /**
     * Returns the number of degrees of freedom in the tag vertex fit
     */
    float getTagVNDF() ;

    /**
     * Returns the chi^2 value of the tag vertex fit result
     */
    float getTagVChi2() ;

    /**
     * Returns the IP component of the chi^2 value of the tag vertex fit result
     */
    float getTagVChi2IP() ;

    /**
     *
     *  Returns the position of the constraint, ie centre of the constraint ellipse
     */

    TVector3 getConstraintCenter();

    /**
     * Get the covariance matrix of the constraint for the tag fit
     */

    TMatrixDSym getConstraintCov();

    /**
     * Returns the position vector (X, Y, Z) of the tag track indexed by trackindex
     */

    TVector3 getVtxFitTrackPosition(unsigned int trackIndex);

    /**
     * Returns the momentum vector of the tag track indexed by trackindex
     */

    TVector3 getVtxFitTrackP(unsigned int trackIndex);

    /**
     * Returns one of the 3 components of the momentum of tag track indexed by trackindex
     */
    double getVtxFitTrackPComponent(unsigned int trackIndex, unsigned int component);

    /**
     * Returns the longitudinal distance from the IP to the POCA of the tag track indexed by trackIndex
     */

    double getVtxFitTrackZ0(unsigned int trackIndex);

    /**
     * Returns the radial distance from the IP to the POCA of the tag track indexed by trackIndex
     */

    double getVtxFitTrackD0(unsigned int trackIndex);

    /**
     * Returns the weight assigned by Rave to the track indexed by trackIndex
     */

    double getRaveWeight(unsigned int trackIndex);

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
    void setMCTagVertex(const TVector3& MCTagVertex);

    /**
     * Set generated Btag PDG code
     */
    void setMCTagBFlavor(int MCTagBFlavor);

    /**
     * Set generated DeltaT
     */
    void setMCDeltaT(float DeltaT);

    /**
     * Set generated DeltaTapprox
     */
    void setMCDeltaTapprox(float DeltaT);

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
    void setVertexFitTracks(std::vector<const TrackFitResult*> const& vtxFitTracks);

    /**
     * Set a vector of pointers to the MC p'cles corresponding to the tracks in the tag vtx fit
     */
    void setVertexFitMCParticles(std::vector<const MCParticle*> const& vtxFitMCParticles);

    /**
     * Set the weights used by Rave in the tag vtx fit
     */
    void setRaveWeights(std::vector<double> const& raveWeights);

    /**
     * Set the centre of the constraint for the tag fit
     */

    void setConstraintCenter(TVector3 const& constraintCenter);

    /**
     * Set the covariance matrix of the constraint for the tag fit
     */

    void setConstraintCov(TMatrixDSym const& constraintCov);

    /**
     * Set the type of the constraint for the tag fit
     */

    void setConstraintType(std::string const& constraintType);


  private:
    TVector3 m_tagVertex;               /**< Btag vertex */
    TMatrixDSym m_tagVertexErrMatrix;   /**< Btag vertex (3x3) error matrix */
    float m_tagVertexPval;              /**< Btag vertex P value */
    float m_deltaT;                     /**< Delta t */
    float m_deltaTErr;                  /**< Delta t error */
    TVector3 m_MCtagV;                  /**< generated Btag vertex */
    int m_mcPDG;                        /**< generated tag side B flavor (PDG code) */
    float m_MCdeltaT;                   /**< generated Delta t */
    float m_MCdeltaTapprox;             /**< generated Delta t approximated */
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
    std::vector<const TrackFitResult*> m_vtxFitTracks; /**< pointers to the tracks used by rave to fit the vertex */
    std::vector<const MCParticle*> m_vtxFitMCParticles; /**< pointers to the MC p'cles corresponding to the tracks in the tag vtx fit */
    int m_NFitTracks;                   /**< Number of tracks used by Rave to fit the vertex */
    std::vector<double> m_raveWeights;  /**< weights of each track in the Rave tag vtx fit */
    std::string m_constraintType;       /**< Type of the constraint used for the tag vertex fit (noConstraint, IP, Boost, Tube) */
    TVector3 m_constraintCenter;        /**< centre of the constraint */
    TMatrixDSym m_constraintCov;        /**< covariance matrix associated to the constraint, ie size of the constraint */


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

    ClassDef(TagVertex, 4) /**<
           4. Add info related to fit tracks, rave weights
           3. Add NDF, Chi2, Chi2IP
           2. Name to contain "Tag"
           1. class definition
                             */

  };

} // end namespace Belle2
