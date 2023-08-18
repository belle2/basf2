/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>

//root
#include <TVector3.h>

namespace Belle2 {

  /**
   * Geometry parameters of cooling system.
   */
  class ARICHGeoCooling: public ARICHGeoBase {

  public:

    /**
     * Default constructor.
     */
    ARICHGeoCooling()
    {}

    /**
     * Set material name of cooling pipe.
     * @param[in] materialName Material name of cooling pipe.
     */
    void setCoolingPipeMaterialName(const std::string& materialName) {m_coolingPipeMaterialName = materialName;}

    /**
     * Set size of cooling system pipe : inner radius in mm.
     * @param[in] rmin Size of cooling system pipe : inner radius in mm.
     */
    void setRmin(double rmin) {m_Rmin = rmin;}

    /**
     * Set size of cooling system pipe : outer radius in mm.
     * @param[in] rmax Size of cooling system pipe : outer radius in mm.
     */
    void setRmax(double rmax) {m_Rmax = rmax;}

    /**
     * Set outer radius of cooling system assembly envelope.
     *
     * @param[in] envelopeOuterRadius
     * Outer radius of cooling system assembly envelope.
     */
    void setEnvelopeOuterRadius(double envelopeOuterRadius) {m_envelopeOuterRadius = envelopeOuterRadius;}

    /**
     * Set inner radius of cooling system envelope.
     *
     * @param[in] envelopeInnerRadius
     * Inner radius of cooling system assembly envelope.
     */
    void setEnvelopeInnerRadius(double envelopeInnerRadius) {m_envelopeInnerRadius = envelopeInnerRadius;}

    /**
     * Set thickness of cooling system assembly envelope.
     *
     * @param[in] envelopeThickness
     * Thickness of cooling system assembly envelope.
     */
    void setEnvelopeThickness(double envelopeThickness) {m_envelopeThickness = envelopeThickness;}

    /**
     * Set position of cooling system assembly envelope.
     * @param[in] x0 X position of cooling system assembly envelope.
     * @param[in] y0 Y position of cooling system assembly envelope.
     * @param[in] z0 Z position of cooling system assembly envelope.
     */
    void setEnvelopeCenterPosition(double x0, double y0, double z0) { m_envelopeX0 = x0; m_envelopeY0 = y0; m_envelopeZ0 = z0; }

    /**
     * Set vector of cooling system object geometry ID
     * 1 - G4Tubs
     * 2 - G4Torus
     * @param[in] coolingGeometryID Vector of cooling system object geometry ID.
     */
    void setCoolingGeometryID(const std::vector<double>& coolingGeometryID) { m_coolingGeometryID = coolingGeometryID; }

    /**
     * Set vector of lengs of the cooling system object with given geometry ID.
     *
     * @param[in] coolingL
     * Vector of lengs of the cooling system object with given geometry ID.
     */
    void setCoolingL(const std::vector<double>& coolingL) { m_coolingL = coolingL;}

    /**
     * Set vector of azimuthal angle of the cooling system object center
     * in polar coordinate system in deg.
     *
     * @param[in] coolingPosPhi
     * Vector of azimuthal angle of the cooling system object center
     * in polar coordinate system in deg.
     */
    void setCoolingPosPhi(const std::vector<double>& coolingPosPhi) { m_coolingPosPhi = coolingPosPhi; }

    /**
     * Set vector of radial distance (r, pho) of the cooling system object
     * center in polar coordinate system in deg.
     *
     * @param[in] coolingPosR
     * Vector of radial distance (r, pho) of the cooling system object center
     * in polar coordinate system in deg.
     */
    void setCoolingPosR(const std::vector<double>& coolingPosR) { m_coolingPosR = coolingPosR; }

    /**
     * Set vector of azimuthal angle of rotation around Z - axis of
     * the cooling system object in polar coordinate system in deg.
     *
     * @param[in] coolinRotationAngle
     * Vector of azimuthal angle of rotation around Z - axis of
     * the cooling system object in polar coordinate system in deg.
     */
    void setCoolinRotationAngle(const std::vector<double>& coolinRotationAngle) { m_coolinRotationAngle = coolinRotationAngle; }

    /**
     * Set material name of cooling test plates.
     * @param[in] materialName Material name of cooling test plates.
     */
    void setCoolingTestPlateMaterialName(const std::string& materialName) {m_coolingTestPlateMaterialName = materialName;}

    /**
     * Set material name of cold tube.
     * @param[in] materialName Material name of cold tube.
     */
    void setColdTubeMaterialName(const std::string& materialName) {m_coldTubeMaterialName = materialName;}

