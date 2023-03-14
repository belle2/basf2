/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TMatrixDSym.h>
#include <TVector3.h>

namespace Belle2 {
  /** This class contains the beam spot position and size modeled as a
   * gaussian distribution in space.
   */
  class BeamSpot: public TObject {
  public:

    /** default constructor
     * IP position at the nominal origin
     * error on the position 0 in all directions
     * Luminous region size set to 0 in all directions
     */
    BeamSpot():  m_IPPosition(),
      m_IPPositionCovMatrix(3),
      m_sizeCovMatrix(3)
    { }

    /**  Destructor */
    ~BeamSpot() {}


    /** equality operator */
    bool operator==(const BeamSpot& b) const
    {
      return
        b.m_IPPosition == m_IPPosition &&
        b.m_IPPositionCovMatrix == m_IPPositionCovMatrix &&
        b.m_sizeCovMatrix == m_sizeCovMatrix;
    }

    /** Set the covariance matrix of the size of the IP position.
     * The upper triangle will be saved. */
    void setSizeCovMatrix(const TMatrixDSym& size)
    {
      m_sizeCovMatrix = size;
    }

    /** Set the IP position and its error matrix.
     * @param ipPosition position in basf2 units: cm
     * @param covariance covariance matrix of the vertex position.
     */
    void setIP(const TVector3& ipPosition, const TMatrixDSym& covariance)
    {
      m_IPPosition = ipPosition;
      m_IPPositionCovMatrix = covariance;
    }

    /** Get the IP position */
    const TVector3& getIPPosition() const
    {
      return m_IPPosition;
    }

    /** Get the covariance matrix of the measured IP position */
    const TMatrixDSym& getIPPositionCovMatrix() const
    {
      return m_IPPositionCovMatrix;
    }

    /** Get the covariance matrix of the size of the IP position modeled as a gaussian */
    const TMatrixDSym& getSizeCovMatrix() const
    {
      return m_sizeCovMatrix;
    }

    /** Get the total covariance matrix of theIP position
     * (for compatibility with BeamParameters)*/
    TMatrixDSym getCovVertex() const
    {
      return  getSizeCovMatrix() + getIPPositionCovMatrix();
    }

    /** Return unique ID of BeamSpot in global Millepede calibration (1) */
    static unsigned short getGlobalUniqueID() { return 1; }

  private:

    /** Beam spot position
     * defined as the average position of the primary vertex */
    TVector3 m_IPPosition;

    /** CovMatrix of the measured beam spot position*/
    TMatrixDSym m_IPPositionCovMatrix;

    /** Size of the luminous region modeled with a
     * three dimensional gaussian. Covariance matrix */
    TMatrixDSym m_sizeCovMatrix;

    ClassDef(BeamSpot, 2); /**<  beam spot position and size **/
  };

} //Belle2 namespace
