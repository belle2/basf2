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
     * Luminous region size
     * x =  40 microns,
     * y =   2 microns,
     * z = 200 microns
     */
    BeamSpot(): m_position{0.}, m_positionCached(),
      m_positionError{0}, m_positionErrorCached(3),
      m_size{0}, m_sizeCached(3)
    {
      m_size[0] = .0040; // X size 40 microns
      m_size[3] = .0002; // Y size 2 microns
      m_size[3] = .0200; // Z size 200 microns
      m_initialized = false;
    }

    /**  Destructor */
    ~BeamSpot() {}

    /** copy constructor */
    BeamSpot(const BeamSpot& b): TObject(b)
    {
      *this = b;
    }

    /** assignment operator */
    BeamSpot& operator=(const BeamSpot& b)
    {
      std::copy_n(b.m_position, 3, m_position);
      std::copy_n(b.m_positionError, 6, m_positionError);
      std::copy_n(b.m_size, 6, m_size);
      m_initialized = false;
      return *this;
    }

    /** equality operator */
    bool operator==(const BeamSpot& b) const
    {
      // since we only save the covariance matrices with float precision we
      // need to also do the comparison with float precision.
      auto floatcmp = [](double dbl_a, double dbl_b) {
        return (float)dbl_a == (float)dbl_b;
      };

      return
        std::equal(m_position, m_position + 3,
                   b.m_position, floatcmp) &&
        std::equal(m_positionError, m_positionError + 6,
                   b.m_positionError, floatcmp) &&
        std::equal(m_size, m_size + 6,
                   b.m_size, floatcmp);
    }

    /** Set the size of the vertex position.
     * The upper triangle will be saved. */
    void setSize(const TMatrixDSym& size)
    {
      setCovMatrix(m_size, size);
      m_initialized = false;
    }

    /** Set the vertex position and error matrix.
     * @param vertex position in BASF2 units: cm
     * @param error entries of the covariance matrix.
     */
    void setVertex(const TVector3& vertex, const TMatrixDSym& covariance);

    /** Get the interaction point position */
    const TVector3& getVertex()
    {
      if (! m_initialized)
        initialize();
      return m_positionCached;
    }

    /** Get the covariance matrix of the measured vertex position */
    const TMatrixDSym& getPositionError()
    {
      if (! m_initialized)
        initialize();
      return m_positionErrorCached;
    }

    /** Get the size of the luminous regione modeled as a gaussian */
    const TMatrixDSym& getSize()
    {
      if (! m_initialized)
        initialize();
      return m_sizeCached;
    }

    /** Get the covariance matrix of the vertex position
     * (for compatibility with BeamParameters)*/
    const TMatrixDSym& getCovVertex()
    {
      if (! m_initialized)
        initialize();
      return m_sizeCached;
    }

    /** Return unique ID of BeamSpot in global Millepede calibration (1) */
    static unsigned short getGlobalUniqueID() { return 1; }

  private:

    /** Set covariance matrix from ROOT Matrix object */
    static void setCovMatrix(Double32_t* member, const TMatrixDSym& cov);

    /** Obtain covariance matrix from a given float array */
    static TMatrixDSym getCovMatrix(const Double32_t* member);

    /** initialize the cached copy of the root objects */
    void initialize(void);

    bool m_initialized; //!
    /** Beam spot position
     * defined as the average position of the primary vertex */
    Double32_t m_position[3];

    /** Cached Beam spot position
     * defined as the average position of the primary vertex */
    TVector3 m_positionCached; //!

    /** Error of the measured beam spot position*/
    Double32_t m_positionError[6];

    /** Error of the measured beam spot position cached*/
    TMatrixDSym m_positionErrorCached; //!

    /** Size of the luminous region modeled with a
     * three dimensional gaussian. Covariance matrix */
    Double32_t m_size[6];

    /** Size of the luminous region modeled with a
     * three dimensional gaussian. Covariance matrix cached*/
    TMatrixDSym m_sizeCached; //!

    ClassDef(BeamSpot, 1); /**<  beam spot position and size **/
  };

} //Belle2 namespace
