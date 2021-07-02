/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>
#include <TVector3.h>
#include <TRotation.h>


namespace Belle2 {

  /**
   * Geometry parameters of ARICH photon detector plane
   */
  class ARICHGeoDetectorPlane: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoDetectorPlane()
    {};


    /**
     * Check if parameters are consistent
     * @return returns true if true
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Detector plane parameters") const override;

    /**
     * Set detector plane positioning within ARICH local volume
     * @param x x position
     * @param y y position
     * @param z z position
     * @param rx rotation around x-axis
     * @param ry rotation around y-axis
     * @param rz rotation around z-axis
     */
    void setPlacement(double x, double y, double z, double rx, double ry, double rz) {m_x = x; m_y = y; m_z = z; m_rx = rx; m_ry = ry; m_rz = rz;}

    /**
     * Set parameters of the support plate
     * @param inR tube inner radius
     * @param outR tube outer radius
     * @param thick tube length
     * @param material tube material
     */
    void addSupportPlate(double inR, double outR, double thick, const std::string& material)
    {
      m_innerR = inR;
      m_outerR = outR;
      m_thickness = thick;
      m_supportMaterial = material;
    };

    /**
     * Set size of module hole in support plate
     * @param hsize hole size
     */
    void setModuleHoleSize(double hsize) {m_moduleHoleSize = hsize;}

    /**
     * Set height of the aluminum walls between modules on the electronics side
     * of aluminum support plate
     * @param h wall height
     */
    void setSupportBackWallHeight(double h) {m_supportBackWallHeight = h;}

    /**
     * Set thickness of the aluminum walls between modules on the electronics side
     * of the aluminum support plate
     * @param h wall height
     */
    void setSupportBackWallThickness(double d) {m_supportBackWallThickness = d;}

    /**
     * Set Z position of the aluminum support plate (start Z)
     * @param zPosition Z position of support plate
     */
    void setSupportZPosition(double zPosition) { m_supportZPosition = zPosition; }

    /**
     * Set parameters of module slot rings (r of center of slots in ring, phi (angle) distance between module slots)
     * @param ringPar vector of module slot ring radiuses and phi (angle) distance between module slots in ring
     */
    void setRingPar(const std::vector<std::pair<double, double>>& ringPar);

    /**
     * Get center point
     * @return center point (TVector3)
     */
    TVector3 getPosition() const {return TVector3(m_x / s_unit, m_y / s_unit, m_z / s_unit);}

    /**
     * Get rotation matrix
     * @return rotation matrix (TRotation)
     */
    TRotation getRotation() const
    {
      TRotation rot;
      rot.RotateX(m_rx).RotateY(m_ry).RotateZ(m_rz);
      return rot;
    }

    /**
     * Get angle of rotation around X axis
     * @return rotation angle arounx X axis
     */
    double getRotationX() const {return m_rx;}

    /**
     * Get angle of rotation around Y axis
     * @return rotation angle arounx Y axis
     */
    double getRotationY() const {return m_ry;}

    /**
     * Get angle of rotation around Z axis
     * @return rotation angle arounx Z axis
     */
    double getRotationZ() const {return m_rz;}

    /**
     * Get support plate inner radius
     * @return support plate inner radius
     */
    double getSupportInnerR() const {return m_innerR / s_unit;};

    /**
     * Get support plate outer radius
     * @return support plate outer radius
     */
    double getSupportOuterR() const {return m_outerR / s_unit;};

    /**
     * Get support plate thickness
     * @return support plate thickness
     */
    double getSupportThickness() const {return m_thickness / s_unit;};

    /**
     * Get Z position of support plate (start point in Z)
     * @return support plate Z position (start point in Z)
     */
    double getSupportZPosition() const {return m_supportZPosition / s_unit;};

    /**
     * Get size of module hole in support plate
     * @return module hole size
     */
    double getModuleHoleSize() const {return m_moduleHoleSize / s_unit;};

    /**
     * Get height of the aluminum walls between modules on the electronics side
     * of aluminum support plate
     * @return support plate back wall height
     */
    double getSupportBackWallHeight() const {return m_supportBackWallHeight / s_unit;};

