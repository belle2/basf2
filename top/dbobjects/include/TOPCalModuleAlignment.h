/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Alignment constants constants for all 16 modules.
   * The constants are three rotation angles around the x, y and z axes (alpha, beta, gamma),
   * and three shifts along the same axes  (x, y, z).
   * From muon events.
   */
  class TOPCalModuleAlignment: public TObject {
  public:

    /**
     * Default constructor
     * All the calibrations constants and the related errors are set to zero
     */
    TOPCalModuleAlignment()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        m_alpha[m] = 0;
        m_beta[m] = 0;
        m_gamma[m] = 0;
        m_x[m] = 0;
        m_y[m] = 0;
        m_z[m] = 0;
        m_errAlpha[m] = 0;
        m_errBeta[m] = 0;
        m_errGamma[m] = 0;
        m_errX[m] = 0;
        m_errY[m] = 0;
        m_errZ[m] = 0;
      }
    }

    /**
     * Sets the angle alpha on a single module
     * @param moduleID module ID (1-based)
     * @param alpha rotation angle around x
     * @param errAlpha error on alpha
     */
    void setAlpha(int moduleID, double alpha, double errAlpha)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_alpha[module] = alpha;
      m_errAlpha[module] = errAlpha;
    }

    /**
     * Sets the angle beta on a single module
     * @param moduleID module ID (1-based)
     * @param beta rotation angle around x
     * @param errBeta error on beta
     */
    void setBeta(int moduleID, double beta, double errBeta)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_beta[module] = beta;
      m_errBeta[module] = errBeta;
    }

    /**
     * Sets the angle gamma on a single module
     * @param moduleID module ID (1-based)
     * @param gamma rotation angle around x
     * @param errGamma error on gamma
     */
    void setGamma(int moduleID, double gamma, double errGamma)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_gamma[module] = gamma;
      m_errGamma[module] = errGamma;
    }

    /**
     * Sets the displacement x on a single module
     * @param moduleID module ID (1-based)
     * @param x displacement along the x axis
     * @param errX error on the displacement
     */
    void setX(int moduleID, double x, double errX)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_alpha[module] = x;
      m_errAlpha[module] = errX;
    }

    /**
     * Sets the displacement y on a single module
     * @param moduleID module ID (1-based)
     * @param y displacement along the y axis
     * @param errY error on the displacement
     */
    void setY(int moduleID, double y, double errY)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_alpha[module] = y;
      m_errAlpha[module] = errY;
    }

    /**
     * Sets the displacement z on a single module
     * @param moduleID module ID (1-based)
     * @param z displacement along the z axis
     * @param errZ error on the displacement
     */
    void setZ(int moduleID, double z, double errZ)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_alpha[module] = z;
      m_errAlpha[module] = errZ;
    }


    /**
     * Gets the angle alpha on a single module
     * @param moduleID module ID (1-based)
     * @return alpha rotation angle around x
     */
    void getAlpha(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_alpha[module];
    }

    /**
     * Gets the angle beta on a single module
     * @param moduleID module ID (1-based)
     * @return beta rotation angle around x
     */
    void getBeta(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_beta[module];
    }

    /**
     * Gets the angle gamma on a single module
     * @param moduleID module ID (1-based)
     * @return gamma rotation angle around x
     */
    void getGamma(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_gamma[module];
    }

    /**
     * Returns the shift x on a single module
     * @param moduleID module ID (1-based)
     * @return x shift along the x direction
     */
    void getX(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_x[module];
    }

    /**
     * Returns the shift y on a single module
     * @param moduleID module ID (1-based)
     * @return y shift along the y direction
     */
    void getY(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_y[module];
    }

    /**
     * Returns the shift z on a single module
     * @param moduleID module ID (1-based)
     * @return z shift along the z direction
     */
    void getZ(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_z[module];
    }


    /**
     * Returns the error on alpha on a single module
     * @param moduleID module ID (1-based)
     * @return errAlpha error on the alpha angle
     */
    void getAlphaErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errAlpha[module];
    }

    /**
     * Returns the error on beta on a single module
     * @param moduleID module ID (1-based)
     * @return errBeta error on the beta angle
     */
    void getBetaErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errBeta[module];
    }

    /**
     * Returns the error on gamma on a single module
     * @param moduleID module ID (1-based)
     * @return errGamma error on the gamma angle
     */
    void getGammaErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errGamma[module];
    }

    /**
     * Returns the error on x on a single module
     * @param moduleID module ID (1-based)
     * @return errX error on the x shift
     */
    void getXErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errX[module];
    }

    /**
     * Returns the error on y on a single module
     * @param moduleID module ID (1-based)
     * @return errY error on the y shift
     */
    void getYErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errY[module];
    }

    /**
     * Returns the error on z on a single module
     * @param moduleID module ID (1-based)
     * @return errZ error on the z shift
     */
    void getZErr(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      return m_errZ[module];
    }




  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
         };

    float m_alpha[c_numModules];    /**< rotation angle around the x axis */
    float m_beta[c_numModules]; /**< rotation angle around the y axis  */
    float m_gamma[c_numModules]; /**< rotation angle around the z axis */
    float m_x[c_numModules];    /**< displacement along the x axis */
    float m_y[c_numModules]; /**< displacement along the y axis */
    float m_z[c_numModules]; /**< displacement along the z axis */

    float m_errAlpha[c_numModules];    /**< error on alpha */
    float m_errBeta[c_numModules]; /**< error on beta */
    float m_errGamma[c_numModules]; /**< error on gamma */
    float m_errX[c_numModules];    /**< error on the x displacement */
    float m_errY[c_numModules]; /**< error on the y displacement */
    float m_errZ[c_numModules]; /**< error on the z displacement */


    ClassDef(TOPCalModuleAlignment, 1); /**< ClassDef */

  };

} // end namespace Belle2

