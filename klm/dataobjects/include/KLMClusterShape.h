/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Belle II Collaboration                                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tommy Lam                                                *
 **************************************************************************/
#pragma once

/* Belle 2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>


#include <TVectorT.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixT.h>

namespace Belle2 {

  /**
   * Variable for KLM cluster shape analysis
   */
  class KLMClusterShape : public RelationsObject {

  public:

    /**
     * Empty Constructor for ROOT IO (needed to make the class storeable).
     */
    KLMClusterShape();

    /**
     * Copy constructor
     */
    KLMClusterShape(const KLMClusterShape&);

    //! Assignment operator
    KLMClusterShape& operator=(const KLMClusterShape&);

    /**
     * Destructor.
     */
    ~KLMClusterShape()
    {
    }

    //Define get functions to obtain private information


    /**Get principal axis eigenvector. */
    float getVariance1() const
    {return abs(m_primaryVec[3]);}

    /**Get secondary axis eigenvector. */
    float getVariance2() const
    {return abs(m_secondaryVec[3]);}

    /**Get tertiary axis eigenvector. */
    float getVariance3() const
    {return abs(m_tertiaryVec[3]);}

    /**Get eigenvectors. */

    /**Get principal axis eigenvector. */
    TVector3 getPrimaryVector();

    /**Get secondary axis eigenvector. */
    TVector3 getSecondaryVector();

    /**Get tertiary axis eigenvector. */
    TVector3 getTertiaryVector();

    /**Get number of hits used in a cluster */
    int getNHits() const
    {return (int) m_nHits;}


    /**Set eigenvectors and eigenvalues. */
    void setEigen(TMatrixT<float> eigenList);

    /**Set number of hits */
    void setNHits(int nHits)
    {
      m_nHits = nHits;
    }



  private:

    /** Number of hits */
    int m_nHits;


    //Normalization for eigenvectors should be unity.
    //Indexing: 0-2 is unit eigenvector, 3 is eigenvalue.

    /**Principal axis eigenvector and eigenvalue. */
    float m_primaryVec[4];

    /**Principal axis eigenvector and eigenvalue. */
    float m_secondaryVec[4];

    /**Principal axis eigenvector and eigenvalue. */
    float m_tertiaryVec[4];

    /**  Needed to make the ROOT object storable */
    ClassDef(KLMClusterShape, 1)

  }; // end of public RelationsObject

}// end of namespace Belle2