    /**
     * Set dimentions of cooling test plates in x, y, z directions.
     * @param[in] lx Length of cooling test plate along x axis.
     * @param[in] ly Length of cooling test plate along y axis.
     * @param[in] lz Length of cooling test plate along z axis.
     */
    void setCoolingTestPlateslengths(double lx, double ly, double lz) { m_coolingTestPlateslengthX = lx; m_coolingTestPlateslengthY = ly; m_coolingTestPlateslengthZ = lz; }

    /**
     * Set radius of cold tubes in the cooling test plane.
     * @param[in] coldTubeR Radius of cold tubes in the cooling test plane.
     */
    void setColdTubeR(double coldTubeR) {m_coldTubeR = coldTubeR;}

    /**
     * Set outer radius of subtracted tubes for cold tube.
     *
     * @param[in] coldTubeSubtractedR
     * Outer radius of subtracted tubes for cold tube.
     */
    void setColdTubeSubtractedR(double coldTubeSubtractedR) {m_coldTubeSubtractedR = coldTubeSubtractedR;}

    /**
     * Set cold tube wall thickness.
     * @param[in] coldTubeWallThickness Cold tube wall thickness.
     */
    void setColdTubeWallThickness(double coldTubeWallThickness) {m_coldTubeWallThickness = coldTubeWallThickness;}

    /**
     * Set depth of the cold tube in the cooling test plate.
     *
     * @param[in] depthColdTubeInPlate
     * Depth of the cold tube in the cooling test plate.
     */
    void setDepthColdTubeInPlate(double depthColdTubeInPlate) {m_depthColdTubeInPlate = depthColdTubeInPlate;}

    /**
     * Set distance from center of the cold tube to edge of cooling plate.
     *
     * @param[in] coldTubeSpacing
     * Distance from center of the cold tube to edge of cooling plate.
     */
    void setColdTubeSpacing(double coldTubeSpacing) {m_coldTubeSpacing = coldTubeSpacing;}

    /**
     * Set number of cold tubes in one plate
     * @param[in] coldTubeNumber Number of cold tubes in one plate.
     */
    void setColdTubeNumber(int coldTubeNumber) {m_coldTubeNumber = coldTubeNumber;}

    /**
     * Set vector of rho of the cooling test plates center
     * in polar coordinate system in mm.
     *
     * @param[in] coolingTestPlatePosR
     * Rho of the cooling test plates center in polar coordinate system in mm.
     */
    void setCoolingTestPlatePosR(const std::vector<double>& coolingTestPlatePosR) { m_coolingTestPlatePosR = coolingTestPlatePosR; }

    /**
     * Set vector of azimuthal angle of the cooling test plates center
     * in polar coordinate system in deg.
     *
     * @param[in] coolingTestPlatePosPhi
     * Azimuthal angle of the cooling test plates center in polar coordinate
     * system in deg.
     */
    void setCoolingTestPlatePosPhi(const std::vector<double>& coolingTestPlatePosPhi) { m_coolingTestPlatePosPhi = coolingTestPlatePosPhi; }

    /**
     * Set vector of Z placement of the cooling test plates center
     * within ARICH mother volume in mm.
     *
     * @param[in] coolingTestPlatePosZ0
     * Vector of Z placement of the cooling test plates center
     * within ARICH mother volume in mm.
     */
    void setCoolingTestPlatePosZ0(const std::vector<double>& coolingTestPlatePosZ0) {  m_coolingTestPlatePosZ0 = coolingTestPlatePosZ0; }

    /**
     * Get material name of cooling pipe.
     * @return Material name of cooling pipe.
     */
    const std::string& getCoolingPipeMaterialName() const { return m_coolingPipeMaterialName; }

    /**
     * Get size of cooling system pipe : inner radius in mm.
     * @return Size of cooling system pipe : inner radius in mm
     */
    double getRmin() const { return m_Rmin; }

    /**
     * Get size of cooling system pipe : outer radius in mm.
     * @return Size of cooling system pipe : outer radius in mm.
     */
    double getRmax() const { return m_Rmax; }

    /**
     * Get outer radius of cooling system assembly envelope.
     * @return Outer radius of cooling system assembly envelope.
     */
    double getEnvelopeOuterRadius() const { return m_envelopeOuterRadius; }

    /**
     * Get inner radius of cooling system assembly envelope.
     * @return Inner radius of cooling system assembly envelope.
     */
    double getEnvelopeInnerRadius() const { return m_envelopeInnerRadius; }

    /**
     * Get thickness of cooling system assembly envelope.
     * @return Thickness of cooling system assembly envelope.
     */
    double getEnvelopeThickness() const { return m_envelopeThickness; }

    /**
     * Get position vector (TVector3) of cooling system assembly envelope.
     * @return Position vector (TVector3) of cooling system assembly envelope.
     */
    TVector3 getEnvelopeCenterPosition() const { return TVector3(m_envelopeX0, m_envelopeY0, m_envelopeZ0); }

