/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {

  /**
  * The Class for VXD Alignment payload
  */
  class VXDAlignmentPar: public TObject {
  public:
    /** Constructor */
    VXDAlignmentPar(double dU, double dV, double dW, double alpha, double beta, double gamma):
      m_dU(dU), m_dV(dV), m_dW(dW), m_alpha(alpha), m_beta(beta), m_gamma(gamma)
    {}
    /** Constructor */
    VXDAlignmentPar():
      m_dU(0.), m_dV(0.), m_dW(0.), m_alpha(0.), m_beta(0.), m_gamma(0.)
    {}
    /** get dU */
    double getDU() const { return m_dU; }
    /** get dV */
    double getDV() const { return m_dV; }
    /** get dW */
    double getDW() const { return m_dW; }
    /** get alpha */
    double getAlpha() const { return m_alpha; }
    /** get beta */
    double getBeta() const { return m_beta; }
    /** get gamma */
    double getGamma() const { return m_gamma; }

  private:
    //! Shift along local u axis
    double m_dU;
    //! Shift along local v axis
    double m_dV;
    //! Shift along local w axis
    double m_dW;
    //! Rotation around local u axis
    double m_alpha;
    //! Rotation around local v axis
    double m_beta;
    //! Rotation around local w axis
    double m_gamma;

    ClassDef(VXDAlignmentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

