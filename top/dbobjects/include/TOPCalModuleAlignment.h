/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <TRotation.h>
#include <vector>

namespace Belle2 {

  /**
   * Alignment constants for all 16 modules.
   * The constants are three rotation angles around the x, y and z axes (alpha, beta, gamma),
   * and three shifts along the same axes  (x, y, z).
   * The meaning of constants is the same as in TOPGeoModuleDisplacement.
   * From muon events.
   */
  class TOPCalModuleAlignment: public TObject {
  public:

    /**
     * Calibration status of a constant
     */
    enum EStatus {
      c_Default = 0,    /**< uncalibrated default value */
      c_Calibrated = 1, /**< good calibrated value */
      c_Unusable = 2    /**< bad calibrated value */
    };

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
    void setAlpha(int moduleID, double alpha, double errAlpha);

    /**
     * Sets the angle beta on a single module
     * @param moduleID module ID (1-based)
     * @param beta rotation angle around y
     * @param errBeta error on beta
     */
    void setBeta(int moduleID, double beta, double errBeta);

    /**
     * Sets the angle gamma on a single module
     * @param moduleID module ID (1-based)
     * @param gamma rotation angle around z
     * @param errGamma error on gamma
     */
    void setGamma(int moduleID, double gamma, double errGamma);

    /**
     * Sets the displacement x on a single module
     * @param moduleID module ID (1-based)
     * @param x displacement along the x axis
     * @param errX error on the displacement
     */
    void setX(int moduleID, double x, double errX);

    /**
     * Sets the displacement y on a single module
     * @param moduleID module ID (1-based)
     * @param y displacement along the y axis
     * @param errY error on the displacement
     */
    void setY(int moduleID, double y, double errY);

    /**
     * Sets the displacement z on a single module
     * @param moduleID module ID (1-based)
     * @param z displacement along the z axis
     * @param errZ error on the displacement
     */
    void setZ(int moduleID, double z, double errZ);

    /**
     * Switches calibration status to calibrated
     * @param moduleID module ID (1-based)
     */
    void setCalibrated(int moduleID);

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     */
    void setUnusable(int moduleID);

    /**
     * Gets the angle alpha on a single module
     * @param moduleID module ID (1-based)
     * @return alpha rotation angle around x
     */
    double getAlpha(int moduleID) const;

    /**
     * Gets the angle beta on a single module
     * @param moduleID module ID (1-based)
     * @return beta rotation angle around y
     */
    double getBeta(int moduleID) const;

    /**
     * Gets the angle gamma on a single module
     * @param moduleID module ID (1-based)
     * @return gamma rotation angle around z
     */
    double getGamma(int moduleID) const;

    /**
     * Returns the shift x on a single module
     * @param moduleID module ID (1-based)
     * @return x shift along the x direction
     */
    double getX(int moduleID) const;

    /**
     * Returns the shift y on a single module
     * @param moduleID module ID (1-based)
     * @return y shift along the y direction
     */
    double getY(int moduleID) const;

    /**
     * Returns the shift z on a single module
     * @param moduleID module ID (1-based)
     * @return z shift along the z direction
     */
    double getZ(int moduleID) const;

    /**
     * Returns the error on alpha on a single module
     * @param moduleID module ID (1-based)
     * @return errAlpha error on the alpha angle
     */
    double getAlphaErr(int moduleID) const;

    /**
     * Returns the error on beta on a single module
     * @param moduleID module ID (1-based)
     * @return errBeta error on the beta angle
     */
    double getBetaErr(int moduleID) const;

    /**
     * Returns the error on gamma on a single module
     * @param moduleID module ID (1-based)
     * @return errGamma error on the gamma angle
     */
    double getGammaErr(int moduleID) const;

    /**
     * Returns the error on x on a single module
     * @param moduleID module ID (1-based)
     * @return errX error on the x shift
     */
    double getXErr(int moduleID) const;

    /**
     * Returns the error on y on a single module
     * @param moduleID module ID (1-based)
     * @return errY error on the y shift
     */
    double getYErr(int moduleID) const;

    /**
     * Returns the error on z on a single module
     * @param moduleID module ID (1-based)
     * @return errZ error on the z shift
     */
    double getZErr(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID) const;

    /**
     * Returns the rotation from local to nominal frame
     * Transformation is: rotation first then translation.
     * @return rotation
     */
    const TRotation& getRotation(int moduleID) const;

    /**
     * Returns the translation from local to nominal frame
     * Transformation is: rotation first then translation.
     * @return translation
     */
    const TVector3& getTranslation(int moduleID) const;

    /**
     * Returns true if all modules are calibrated
     */
    bool areAllCalibrated() const;

    /**
     * Returns true if calibration precision for all modules is within specified values
     * @param spatialPrecision precision for displacements
     * @param angularPrecision precision for rotations
     */
    bool areAllPrecise(double spatialPrecision, double angularPrecision) const;

  private:

    /**
     * Sets the transformation cache
     */
    void setTransformations() const;

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
         };

    float m_alpha[c_numModules] = {0}; /**< rotation angle around the x axis */
    float m_beta[c_numModules] = {0}; /**< rotation angle around the y axis */
    float m_gamma[c_numModules] = {0}; /**< rotation angle around the z axis */
    float m_x[c_numModules] = {0};    /**< displacement along the x axis */
    float m_y[c_numModules] = {0}; /**< displacement along the y axis */
    float m_z[c_numModules] = {0}; /**< displacement along the z axis */

    float m_errAlpha[c_numModules] = {0};    /**< error on alpha */
    float m_errBeta[c_numModules] = {0}; /**< error on beta */
    float m_errGamma[c_numModules] = {0}; /**< error on gamma */
    float m_errX[c_numModules] = {0};    /**< error on the x displacement */
    float m_errY[c_numModules] = {0}; /**< error on the y displacement */
    float m_errZ[c_numModules] = {0}; /**< error on the z displacement */

    EStatus m_status[c_numModules] = {c_Default}; /**< calibration status */

    /** cache for rotations (from local to nominal) */
    mutable std::vector<TRotation> m_rotations; //! do not write out
    /** cache for translations (from local to nominal) */
    mutable std::vector<TVector3> m_translations; //! do not write out

    ClassDef(TOPCalModuleAlignment, 3); /**< ClassDef */

  };

} // end namespace Belle2

