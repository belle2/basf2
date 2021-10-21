/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

#include <TMatrixT.h>
#include <Math/Vector3D.h>

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
    double getVariance1() const
    {return abs(m_primaryVec[3]);}

    /**Get secondary axis eigenvector. */
    double getVariance2() const
    {return abs(m_secondaryVec[3]);}

    /**Get tertiary axis eigenvector. */
    double getVariance3() const
    {return abs(m_tertiaryVec[3]);}

    /**Get eigenvectors. */

    /**Get principal axis eigenvector. */
    ROOT::Math::XYZVector getPrimaryVector();

    /**Get secondary axis eigenvector. */
    ROOT::Math::XYZVector getSecondaryVector();

    /**Get tertiary axis eigenvector. */
    ROOT::Math::XYZVector getTertiaryVector();

    /**Get number of hits used in a cluster */
    int getNHits() const
    {return m_nHits;}


    /**Set eigenvectors and eigenvalues. */
    void setEigen(TMatrixT<double> eigenList);

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
    std::array<double, 4> m_primaryVec;

    /**Principal axis eigenvector and eigenvalue. */
    std::array<double, 4> m_secondaryVec;

    /**Principal axis eigenvector and eigenvalue. */
    std::array<double, 4> m_tertiaryVec;

    /**  Needed to make the ROOT object storable */
    ClassDef(KLMClusterShape, 1)

  }; // end of public RelationsObject

}// end of namespace Belle2

