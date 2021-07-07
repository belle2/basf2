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

    /** Magnetic field data structure. */
    struct BFieldPoint {
      double r; /**< Magnetic field in r direction. */
      double z; /**< Magnetic field in z direction. */
    };

    /** The BFieldComponentRadial constructor. */
    BFieldComponentRadial() = default;

    /** The BFieldComponentRadial destructor. */
    virtual ~BFieldComponentRadial() = default;

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


    /**
     * Sets prameter for EKLM
     * @param srmin minimum radius for the gap in endyoke [cm].
     * @param zyoke minimum Z of endyoke [cm].
     * @param gaph height of the gap in endyoke [cm].
     * @param iront thickness of iron plate in endyoke [cm].
     */
    void setKlmParameters(double srmin, double zyoke, double gaph, double iront)
    {
      m_slotRMin = srmin, m_endyokeZMin = zyoke; m_gapHeight = gaph; m_ironPlateThickness = iront;
    }

  protected:

  private:

    std::string m_mapFilename{""}; /**< The filename of the magnetic field map. */
    std::vector<BFieldPoint> m_mapBuffer; /**< The memory buffer for the magnetic field map. */
    int m_mapSize[2] {0};         /**< The size of the map in r and z. */
    double m_mapRegionZ[2] {0};   /**< The min and max boundaries of the map region in z. */
    double m_mapOffset{0};        /**< Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapRegionR[2] {0};   /**< The min and max boundaries of the map region in r. */
    double m_gridPitchR{0};       /**< The grid pitch in r. */
    double m_gridPitchZ{0};       /**< The grid pitch in z. */
    double m_igridPitchR{0};       /**< The reciprocal of grid pitch in r. */
    double m_igridPitchZ{0};       /**< The reciprocal of grid pitch in z. */

    double m_slotRMin{0};          /**< minimum radius for the gap in endyoke */
    double m_endyokeZMin{0};       /**< minimum Z of endyoke */
    double m_gapHeight{0};         /**< height of the gap in endyoke */
    double m_ironPlateThickness{0};/**< thickness of iron plate in endyoke */
    double m_Layer{0};            /**< height of the layer (gap + iron plate) in endyoke */
    double m_iLayer{0};           /**< reciprocal of height of the layer (gap + iron plate) in endyoke */
  };

} //end of namespace Belle2
