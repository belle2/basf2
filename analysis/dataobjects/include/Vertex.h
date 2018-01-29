/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VERTEX_H
#define VERTEX_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TMatrixFSym.h>

#include <vector>
#include <set>

namespace Belle2 {



  /**
   * Vertex data object: contains Btag Vertex and DeltaT
   */

  class Vertex : public RelationsObject {

  public:

    /**
      * Default constructor.
      * All private members are set to 0 (all vectors are empty).
      */
    Vertex()
    {
      m_tagVertex(0) = 0; m_tagVertex(1) = 0; m_tagVertex(2) = 0;
      m_tagVertexPval = 0;
      m_deltaT = 0;
      m_deltaTErr = 0;
      m_MCdeltaT = 0;
      m_MCtagV(0) = 0; m_MCtagV(1) = 0; m_MCtagV(2) = 0;
      m_mcPDG = 0;
      resetTagVertexErrorMatrix();
      m_FitType = 0;
      m_NTracks = -1;
    }


    /**
     * Returns BTag Vertex
     */
    TVector3 getTagVertex();

    /**
     * Returns BTag Vertex (3x3) error matrix
     */
    TMatrixFSym getTagVertexErrMatrix();

    /**
     * Returns BTag Vertex P value
     */
    float getTagVertexPval();

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
     * Returns fit algo type
     */
    int getFitType() ;

    /**
     * Returns number of tracks used in the fit
     */
    int getNTracks() ;

    /**
     * Set BTag Vertex
     */
    void setTagVertex(TVector3 TagVertex);

    /**
     *  Set BTag Vertex (3x3) error matrix
     */
    void setTagVertexErrMatrix(TMatrixFSym TagVertexErrMatrix);

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
    void setMCTagVertex(TVector3 MCTagVertex);

    /**
     * Set generated Btag PDG code
     */
    void setMCTagBFlavor(int MCTagBFlavor);

    /**
     * Set generated DeltaT
     */
    void setMCDeltaT(float DeltaT);

    /**
     * Set fit algo type
     */
    void setFitType(float FitType) ;

    /**
     * Set number of tracks used in the fit
     */
    void setNTracks(int nTracks) ;
  private:
    TVector3 m_tagVertex;               /**< Btag vertex */
    TMatrixFSym m_tagVertexErrMatrix;   /**< Btag vertex (3x3) error matrix */
    float m_tagVertexPval;              /**< Btag vertex P value */
    float m_deltaT;                     /**< Delta t */
    float m_deltaTErr;                  /**< Delta t error */
    TVector3 m_MCtagV;                  /**< generated Btag vertex */
    int m_mcPDG;                        /**< generated tag side B flavor (PDG code) */
    float m_MCdeltaT;                   /**< generated Delta t */
    int m_FitType;                      /**< Fit algo used */
    int m_NTracks;                      /**< Number of tracks used in the fit */


    /**
     * Resets 3x3 tag vertex error matrix
     * All elements are set to 0.0
     */
    void resetTagVertexErrorMatrix();


    ClassDef(Vertex, 1) /**< class definition */

  };



} // end namespace Belle2

#endif
