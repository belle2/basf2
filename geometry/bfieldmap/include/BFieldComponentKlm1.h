/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

    /** Trivial struct representing rz coordinate */
    struct BFieldPoint {
      /** r coordinate of the point */
      double r;
      /** z coordinate of the point */
      double z;
    };

    /** The BFieldComponentklm1 constructor. */
    BFieldComponentKlm1() = default;

    /** The BFieldComponentklm1 destructor. */
    virtual ~BFieldComponentKlm1() = default;

    /**
     * Initializes the magnetic field Component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize() override;

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the Component.
     */
    virtual B2Vector3D calculate(const B2Vector3D& point) const override;

    /**
     * Terminates the magnetic field Component.
     */
    virtual void terminate() override;

    /**
     * Sets the filename of the magnetic field map.
     * @param filename The filname of the magnetic field map.
     */
    void setMapFilename(const std::string& filename) { m_mapFilename = filename; };

    /** Sets the number of barrel and endcap layers
     * @param b barrel layers
     * @param e endcap layers
     */
    void setNLayers(int b, int e) {m_nBarrelLayers = b; m_nEndcapLayers = e;}

    /** Sets the dimensions of the barrel region
     * @param minR minimal radius
     * @param maxZ max extension in z in both directions
     * @param offset map offset in z
     */
    void setBarrelRegion(double minR, double maxZ, double offset) { m_barrelRMin = minR; m_barrelZMax = maxZ; m_mapOffset = offset; }

    /** Set the dimensions of the endcap region
     * @param minR minimal radius
     * @param minZ starting z coordinate in both directions
     */
    void setEndcapRegion(double minR, double minZ) { m_endcapRMin = minR; m_endcapZMin = minZ;}

    /** Set the layer parameters
     * @bgapl0 barrel gap height for layer 0
     * @bironth barrel iron thickness
     * @egap endcap gap height
     * @dl distance between two layers
     */
    void setLayerParam(double bgapl0, double bironth, double egap, double dl)
    {
      m_barrelGapHeightLayer0 = bgapl0;  m_barrelIronThickness = bironth;
      m_endcapGapHeight = egap; m_dLayer = dl;
    }

  private:
    /** cos(pi/8) */
    double m_cospi8{cos(M_PI / 8)};
    /** cos(3pi/8) */
    double m_cos3pi8{cos(3 * M_PI / 8)};
    /** cos(pi/4) */
    double m_cospi4{cos(M_PI / 4)};

    /** The filename of the magnetic field map. */
    std::string m_mapFilename{""};
    /** Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapOffset{0};

    /** The minimum boundaries of BKLM region in r. */
    double m_barrelRMin{0};
    /** The maximum boundaries of BKLM region in r. */
    double m_barrelZMax{0};
    /** The minimum boundaries of EKLM region in r. */
    double m_endcapRMin{0};
    /** The minimum boundaries of EKLM region in z. */
    double m_endcapZMin{0};

    /** The number of layers per 1 sector for BKLM */
    int m_nBarrelLayers{0};
    /** The number of layers per 1 sector for EKLM */
    int m_nEndcapLayers{0};

    /** Gap height of BKLM layer0 */
    double m_barrelGapHeightLayer0{0};
    /** Gap height of BKLM layer1-14 */
    double m_endcapGapHeight{0};
    /** deppth of BKLM module? */
    double m_dLayer{0};
    /** Thickness of Barrel iron plate */
    double m_barrelIronThickness{0};

    /** z position of breakpoints between the two linear approximations of Bz in the barrel.
     * Index indicates the current layer */
    double m_barrelZBreakpoint[15] {0};
    /** z position of breakpoints between the two linear approximations of Br in the barrel.
     * Index indicates the current layer */
    double m_barrelRBreakpoint[15] {0};
    /** Slope of Bz before the breakpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldZSlope1[15] {0};
    /** Intercept of Bz before the beackpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldZIntercept1[15] {0};
    /** Slope of Bz after the breakpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldZSlope2[15] {0};
    /** Intercept of Bz after the beackpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldZIntercept2[15] {0};
    /** Slope of Br before the breakpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldRSlope1[15] {0};
    /** Intercept of Br before the beackpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldRIntercept1[15] {0};
    /** Slope of Br after the breakpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldRSlope2[15] {0};
    /** Intercept of Br after the beackpoint in the barrel.
     * Index indicates the current layer */
    double m_barrelFieldRIntercept2[15] {0};

    /** z position of breakpoints between linear functions in the endcap.
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapZBreakpoint[2][15][5] {{{0}}};
    /** r position of breakpoints between linear functions in the endcap
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapRBreakpoint[2][15][5] {{{0}}};

    /** Slopes of the linear approximation of Bz in the endcap.
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapFieldZSlope[2][15][5] {{{0}}};
    /** Intercepts of the linear approximation of Bz in the endcap.
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapFieldZIntercept[2][15][5] {{{0}}};
    /** Slopes of the linear approximation of Br in the endcap.
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapFieldRSlope[2][15][5] {{{0}}};
    /** Intercepts of the linear approximation of Br in the endcap.
     * First index indicates whether or not where in a gap (0) or in iron (1),
     * second index is the layer and third index is the number of breaks we
     * have in the linear approximation and their positions */
    double m_endcapFieldRIntercept[2][15][5] {{{0}}};
  };

} //end of namespace Belle2
