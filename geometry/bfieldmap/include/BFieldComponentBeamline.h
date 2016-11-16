/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Kazutaka. Sumisawa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTBEAMLINE_H
#define BFIELDCOMPONENTBEAMLINE_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>
#include <string>

namespace Belle2 {
  /** forward declaration */
  class interpol3d_t;

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

    bool isInRange(const TVector3&) const;
    static BFieldComponentBeamline& Instance();

    /**
     * Sets the filename of the magnetic field map.
     * @param filename The filname of the magnetic field map.
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
      s_mapRegionR[0] = minR;
      s_mapRegionR[1] = maxR;
    }
    /** Parameter to set Map Region  */
    void setBeamAngle(double beamAngle)
    {
      sincos(beamAngle, &s_sinBeamCrossAngle, &s_cosBeamCrossAngle);
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
    double s_mapRegionR[2];
    /** The sin and cos of the crossing angle of the beams   */
    double s_sinBeamCrossAngle, s_cosBeamCrossAngle;
    /** Actual magnetic field interpolation objects */
    interpol3d_t* m_her{0};
    interpol3d_t* m_ler{0};
  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTBEAMLINE_H */
