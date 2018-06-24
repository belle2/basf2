/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>

//root
#include <TVector3.h>

namespace Belle2 {

  /**
   * Geometry parameters of Cooling System
   */
  class ARICHGeoCooling: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoCooling()
    {}

    /**
     * Set material name of cooling pipe
     * @param materialName material name of cooling pipe
     */
    void setCoolingPipeMaterialName(const std::string& materialName) {m_coolingPipeMaterialName = materialName;}

    /**
     * Set size of cooling system pipe : inner radius in mm
     * @param rmin size of cooling system pipe : inner radius in mm
     */
    void setRmin(double rmin) {m_Rmin = rmin;}

    /**
     * Set size of cooling system pipe : outer radius in mm
     * @param rmin size of cooling system pipe : outer radius in mm
     */
    void setRmax(double rmax) {m_Rmax = rmax;}

    /**
     * Set outer radius of cooling system assembly envelope
     * @param envelopeOuterRadius outer radius of cooling system assembly envelope
     */
    void setEnvelopeOuterRadius(double envelopeOuterRadius) {m_envelopeOuterRadius = envelopeOuterRadius;}

    /**
     * Set inner radius of cooling system envelope
     * @param envelopeInnerRadius inner radius of cooling system assembly envelope
     */
    void setEnvelopeInnerRadius(double envelopeInnerRadius) {m_envelopeInnerRadius = envelopeInnerRadius;}

    /**
     * Set thickness of cooling system assembly envelope
     * @param envelopeThickness thickness of cooling system assembly envelope
     */
    void setEnvelopeThickness(double envelopeThickness) {m_envelopeThickness = envelopeThickness;}

    /**
     * Set position of cooling system assembly envelope
     * @param x0 x position of cooling system assembly envelope
     * @param y0 y position of cooling system assembly envelope
     * @param z0 z position of cooling system assembly envelope
     */
    void setEnvelopeCenterPosition(double x0, double y0, double z0) { m_envelopeX0 = x0; m_envelopeY0 = y0; m_envelopeZ0 = z0; }

    /**
     * Set vector of cooling system object geometry ID
     * 1 - G4Tubs
     * 2 - G4Torus
     * @param coolingGeometryID vector of cooling system object geometry ID
     */
    void setCoolingGeometryID(const std::vector<double>& coolingGeometryID) { m_coolingGeometryID = coolingGeometryID; }

    /**
     * Set vector of lengs of the cooling system object with given geometry ID
     * @param coolingL vector of lengs of the cooling system object with given geometry ID
     */
    void setCoolingL(const std::vector<double>& coolingL) { m_coolingL = coolingL;}

    /**
     * Set vector of azimuthal angle of the cooling system object center in polar coordinate system in deg
     * @param coolingPosPhi vector of azimuthal angle of the cooling system object center in polar coordinate system in deg
     */
    void setCoolingPosPhi(const std::vector<double>& coolingPosPhi) { m_coolingPosPhi = coolingPosPhi; }

    /**
     * Set vector of radial distance (r, pho) of the cooling system object center in polar coordinate system in deg
     * @param coolingPosR vector of radial distance (r, pho) of the cooling system object center in polar coordinate system in deg
     */
    void setCoolingPosR(const std::vector<double>& coolingPosR) { m_coolingPosR = coolingPosR; }

    /**
     * Set vector of azimuthal angle of rotation aroud Z - axis of the cooling system object in polar coordinate system in deg
     * @param coolinRotationAngle vector of azimuthal angle of rotation aroud Z - axis of the cooling system object in polar coordinate system in deg
     */
    void setCoolinRotationAngle(const std::vector<double>& coolinRotationAngle) { m_coolinRotationAngle = coolinRotationAngle; }

    /**
     * Set material name of cooling test plates
     * @param materialName material name of cooling test plates
     */
    void setCoolingTestPlateMaterialName(const std::string& materialName) {m_coolingTestPlateMaterialName = materialName;}

    /**
     * Set material name of cold tube
     * @param materialName material name of cold tube
     */
    void setColdTubeMaterialName(const std::string& materialName) {m_coldTubeMaterialName = materialName;}