    /**
     * Get vector of cooling system object geometry ID.
     * @return Vector of cooling system object geometry ID.
     */
    const std::vector<double>& getCoolingGeometryID() const { return m_coolingGeometryID; }

    /**
     * Get vector of lengs of the cooling system object with given geometry ID.
     * @return Vector of lengs of the cooling system object with given
     * geometry ID.
     */
    const std::vector<double>& getCoolingL() const { return m_coolingL; }

    /**
     * Get vector of azimuthal angle of the cooling system object center
     * in polar coordinate system in deg.
     * @return Vector of azimuthal angle of the cooling system object center
     * in polar coordinate system in deg.
     */
    const std::vector<double>& getCoolingPosPhi() const { return m_coolingPosPhi; }

    /**
     * Get vector of radial distance (r, pho) of the cooling system object
     * center in polar coordinate system in deg.
     * @return Vector of radial distance (r, pho) of the cooling system object
     * center in polar coordinate system in deg.
     */
    const std::vector<double>& getCoolingPosR() const { return m_coolingPosR; }

    /**
     * Get vector of azimuthal angle of rotation aroud Z - axis of the cooling
     * system object in polar coordinate system in deg.
     * @return Vector of Azimuthal angle of rotation aroud Z - axis of
     * the cooling system object in polar coordinate system in deg.
     */
    const std::vector<double>& getCoolinRotationAngle() const { return m_coolinRotationAngle; }

    /**
     * Get material name of cooling test plates.
     * @return Material name of cooling test plates.
     */
    const std::string& getCoolingTestPlateMaterialName() const { return m_coolingTestPlateMaterialName; }

    /**
     * Get material name of cold tube.
     * @return Material name of cold tube.
     */
    const std::string& getColdTubeMaterialName() const { return m_coldTubeMaterialName; }

    /**
     * Get sizes vector (TVector3) of cooling test plates.
     * @return Sizes vector (TVector3) of cooling test plates.
     */
    TVector3 getCoolingTestPlateslengths() const { return TVector3(m_coolingTestPlateslengthX, m_coolingTestPlateslengthY, m_coolingTestPlateslengthZ); }

    /**
     * Get radius of cold tubes.
     * @return Radius of cold tubes.
     */
    double getColdTubeR() const { return m_coldTubeR; }

    /**
     * Get outer radius of subtracted tubes for cold tube.
     * @return Outer radius of subtracted tubes for cold tube.
     */
    double getColdTubeSubtractedR() const { return m_coldTubeSubtractedR; }

    /**
     * Get cold tube wall thickness.
     * @return Cold tube wall thickness.
     */
    double getColdTubeWallThickness() const { return m_coldTubeWallThickness; }

    /**
     * Get depth of the cold tube in the cooling plate.
     * @return Depth of the cold tube in the cooling plate.
     */
    double getDepthColdTubeInPlate() const { return m_depthColdTubeInPlate; }

    /**
     * Get distance from center of the cold tube to edge of cooling plate.
     * @return Distance from center of the cold tube to edge of cooling plate.
     */
    double getColdTubeSpacing() const { return m_coldTubeSpacing; }

    /**
     * Get number of cold tubes in one plate.
     * @return number of cold tubes in one plate.
     */
    int getColdTubeNumber() const { return m_coldTubeNumber; }

    /**
     * Get vector of Rho of the cooling test plates center
     * in polar coordinate system in mm.
     * @return vector of Rho of the cooling test plates center
     * in polar coordinate system in mm.
     */
    const std::vector<double>& getCoolingTestPlatePosR() const { return m_coolingTestPlatePosR; }

    /**
     * Get vector of Azimuthal angle of the cooling test plates center
     * in polar coordinate system in deg.
     * @return vector of Azimuthal angle of the cooling test plates center
     * in polar coordinate system in deg.
     */
    const std::vector<double>& getCoolingTestPlatePosPhi() const { return m_coolingTestPlatePosPhi; }

    /**
     * Get vector of Z placement of the cooling test plates center
     * within ARICH mother volume in mm.
     * @return vector of Z placement of the cooling test plates center
     * within ARICH mother volume in mm.
     */
    const std::vector<double>& getCoolingTestPlatePosZ0() const { return m_coolingTestPlatePosZ0; }

    /**
     * Print the content of the class.
     * @param[in] title Title to be printed.
     */
    void print(const std::string& title = "Cooling system geometry parameters") const;

    /**
     * Check data consistency of the cooling system positions.
     * In case of failure print the basf2 ERROR message using B2ASSERT.
     */
    void checkCoolingSystemDataConsistency() const;

  private:

    std::string m_coolingPipeMaterialName;      /**< Material of cooling pipe */
    double m_Rmin = 0.0;                        /**< Size of cooling system pipe : inner radius in mm */
    double m_Rmax = 0.0;                        /**< Size of cooling system pipe : outer radius in mm */
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
