/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENT3D_H
#define BFIELDCOMPONENT3D_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <boost/array.hpp>
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

    BFieldComponent3d();

    /** The BFieldComponent3d destructor. */
    virtual ~BFieldComponent3d();

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize();

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T].
     *         Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual TVector3 calculate(const TVector3& point) const;

    /**
     * Terminates the magnetic field component.
     * This method closes the magnetic field map file.
     */
    virtual void terminate();

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
     * @param errBPhi The size of the error on Bphi, Bphi_new = errBphi*Bphi [fraction].
     * @param errBz   The size of the error on Bz, Bz_new = errBz*Bz [fraction].
     */
    void setErrorRegionR(double minR = -1., double maxR = -1., double errBr = 0.0, double errBphi = 0.0, double errBz = 0.0)
    { m_errRegionR[0] = minR; m_errRegionR[1] = maxR; m_errB[0] = errBr; m_errB[1] = errBphi; m_errB[2] = errBz; }

    /**
     * @param mirrorPhi Flag to enable mirroring in phi about x-z plane
     */
    void mirrorPhi(bool mirrorPhi = 0.) { m_mirrorPhi = mirrorPhi; }

    /**
     * @param mapEnable Flag to switch on/off interpolation
     */
    void doInterpolation(bool interpolate = true) { m_interpolate = interpolate; }

    /**
     * Optino to reduce 3D to 2D map (in coordinates, _NOT_ Br, Bphi, Bz components)
     * @param mapEnable List of dimensions to enable: "rphiz", "rphi", "phiz" or "rz"
     */
    void enableCoordinate(std::string mapEnable = "rphiz") { m_mapEnable = mapEnable; }

  protected:

  private:

    /**
     * Interpolate the value of B-field for a given set of map indicies (ir, iphi, iz) and exact position (r, phi, z)
     */
    double interpolate(int& ir, int& iphi, int& iz, double& r, double& phi, double& z,
                       const std::vector< std::vector< std::vector<double> > >& bmap) const;

    std::string m_mapFilename; /**< The filename of the magnetic field map. */
    std::vector< std::vector< std::vector<double> > > m_mapBuffer[3]; /**< The memory buffer for the magnetic field map. */
    std::string m_mapEnable;   /**< Enable different dimension, \"rphiz\", \"rphi\", \"phiz\" or \"rz\" > */
    bool m_interpolate;        /**< Flag to switch on/off interpolation > */
    int m_mapSize[3];          /**< The size of the map in r, phi and z. */
    double m_mapRegionZ[2];    /**< The min and max boundaries of the map region in z. */
    double m_mapOffset;        /**< Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapRegionR[2];    /**< The min and max boundaries of the map region in r. */
    double m_gridPitch[3];     /**< The grid pitch in r,phi,z. */
    double m_exRegionZ[2];     /**< The min and max boundaries of the excluded region in z. */
    double m_exRegionR[2];     /**< The min and max boundaries of the excluded region in r. */
    bool m_exRegion;           /**< Flag to indicate whether there is a region to exclude. > */
    bool m_mirrorPhi;          /**< Flag to indicate whether there is a region to exclude. > */
    double m_errRegionR[2];    /**< The min and max boundaries of the region in r to apply error. */
    double m_errB[3];          /**< The error Br, Bphi, Bz as a scale factor (B_new = m_errB * B_old). */

  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENT3D_H */