    /**
     * Set dimentions of cooling test plates in x, y, z directions
     * @param lx length of cooling test plate along x axis
     * @param ly length of cooling test plate along y axis
     * @param lz length of cooling test plate along z axis
     */
    void setCoolingTestPlateslengths(double lx, double ly, double lz) { m_coolingTestPlateslengthX = lx; m_coolingTestPlateslengthY = ly; m_coolingTestPlateslengthZ = lz; }

    /**
     * Set radius of cold tubes in the cooling test plane
     * @param coldTubeR radius of cold tubes in the cooling test plane
     */
    void setColdTubeR(double coldTubeR) {m_coldTubeR = coldTubeR;}

    /**
     * Set outer radius of subtracted tubes for cold tube
     * @param coldTubeSubtractedR outer radius of subtracted tubes for cold tube
     */
    void setColdTubeSubtractedR(double coldTubeSubtractedR) {m_coldTubeSubtractedR = coldTubeSubtractedR;}

    /**
     * Set cold tube wall thickness
     * @param coldTubeWallThickness cold tube wall thickness
     */
    void setColdTubeWallThickness(double coldTubeWallThickness) {m_coldTubeWallThickness = coldTubeWallThickness;}

    /**
     * Set depth of the cold tube in the cooling test plate
     * @param depthColdTubeInPlate depth of the cold tube in the cooling test plate
     */
    void setDepthColdTubeInPlate(double depthColdTubeInPlate) {m_depthColdTubeInPlate = depthColdTubeInPlate;}

    /**
     * Set distance from center of the cold tube to edge of cooling plate
     * @param coldTubeSpacing distance from center of the cold tube to edge of cooling plate
     */
    void setColdTubeSpacing(double coldTubeSpacing) {m_coldTubeSpacing = coldTubeSpacing;}

    /**
     * Set number of cold tubes in one plate
     * @param coldTubeNumber number of cold tubes in one plate
     */
    void setColdTubeNumber(int coldTubeNumber) {m_coldTubeNumber = coldTubeNumber;}

    /**
     * Set vector of rho of the cooling test plates center in polar coordinate system in mm
     * @param coolingTestPlatePosR rho of the cooling test plates center in polar coordinate system in mm
     */
    void setCoolingTestPlatePosR(const std::vector<double>& coolingTestPlatePosR) { m_coolingTestPlatePosR = coolingTestPlatePosR; }

    /**
     * Set vector of azimuthal angle of the cooling test plates center in polar coordinate system in deg
     * @param coolingTestPlatePosPhi azimuthal angle of the cooling test plates center in polar coordinate system in deg
     */
    void setCoolingTestPlatePosPhi(const std::vector<double>& coolingTestPlatePosPhi) { m_coolingTestPlatePosPhi = coolingTestPlatePosPhi; }

    /**
     * Set vector of Z placement of the cooling test plates center within ARICH mother volume in mm
     * @return vector of Z placement of the cooling test plates center within ARICH mother volume in mm
     */
    void setCoolingTestPlatePosZ0(const std::vector<double>& coolingTestPlatePosZ0) {  m_coolingTestPlatePosZ0 = coolingTestPlatePosZ0; }

    /**
     * Returns material name of cooling pipe
     * @return material name of cooling pipe
     */
    const std::string& getCoolingPipeMaterialName() const { return m_coolingPipeMaterialName; }

    /**
     * Returns Size of cooling system pipe : inner radius in mm
     * @return Size of cooling system pipe : inner radius in mm
     */
    double getRmin() const { return m_Rmin; }

    /**
     * Returns Size of cooling system pipe : outer radius in mm
     * @return Size of cooling system pipe : outer radius in mm
     */
    double getRmax() const { return m_Rmax; }

    /**
     * Returns Outer radius of cooling system assembly envelope
     * @return Outer radius of cooling system assembly envelope
     */
    double getEnvelopeOuterRadius() const { return m_envelopeOuterRadius; }

    /**
     * Returns Inner radius of cooling system assembly envelope
     * @return Inner radius of cooling system assembly envelope
     */
    double getEnvelopeInnerRadius() const { return m_envelopeInnerRadius; }

