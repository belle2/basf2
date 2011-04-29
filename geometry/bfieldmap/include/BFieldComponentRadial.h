/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTRADIAL_H
#define BFIELDCOMPONENTRADIAL_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <boost/array.hpp>
#include <string>

namespace Belle2 {

  /**
   * The BFieldComponentRadial class.
   *
   * This class represents a radial magnetic field map.
   * The magnetic field map is stored as a grid in cylindrical coordinates.
   * It is defined by a minimum radius and a maximum radius, a minimum z and
   * a maximum z value, a pitch size in both, r and z, and the number of grid
   * points. The ZOffset is used to account for the fact that the acceleration
   * group defines 0 to be in the center of the detector, while the detector
   * group defines the IP to be the center. The filename points to the zip file
   * containing the magnetic field map.
   */
  class BFieldComponentRadial : public BFieldComponentAbs {

  public:

    struct BFieldPoint {
      double r;
      double z;
    };

    /** The BFieldComponentRadial constructor. */
    BFieldComponentRadial();

    /** The BFieldComponentRadial destructor. */
    virtual ~BFieldComponentRadial();

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize();

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
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
     * @param sizeR The number of points in the r direction.
     * @param sizeZ The number of points in the z direction.
     */
    void setMapSize(int sizeR, int sizeZ) { m_mapSize[0] = sizeR; m_mapSize[1] = sizeZ; }

    /**
     * Sets the size of the magnetic field map.
     * @param minZ The left (min) border of the magnetic field map region in z [cm].
     * @param maxZ The right (max) border of the magnetic field map region in z [cm].
     * @param offset The offset in z [cm] which is required because the accelerator group defines the Belle center as zero.
     */
    void setMapRegionZ(double minZ, double maxZ, double offset) { m_mapRegionZ[0] = minZ; m_mapRegionZ[1] = maxZ; m_mapOffset = offset; }

    /**
     * Sets the size of the magnetic field map.
     * @param minR The left (min) border of the magnetic field map region in r [cm].
     * @param maxR The right (max) border of the magnetic field map region in r [cm].
     */
    void setMapRegionR(double minR, double maxR) { m_mapRegionR[0] = minR; m_mapRegionR[1] = maxR; }

    /**
     * Sets the grid pitch of the magnetic field map.
     * @param pitchR The pitch of the grid in r [cm].
     * @param pitchZ The pitch of the grid in z [cm].
     */
    void setGridPitch(double pitchR, double pitchZ) { m_gridPitchR = pitchR; m_gridPitchZ = pitchZ; }


  protected:

  private:

    std::string m_mapFilename; /**< The filename of the magnetic field map. */
    BFieldPoint **m_mapBuffer; /**< The memory buffer for the magnetic field map. */
    int m_mapSize[2];          /**< The size of the map in r and z. */
    double m_mapRegionZ[2];    /**< The min and max boundaries of the map region in z. */
    double m_mapOffset;        /**< Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapRegionR[2];    /**< The min and max boundaries of the map region in r. */
    double m_gridPitchR;       /**< The grid pitch in r. */
    double m_gridPitchZ;       /**< The grid pitch in z. */

  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTRADIAL_H */
