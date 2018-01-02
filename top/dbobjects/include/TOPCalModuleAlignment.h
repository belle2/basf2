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
    TOPCalModuleAlignment() {}

    /**
     * Sets the angle alpha on a single module
     * @param moduleID module ID (1-based)
     * @param alpha rotation angle around x
     * @param errAlpha error on alpha
     */
    void setAlpha(int moduleID, double alpha, double errAlpha)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
      m_x[module] = x;
      m_errX[module] = errX;
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
      m_y[module] = y;
      m_errY[module] = errY;
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
      m_z[module] = z;
      m_errZ[module] = errZ;
    }

    /**
     * Sets calibration status to true
     */
    void setCalibrationStatus(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
      m_status[module] = true;
    }

    /**
     * Sets calibration status to false
     */
    void unsetCalibrationStatus(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules);
        return ;
      }
      m_status[module] = false;
    }

    /**
     * Gets the angle alpha on a single module
     * @param moduleID module ID (1-based)
     * @return alpha rotation angle around x
     */
    double getAlpha(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_alpha[module];
    }

    /**
     * Gets the angle beta on a single module
     * @param moduleID module ID (1-based)
     * @return beta rotation angle around x
     */
    double getBeta(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_beta[module];
    }

    /**
     * Gets the angle gamma on a single module
     * @param moduleID module ID (1-based)
     * @return gamma rotation angle around x
     */
    double getGamma(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_gamma[module];
    }

    /**
     * Returns the shift x on a single module
     * @param moduleID module ID (1-based)
     * @return x shift along the x direction
     */
    double getX(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_x[module];
    }

    /**
     * Returns the shift y on a single module
     * @param moduleID module ID (1-based)
     * @return y shift along the y direction
     */
    double getY(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_y[module];
    }

    /**
     * Returns the shift z on a single module
     * @param moduleID module ID (1-based)
     * @return z shift along the z direction
     */
    double getZ(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_z[module];
    }


    /**
     * Returns the error on alpha on a single module
     * @param moduleID module ID (1-based)
     * @return errAlpha error on the alpha angle
     */
    double getAlphaErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errAlpha[module];
    }

    /**
     * Returns the error on beta on a single module
     * @param moduleID module ID (1-based)
     * @return errBeta error on the beta angle
     */
    double getBetaErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errBeta[module];
    }

    /**
     * Returns the error on gamma on a single module
     * @param moduleID module ID (1-based)
     * @return errGamma error on the gamma angle
     */
    double getGammaErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errGamma[module];
    }

    /**
     * Returns the error on x on a single module
     * @param moduleID module ID (1-based)
     * @return errX error on the x shift
     */
    double getXErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errX[module];
    }

    /**
     * Returns the error on y on a single module
     * @param moduleID module ID (1-based)
     * @return errY error on the y shift
     */
    double getYErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errY[module];
    }

    /**
     * Returns the error on z on a single module
     * @param moduleID module ID (1-based)
     * @return errZ error on the z shift
     */
    double getZErr(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning 0.");
        return 0. ;
      }
      return m_errZ[module];
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if calibrated
     */
    bool isCalibrated(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than " << c_numModules << ". Returning false");
        return false ;
      }
      return m_status[module];
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
         };

    float m_alpha[c_numModules] = {0};    /**< rotation angle around the x axis. 0 by default. */
    float m_beta[c_numModules] = {0}; /**< rotation angle around the y axis. 0 by default.  */
    float m_gamma[c_numModules] = {0}; /**< rotation angle around the z axis. 0 by default. */
    float m_x[c_numModules] = {0};    /**< displacement along the x axis. 0 by default. */
    float m_y[c_numModules] = {0}; /**< displacement along the y axis. 0 by default. */
    float m_z[c_numModules] = {0}; /**< displacement along the z axis. 0 by default. */

    float m_errAlpha[c_numModules] = {0};    /**< error on alpha. 0 by default. */
    float m_errBeta[c_numModules] = {0}; /**< error on beta. 0 by default. */
    float m_errGamma[c_numModules] = {0}; /**< error on gamma. 0 by default. */
    float m_errX[c_numModules] = {0};    /**< error on the x displacement. 0 by default. */
    float m_errY[c_numModules] = {0}; /**< error on the y displacement. 0 by default. */
    float m_errZ[c_numModules] = {0}; /**< error on the z displacement. 0 by default. */

    bool m_status[c_numModules] = {false}; /**< calibration status */

    ClassDef(TOPCalModuleAlignment, 2); /**< ClassDef */

  };

} // end namespace Belle2