    /**
     * Returns Thickness of cooling system assembly envelope
     * @return Thickness of cooling system assembly envelope
     */
    double getEnvelopeThickness() const { return m_envelopeThickness; }

    /**
     * Returns position vector (TVector3) of cooling system assembly envelope
     * @return position vector (TVector3) of cooling system assembly envelope
     */
    TVector3 getEnvelopeCenterPosition() const { return TVector3(m_envelopeX0, m_envelopeY0, m_envelopeZ0); }

    /**
     * Returns vector of cooling system object geometry ID
     * @return vector of cooling system object geometry ID
     */
    const std::vector<double>& getCoolingGeometryID() const { return m_coolingGeometryID; }

    /**
     * Returns vector of lengs of the cooling system object with given geometry ID
     * @return vector of lengs of the cooling system object with given geometry ID
     */
    const std::vector<double>& getCoolingL() const { return m_coolingL; }

    /**
     * Returns vector of azimuthal angle of the cooling system object center in polar coordinate system in deg
     * @return vector of azimuthal angle of the cooling system object center in polar coordinate system in deg
     */
    const std::vector<double>& getCoolingPosPhi() const { return m_coolingPosPhi; }

    /**
     * Returns vector of radial distance (r, pho) of the cooling system object center in polar coordinate system in deg
     * @return vector of radial distance (r, pho) of the cooling system object center in polar coordinate system in deg
     */
    const std::vector<double>& getCoolingPosR() const { return m_coolingPosR; }

    /**
     * Returns vector of azimuthal angle of rotation aroud Z - axis of the cooling system object in polar coordinate system in deg
     * @return vector of Azimuthal angle of rotation aroud Z - axis of the cooling system object in polar coordinate system in deg
     */
    const std::vector<double>& getCoolinRotationAngle() const { return m_coolinRotationAngle; }

    /**
     * Returns material name of cooling test plates
     * @return Material name of cooling test plates
     */
    const std::string& getCoolingTestPlateMaterialName() const { return m_coolingTestPlateMaterialName; }

    /**
     * Returns material name of cold tube
     * @return Material name of cold tube
     */
    const std::string& getColdTubeMaterialName() const { return m_coldTubeMaterialName; }

    /**
     * Returns sizes vector (TVector3) of cooling test plates
     * @return sizes vector (TVector3) of cooling test plates
     */
    TVector3 getCoolingTestPlateslengths() const { return TVector3(m_coolingTestPlateslengthX, m_coolingTestPlateslengthY, m_coolingTestPlateslengthZ); }

    /**
     * Returns radius of cold tubes
     * @return radius of cold tubes
     */
    double getColdTubeR() const { return m_coldTubeR; }

    /**
     * Returns outer radius of subtracted tubes for cold tube
     * @return outer radius of subtracted tubes for cold tube
     */
    double getColdTubeSubtractedR() const { return m_coldTubeSubtractedR; }

    /**
     * Returns cold tube wall thickness
     * @return cold tube wall thickness
     */
    double getColdTubeWallThickness() const { return m_coldTubeWallThickness; }

    /**
     * Returns depth of the cold tube in the cooling plate
     * @return depth of the cold tube in the cooling plate
     */
    double getDepthColdTubeInPlate() const { return m_depthColdTubeInPlate; }

    /**
     * Returns distance from center of the cold tube to edge of cooling plate
     * @return distance from center of the cold tube to edge of cooling plate
     */
    double getColdTubeSpacing() const { return m_coldTubeSpacing; }

    /**
     * Returns number of cold tubes in one plate
     * @return number of cold tubes in one plate
     */
    int getColdTubeNumber() const { return m_coldTubeNumber; }

    /**
     * Returns vector of Rho of the cooling test plates center in polar coordinate system in mm
     * @return vector of Rho of the cooling test plates center in polar coordinate system in mm
     */
    const std::vector<double>& getCoolingTestPlatePosR() const { return m_coolingTestPlatePosR; }

