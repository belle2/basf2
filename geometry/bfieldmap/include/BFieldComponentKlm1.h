/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Kazutaka SUMISAWA                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTKLM1_H
#define BFIELDCOMPONENTKLM1_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <string>

namespace Belle2 {

  /**
   * The Bfieldcomponentklm1 class.
   *
   * This class represents a magnetic field map in KLM region and outside of solenoid.
   * This magnetic field is for Belle. This is porting class from g4superb to basf2.
   * The magnetic field map is stored as linear functions in cylindrical coordinates.
   * It is defined by a maximum radius, a minimum z, r and z,
   * the number of layers for Barrel KLM and Endcap KLM.
   * The ZOffset is used to account for the fact that the acceleration
   * group defines 0 to be in the center of the detector, while the detector
   * group defines the IP to be the center.
   */
  class BFieldComponentKlm1 : public BFieldComponentAbs {

  public:

    struct BFieldPoint {
      double r;
      double z;
    };

    /** The BFieldComponentklm1 constructor. */
    BFieldComponentKlm1();

    /** The BFieldComponentklm1 destructor. */
    virtual ~BFieldComponentKlm1();

    /**
     * Initializes the magnetic field Component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize();

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the Component.
     */
    virtual TVector3 calculate(const TVector3& point) const;

    /**
     * Terminates the magnetic field Component.
     */
    virtual void terminate();

    /**
     * Sets the filename of the magnetic field map.
     * @param filename The filname of the magnetic field map.
     */
    void setMapFilename(const std::string& filename) { m_mapFilename = filename; };

    void setNLayers(int b, int e) {m_nBarrelLayers = b; m_nEndcapLayers = e;}

    void setBarrelRegion(double minR, double maxZ, double offset) { m_barrelRMin = minR; m_barrelZMax = maxZ; m_mapOffset = offset; }

    void setEndcapRegion(double minR, double minZ) { m_endcapRMin = minR; m_endcapZMin = minZ;}

    void setLayerParam(double bgapl0, double bironth, double egap, double dl) {
      m_barrelGapHeightLayer0 = bgapl0;  m_barrelIronThickness = bironth;
      m_endcapGapHeight = egap; m_dLayer = dl;
    }


  protected:

  private:

    double m_cospi8; /**< cos(pi/8) */
    double m_cos3pi8; /**< cos(3pi/8) */
    double m_cospi4; /**< cos(pi/4) */

    std::string m_mapFilename; /**< The filename of the magnetic field map. */
    double m_mapOffset;        /**< Offset required because the accelerator group defines the Belle center as zero. */

    double m_barrelRMin; /**< The minimum boundaries of BKLM region in r. */
    double m_barrelZMax; /**< The maximum boundaries of BKLM region in r. */
    double m_endcapRMin; /**< The minimum boundaries of EKLM region in r. */
    double m_endcapZMin; /**< The minimum boundaries of EKLM region in z. */

    int m_nBarrelLayers; /**< The number of layers per 1 sector for BKLM */
    int m_nEndcapLayers; /**< The number of layers per 1 sector for EKLM */

    double m_barrelGapHeightLayer0; /**< Gap height of BKLM layer0 */
    double m_endcapGapHeight;  /**< Gap height of BKLM layer1-14 */
    double m_dLayer;  /**< deppth of BKLM module? */
    double m_barrelIronThickness;  /**< Thickness of Barrel iron plate */

    // parameters for field map, field map represent linear function
    double m_barrelZBreakpoint[15];
    double m_barrelRBreakpoint[15];

    double m_barrelFieldZSlope1[15];
    double m_barrelFieldZIntercept1[15];
    double m_barrelFieldZSlope2[15];
    double m_barrelFieldZIntercept2[15];
    double m_barrelFieldRSlope1[15];
    double m_barrelFieldRIntercept1[15];
    double m_barrelFieldRSlope2[15];
    double m_barrelFieldRIntercept2[15];

    double m_endcapZBreakpoint[2][15][5];
    double m_endcapRBreakpoint[2][15][5];

    double m_endcapFieldZSlope[2][15][5];
    double m_endcapFieldZIntercept[2][15][5];
    double m_endcapFieldRSlope[2][15][5];
    double m_endcapFieldRIntercept[2][15][5];
    // > for KLM field
  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTKLM1_H */
