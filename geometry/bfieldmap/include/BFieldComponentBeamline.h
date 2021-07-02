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
  /** forward declaration */
  class BeamlineFieldMapInterpolation;

  /**
   * The BFieldComponentBeamline class.
   *
   * This class represents a magnetic field map around beamline.
   * The magnetic field map is stored as a grid in cylindrical coordinates.
   * It is defined by a minimum radius and a maximum radius, a minimum z and
   * a maximum z value, a pitch size in both, r and z, and the number of grid
   * points. The filename points to the zip file containing the magnetic field map.
   */
  class BFieldComponentBeamline : public BFieldComponentAbs {

  public:

    /** The BFieldComponentBeamline constructor. */
    BFieldComponentBeamline();

    /** The BFieldComponentBeamline destructor. */
    virtual ~BFieldComponentBeamline();

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize() override;

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual B2Vector3D calculate(const B2Vector3D& point) const override;

    /**
     * Terminates the magnetic field component.
     * This method closes the magnetic field map file.
     */
    virtual void terminate() override;

    /**
     * Check presence of beamline field at the specific space point in
     * the detector coordinate frame
     *
     * @param point The space point in Cartesian coordinates (x,y,z)
     * in [cm] at which the magnetic field presence is chcked
     * @return true is in case of the magnetic field false -- otherwise
     */
    bool isInRange(const B2Vector3D& point) const;

    /**
     * BFieldComponentBeamline instance
     *
     * @return reference to the BFieldComponentBeamline instance
     */
    static BFieldComponentBeamline& Instance();

    /**
     * Sets the filename of the magnetic field map.
     * @param filename_her The filname of the HER magnetic field map.
     * @param filename_ler The filname of the LER magnetic field map.
     */
    void setMapFilename(const std::string& filename_her, const std::string& filename_ler)
    {
      m_mapFilename_her = filename_her;
      m_mapFilename_ler = filename_ler;
    };

    /**
     * Sets the filename of the map for interpolation.
     */
    void setInterpolateFilename(const std::string& filename_her, const std::string& filename_ler)
    {
      m_interFilename_her = filename_her;
      m_interFilename_ler = filename_ler;
    };

    /**
     * Sets the size of the magnetic field map.
     * @param minZ The left (min) border of the magnetic field map region in z [cm].
     * @param maxZ The right (max) border of the magnetic field map region in z [cm].
     * @param offset The offset in z [cm] which is required because the accelerator group defines the Belle center as zero.
     */
    void setMapRegionZ(double minZ, double maxZ, double offset = 0.)
    {
      m_mapRegionZ[0] = minZ;
      m_mapRegionZ[1] = maxZ;
      m_mapOffset = offset;
    }

    /**
     * Sets the size of the magnetic field map.
     * @param minR The left (min) border of the magnetic field map region in r [cm].
     * @param maxR The right (max) border of the magnetic field map region in r [cm].
     */
    void setMapRegionR(double minR, double maxR)
    {
      m_mapRegionR[0] = minR;
      m_mapRegionR[1] = maxR;
    }
    /** Parameter to set Map Region  */
    void setBeamAngle(double beamAngle)
    {
      sincos(beamAngle, &m_sinBeamCrossAngle, &m_cosBeamCrossAngle);
    }
    /** Parameter to set Angle of the beam */

  private:
    /** The filename of the magnetic field map. */
    std::string m_mapFilename_her{""};
    /** The filename of the magnetic field map. */
    std::string m_mapFilename_ler{""};
    /** The filename of the map for interpolation. */
    std::string m_interFilename_her{""};
    /** The filename of the map for interpolation. */
    std::string m_interFilename_ler{""};
    /** The min and max boundaries of the map region in z. */
    double m_mapRegionZ[2] {0};
    /** Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapOffset{0};
    /** The min and max boundaries of the map region in r. */
    double m_mapRegionR[2] {0};
    /** The sin of the crossing angle of the beams   */
    double m_sinBeamCrossAngle{0};
    /** The cos of the crossing angle of the beams   */
    double m_cosBeamCrossAngle{1};
    /** Actual magnetic field interpolation object for HER */
    BeamlineFieldMapInterpolation* m_her{0};
    /** Actual magnetic field interpolation object for LER */
    BeamlineFieldMapInterpolation* m_ler{0};
  };

} //end of namespace Belle2
