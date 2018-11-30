/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    BeamSpot():  m_position(),
      m_positionError(3),
      m_size(3)
    { }

    /**  Destructor */
    ~BeamSpot() {}


    /** equality operator */
    bool operator==(const BeamSpot& b) const
    {
      return
        b.m_position == m_position &&
        b.m_positionError == m_positionError &&
        b.m_size == m_size;
    }

    /** Set the size of the vertex position.
     * The upper triangle will be saved. */
    void setSize(const TMatrixDSym& size)
    {
      m_size = size;
    }

    /** Set the vertex position and its error matrix.
     * @param vertex position in BASF2 units: cm
     * @param error covariance matrix of the vertext position.
     */
    void setVertex(const TVector3& vertex, const TMatrixDSym& covariance)
    {
      m_position = vertex;
      m_positionError = covariance;
    }

    /** Get the interaction point position */
    const TVector3& getVertex()
    {
      return m_position;
    }

    /** Get the covariance matrix of the measured vertex position */
    const TMatrixDSym& getPositionError()
    {
      return m_positionError;
    }

    /** Get the size of the luminous regione modeled as a gaussian */
    const TMatrixDSym& getSize()
    {
      return m_size;
    }

    /** Get the covariance matrix of the vertex position
     * (for compatibility with BeamParameters)*/
    const TMatrixDSym& getCovVertex()
    {
      return getSize();
    }

    /** Return unique ID of BeamSpot in global Millepede calibration (1) */
    static unsigned short getGlobalUniqueID() { return 1; }

  private:

    /** Beam spot position
     * defined as the average position of the primary vertex */
    TVector3 m_position;

    /** Error of the measured beam spot position*/
    TMatrixDSym m_positionError;

    /** Size of the luminous region modeled with a
     * three dimensional gaussian. Covariance matrix */
    TMatrixDSym m_size;

    ClassDef(BeamSpot, 1); /**<  beam spot position and size **/
  };

} //Belle2 namespace
