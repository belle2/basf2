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
   * The BFieldComponent3d class.
   *
   * This class represents a 3d magnetic field map.
   * The magnetic field map is stored as a grid in cylindrical coordinates.
   * It is defined by a minimum radius and a maximum radius, a minimum z and
   * a maximum z value (phi is assumed to be in 360 degrees range,
   * option to mirror about the x-y plane exist), a pitch
   * size in both, r, z and phi and the number of grid points.
   * The ZOffset is used to account for the fact that the acceleration
   * group defines 0 to be in the center of the detector, while the detector
   * group defines the IP to be the center. The filename points to the zip file
   * containing the magnetic field map.
   */
  class BFieldComponent3d : public BFieldComponentAbs {

  public:

    /** The BFieldComponent3d constructor. */
    BFieldComponent3d() = default;

    /** The BFieldComponent3d destructor. */
    virtual ~BFieldComponent3d() = default;

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize() override;

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T].
     *         Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual B2Vector3D calculate(const B2Vector3D& point) const override;

    /**
     * Terminates the magnetic field component.
     * This method closes the magnetic field map file.
     */
    virtual void terminate() override;

    /**
     * Sets the filename of the magnetic field map.
     * @param filename The filname of the magnetic field map.
     */
    void setMapFilename(const std::string& filename) { m_mapFilename = filename; };

    /**
     * Sets the size of the magnetic field map.
     * @param sizeR   The number of points in the r direction.
     * @param sizePhi The number of points in the phi direction.
     * @param sizeZ   The number of points in the z direction.
     */
    void setMapSize(int sizeR, int sizePhi, int sizeZ) { m_mapSize[0] = sizeR; m_mapSize[1] = sizePhi; m_mapSize[2] = sizeZ; }

    /**
     * Sets the size of the magnetic field map.
     * @param minZ The left (min) border of the magnetic field map region in z [m] -> [cm].
     * @param maxZ The right (max) border of the magnetic field map region in z [m] -> [cm].
     * @param offset The offset in z [m] -> [cm] which is required because the accelerator group defines the Belle center as zero.
     */
    void setMapRegionZ(double minZ, double maxZ, double offset) { m_mapRegionZ[0] = minZ; m_mapRegionZ[1] = maxZ; m_mapOffset = offset; }

    /**
     * Sets the size of the magnetic field map.
     * @param minR The left (min) border of the magnetic field map region in r [m] -> [cm].
     * @param maxR The right (max) border of the magnetic field map region in r [m] -> [cm].
     */
    void setMapRegionR(double minR, double maxR) { m_mapRegionR[0] = minR; m_mapRegionR[1] = maxR; }

    /**
     * Sets the grid pitch of the magnetic field map.
     * @param pitchR   The pitch of the grid in r [m] -> [cm].
     * @param pitchPhi The pitch of the grid in phi [degrees] -> [rad] converted back to degrees in the code.
     * @param pitchZ   The pitch of the grid in z [m] -> [cm].
     */
    void setGridPitch(double pitchR, double pitchPhi, double pitchZ) { m_gridPitch[0] = pitchR; m_gridPitch[1] = pitchPhi; m_gridPitch[2] = pitchZ; }

    /**
     * Sets the size of the magnetic field map to exclude.
     * @param minZ The left (min) border of the magnetic field map region in z [m] -> [cm].
     * @param maxZ The right (max) border of the magnetic field map region in z [m] -> [cm].
     */
    void setExcludeRegionZ(double minZ, double maxZ) { m_exRegionZ[0] = minZ + m_mapOffset; m_exRegionZ[1] = maxZ + m_mapOffset; }

    /**
     * Sets the size of the magnetic field map to exclude.
     * @param minR The left (min) border of the magnetic field map region in r [m] -> [cm].
     * @param maxR The right (max) border of the magnetic field map region in r [m] -> [cm].
     */
    void setExcludeRegionR(double minR, double maxR) { m_exRegionR[0] = minR; m_exRegionR[1] = maxR; }

    /**
     * Sets the size of the magnetic field map to apply error on B.
     * @param minR    The left (min) border of the magnetic field map region in r [m] -> [cm].
     * @param maxR    The right (max) border of the magnetic field map region in r [m] -> [cm].
     * @param errBr   The size of the error on Br, Br_new = errBr*Br [fraction].
     * @param errBphi The size of the error on Bphi, Bphi_new = errBphi*Bphi [fraction].
     * @param errBz   The size of the error on Bz, Bz_new = errBz*Bz [fraction].
     */
    void setErrorRegionR(double minR = -1., double maxR = -1., double errBr = 0.0, double errBphi = 0.0, double errBz = 0.0)
    { m_errRegionR[0] = minR; m_errRegionR[1] = maxR; m_errB[0] = errBr; m_errB[1] = errBphi; m_errB[2] = errBz; }

    /**
     * @param mirrorPhi Flag to enable mirroring in phi about x-z plane
     */
    void mirrorPhi(bool mirrorPhi = 0.) { m_mirrorPhi = mirrorPhi; }

    /**
     * @param interpolate Flag to switch on/off interpolation
     */
    void doInterpolation(bool interpolate = true) { m_interpolate = interpolate; }

    /**
     * Optino to reduce 3D to 2D map (in coordinates, _NOT_ Br, Bphi, Bz components)
     * @param mapEnable List of dimensions to enable: "rphiz", "rphi", "phiz" or "rz"
     */
    void enableCoordinate(const std::string& mapEnable = "rphiz") { m_mapEnable = mapEnable; }
  protected:

  private:

    /**
     * Interpolate the value of B-field between (ir, iphi, iz) and (ir+1, iphi+1, iz+1) using weights (wr, wphi, wz)
     */
    B2Vector3D interpolate(unsigned int ir, unsigned int iphi, unsigned int iz, double wr, double wphi, double wz) const;

    /** The filename of the magnetic field map. */
    std::string m_mapFilename{""};
    /** The memory buffer for the magnetic field map. */
    std::vector<B2Vector3F> m_bmap;
    /** Enable different dimension, \"rphiz\", \"rphi\", \"phiz\" or \"rz\" > */
    std::string m_mapEnable{"rphiz"};
    /** Flag to switch on/off interpolation > */
    bool m_interpolate{true};
    /** The size of the map in r, phi and z. */
    int m_mapSize[3];
    /** The min and max boundaries of the map region in z. */
    double m_mapRegionZ[2] {0};
    /** Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapOffset{0};
    /** The min and max boundaries of the map region in r. */
    double m_mapRegionR[2] {0};
    /** The grid pitch in r,phi,z. */
    double m_gridPitch[3] {0};
    /** The inverted grid pitch in r,phi,z. */
    double m_igridPitch[3] {0};
    /** The min and max boundaries of the excluded region in z. */
    double m_exRegionZ[2] {0};
    /** The min and max boundaries of the excluded region in r. */
    double m_exRegionR[2] {0};
    /** Flag to indicate whether there is a region to exclude. > */
    bool m_exRegion{true};
    /** Flag to indicate whether there is a region to exclude. > */
    bool m_mirrorPhi{true};
    /** The min and max boundaries of the region in r to apply error. */
    double m_errRegionR[2] {0};
    /** The error Br, Bphi, Bz as a scale factor (B_new = m_errB * B_old). */
    double m_errB[3] {0};
  };

} //end of namespace Belle2