    /**
     * Get thickness of aluminum walls between modules on the electronics side
     * of aluminum support plate
     * @return support plate back wall thickness
     */
    double getSupportBackWallThickness() const {return m_supportBackWallThickness / s_unit;};

    /**
     * Get material of support plate
     * @return material of support plate
     */
    const std::string& getSupportMaterial() const {return m_supportMaterial;}

    /**
     * Get radius of i-th module slot ring (radius of center point)
     * @param iRing module slot ring number
     * @return radius of module slot ring
     */
    double getRingR(unsigned iRing) const { if (iRing == 0 || iRing > m_nRings) B2ERROR("ARICHGeoDetectorPlane: invalid module ring number!"); return m_ringPar[iRing - 1].first / s_unit;}

    /**
     * Get phi (angle) distance between module slots in i-th ring
     * @param iRing module slot ring number
     * @return phi (angle) distance between module slots
     */
    double getRingDPhi(unsigned iRing) const { if (iRing == 0 || iRing > m_nRings) B2ERROR("ARICHGeoDetectorPlane: invalid module ring number!"); return m_ringPar[iRing - 1].second;}

    /**
     * Get radial position of module with given module ID number
     * @param modID module ID number
     * @return radial position of module
     */
    double getSlotR(unsigned modID) const { if (modID == 0 || modID > m_nSlots) B2ERROR("ARICHGeoDetectorPlane: invalid module ID number!"); return m_slotPar[modID - 1].first / s_unit;};

    /**
     * Get phi (angle) position of module with given module ID number
     * @param modID module ID number
     * @return phi (angle) position of module
     */
    double getSlotPhi(unsigned modID) const { if (modID == 0 || modID > m_nSlots) B2ERROR("ARICHGeoDetectorPlane: invalid module ID number!"); return m_slotPar[modID - 1].second;};

    /**
     * Get total number of module slots
     * @return total number of module slots
     */
    unsigned getNSlots() const {return m_nSlots;}

    /**
     * Get number of module slot rings
     * @return number of module slot rings
     */
    unsigned getNRings() const {return m_nRings;}

    /**
     * Get ring number of slot with slot ID
     * @param slotID slot ID
     * @return ring number of slot
     */
    unsigned getSlotRing(unsigned slotID) const;

    unsigned pointSlotID(double x, double y) const;

    unsigned getSlotIDFromSRF(unsigned sector, unsigned ring, unsigned azimuth) const;

  private:

    std::vector<std::pair<double, double>> m_ringPar; /**< parameters of module rings (radius, dPhi) */

    std::vector<std::pair<double, double>> m_slotPar; /**< parameters of all module slots (radius, phi) */

    // detector plane position in local ARICH volume
    double m_x = 0; /**< x position in local ARICH volume */
    double m_y = 0; /**< y position in local ARICH volume */
    double m_z =
      0; /**< z position in local ARICH volume (position of center of detector plane tube, with Z length of: module height) */

    // rotations in local ARICH volume
    double m_rx = 0; /**< rotation around x axis */
    double m_ry = 0; /**< rotation around y axis */
    double m_rz = 0; /**< rotation around z axis */

    // support structure parameters
    std::string m_supportMaterial; /**< support structure material */
    double m_innerR = 0;        /**< inner radius of support plate */
    double m_outerR = 0;        /**< outer radius of support plate */
    double m_thickness = 0;     /**< thickness of support plate */
    double m_moduleHoleSize = 0;  /**< size of module hole in support plate */
    double m_supportBackWallHeight = 0; /**< height of aluminum walls on the back side (electronics side) of support plate */
    double m_supportBackWallThickness = 0;  /**<< thickness of aluminum walls on the back side (electronics side) of support plate */

    double m_supportZPosition =
      0; /**< Z position of support plate (position of center of support tube, with Z length of: support plate thickness + support back wall height) */

    unsigned m_nRings = 0;        /**< number of HAPD module rings */
    unsigned m_nSlots = 0;        /**< number of HAPD module slots */

    ClassDefOverride(ARICHGeoDetectorPlane, 1); /**< ClassDef */

  };

} // end namespace Belle2
