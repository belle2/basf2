/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 **************************************************************************/

/*This Class desribes the displaced-geometry of BKLM modules.             *
 * The displaced-geometry is supposed to be inclued in geometry.          *
 * Major displaced geometry should be put here and included in geometry   *
 * constructor.                                                           *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /// BKLM displaced geometry
  class BKLMDisplacement: public TObject {

  public:
    //! Constructor
    BKLMDisplacement() : m_elementID(65535),
      m_uShift(0), m_vShift(0), m_wShift(0),
      m_alphaRotation(0), m_betaRotation(0), m_gammaRotation(0) {}

    //! Constructor
    BKLMDisplacement(const unsigned short elementID, float dU, float dV, float dW, float dAlpha, float dBeta, float dGamma) :
      m_elementID(elementID),
      m_uShift(dU), m_vShift(dV), m_wShift(dW),
      m_alphaRotation(dAlpha), m_betaRotation(dBeta), m_gammaRotation(dGamma)
    {
    }

    //! Destructor
    ~BKLMDisplacement() {}

    //! get the identifier id
    float getElementID() const {return m_elementID;}

    //! get the movement along u
    float getUShift() const {return m_uShift;}

    //! get the movement along v
    float getVShift() const {return m_vShift;}

    //! get the movement along w
    float getWShift() const {return m_wShift;}

    //! get the rotation around u
    float getAlphaRotation() const {return m_alphaRotation;}

    //! get the rotation around v
    float getBetaRotation() const {return m_betaRotation;}

    //! get the rotation around w
    float getGammaRotation() const {return m_gammaRotation;}


  private:

    //! unique identifier for a BKLM module/supperlayer
    unsigned short m_elementID;

    /* same with six alignment parameters
     *let's define them in local system, which should be more convenient for geometry constructor
     *movement along u, v, w and rotation around u v w
     */

    //! movement along u, u direction is along z-measuring strips
    float m_uShift;

    //! movement along v, v direction is along phi-measuring strips
    float m_vShift;

    //! movement along w, w direction is normal the a module plane
    float m_wShift;

    //! rotation around u, hardly be non-zero
    float m_alphaRotation;

    //! rotation around v, hardly be non-zero
    float m_betaRotation;

    //! rotation around w, hardly be non-zero
    float m_gammaRotation;

    ClassDef(BKLMDisplacement, 1); /**< BKLM displaced-geometry*/
  };
} // end namespace Belle2