    /**
     * Returns vector of Azimuthal angle of the cooling test plates center in polar coordinate system in deg
     * @return vector of Azimuthal angle of the cooling test plates center in polar coordinate system in deg
     */
    const std::vector<double>& getCoolingTestPlatePosPhi() const { return m_coolingTestPlatePosPhi; }

    /**
     * Returns vector of Z placement of the cooling test plates center within ARICH mother volume in mm
     * @return vector of Z placement of the cooling test plates center within ARICH mother volume in mm
     */
    const std::vector<double>& getCoolingTestPlatePosZ0() const { return m_coolingTestPlatePosZ0; }

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Cooling system geometry parameters") const;

    /**
     * Check data consistency of the cooling system positions
     * In case of failure print the BASF2 ERROR message using B2ASSERT
     */
    void checkCoolingSystemDataConsistency() const;

  private:

    std::string m_coolingPipeMaterialName;      /**< Material of cooling pipe */
    double m_Rmin;                              /**< Size of cooling system pipe : inner radius in mm */
    double m_Rmax;                              /**< Size of cooling system pipe : outer radius in mm */
    double m_envelopeOuterRadius = 0.0;         /**< Outer radius of cooling system assembly envelope */
    double m_envelopeInnerRadius = 0.0;         /**< Inner radius of cooling system assembly envelope */
    double m_envelopeThickness = 0.0;           /**< Thickness of  cooling system assembly envelope */
    double m_envelopeX0 = 0.0;                  /**< X0 cooling system assembly envelope */
    double m_envelopeY0 = 0.0;                  /**< Y0 cooling system assembly envelope */
    double m_envelopeZ0 = 0.0;                  /**< Z0 cooling system assembly envelope */
    std::vector<double> m_coolingGeometryID;    /**< Cooling system object geometry ID */
    std::vector<double> m_coolingL;             /**< Lengs of the cooling system object with given geometry ID */
    std::vector<double>
    m_coolingPosPhi;        /**< Azimuthal angle of the cooling system object center in polar coordinate system in deg */
    std::vector<double>
    m_coolingPosR;          /**< Radial distance (r, pho) of the cooling system object center in polar coordinate system in deg */
    std::vector<double>
    m_coolinRotationAngle;  /**< Azimuthal angle of rotation aroud Z - axis of the cooling system object in polar coordinate system in deg */
    //Please note that ARICH detector envelope does not have enough
    //space for 8 mm thick cooling test plate (5.5 mm is only available).
    //For this reason the material assigned to the volume (aluminum)
    //will have a bigger dencity for effective descrption of the material.
    std::string m_coolingTestPlateMaterialName; /**< Material name of cooling test plates */
    std::string m_coldTubeMaterialName;         /**< Material name of cold tube */
    double m_coolingTestPlateslengthX = 0.0;    /**< Size of cooling test plates (x) */
    double m_coolingTestPlateslengthY = 0.0;    /**< Size of cooling test plates (y) */
    double m_coolingTestPlateslengthZ = 0.0;    /**< Size of cooling test plates (z) */
    double m_coldTubeR                = 0.0;    /**< Outer radius of cold tubes */
    double m_coldTubeSubtractedR      = 0.0;    /**< Outer radius of subtracted tubes for cold tube */
    double m_coldTubeWallThickness    = 0.0;    /**< Cold tube wall thickness */
    double m_depthColdTubeInPlate     = 0.0;    /**< Depth of the cold tube in the cooling plate */
    double m_coldTubeSpacing = 0.0;             /**< Distance from center of the cold tube to edge of cooling plate */
    int m_coldTubeNumber     = 0;               /**< number of cold tubes in one plate */
    std::vector<double> m_coolingTestPlatePosR;   /**< Rho of the cooling test plates center in polar coordinate system in mm */
    std::vector<double>
    m_coolingTestPlatePosPhi; /**< Azimuthal angle of the cooling test plates center in polar coordinate system in deg */
    std::vector<double> m_coolingTestPlatePosZ0;  /**< Z placement of the cooling test plates center within ARICH mother volume in mm */
    ClassDef(ARICHGeoCooling, 1);               /**< ClassDef */

  };

} // end namespace Belle2
