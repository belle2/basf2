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

#include "TVectorD.h"
#include "TMatrixD.h"

#include <boost/array.hpp>
#include <string>
#include <vector>

namespace Belle2 {

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

    /** BField Class as grid in cylindrical coordinates */
    struct BFieldPoint {     /** BField Class as grid in cylindrical coordinates*/
      double r;              /**< Position in radius r */
      double phi;            /**< Position in phi*/
      //double z;
      double Br;             /**< BField coordinate in radius r */
      double Bphi;           /**< BField coordinate in phi   */
      double Bz;             /**< BField coordinate in z     */
    };

    /** Interpolation Point */
    struct InterpolationPoint {
      std::vector<int> p;   /**< Parameter in p */
    };

    /** The BFieldComponentBeamline constructor. */
    BFieldComponentBeamline() = default;

    /** The BFieldComponentBeamline destructor. */
    virtual ~BFieldComponentBeamline() = default;

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize()
    {
      initialize_her();    /** Initialization of LER and HER */
      initialize_ler();
    }
    /**
     * Initializes the magnetic field component around the beampipe
     * @param isher parameter  for Lower Energy Ring
     */
    void initialize_beamline(int isher);                 /** Parameter for Lower Energy Ring */
    void initialize_her(void)
    {
      initialize_beamline(1);   /** Higher Beam Ring */
    }
    void initialize_ler(void)
    {
      initialize_beamline(0);   /** Lower Beam Ring  */
    }

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual TVector3 calculate(const TVector3& point) const;
    TVector3 calculate_beamline(const TVector3& point0, int isher) const;  /**< Beamline Calculation */
    TVector3 calculate_her(const TVector3& point) const
    {
      return calculate_beamline(point, 1);   /**< HER calculation */
    }
    TVector3 calculate_ler(const TVector3& point) const
    {
      return calculate_beamline(point, 0);   /**< LER calculation */
    }

    /**
     * Terminates the magnetic field component.
     * This method closes the magnetic field map file.
     */
    virtual void terminate();

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
    static void setMapRegionR(double minR, double maxR)
    {
      s_mapRegionR[0] = minR;
      s_mapRegionR[1] = maxR;
    }
    /** Parameter to set Map Region  */
    static void setBeamAngle(double beamAngle)
    {
      sincos(beamAngle, &s_sinBeamCrossAngle, &s_cosBeamCrossAngle);
    }
    /** Parameter to set Angle of the beam */

    /**
     * calculate coefficient for interpolation
     */
    TVectorD calculateCoefficientRectangle(const TVector3& x, const std::vector<TVector3>& vx) const;
    /** Parameter for calculation of Coefficient Rectangle */
    TVectorD calculateCoefficientTriangle(const TVector3& x, const std::vector<TVector3>& vx) const;
    /** Parameter for calculation of Coefficient Triangle  */
    static bool isInRange(const TVector3& point);
    /** Parameter for checking the range of beam */

  private:
    /** The filename of the magnetic field map. */
    std::string m_mapFilename_her{""};
    /** The filename of the magnetic field map. */
    std::string m_mapFilename_ler{""};
    /** The filename of the map for interpolation. */
    std::string m_interFilename_her{""};
    /** The filename of the map for interpolation. */
    std::string m_interFilename_ler{""};
    /** The memory buffer for the magnetic field map. */
    BFieldPoint** m_mapBuffer_ler{nullptr};
    /** The memory buffer for the magnetic field map. */
    BFieldPoint** m_mapBuffer_her{nullptr};
    /** The memory buffer for the interpolation map. */
    InterpolationPoint** m_interBuffer_ler{nullptr};
    /** The memory buffer for the interpolation map. */
    InterpolationPoint** m_interBuffer_her{nullptr};
    /** The min and max boundaries of the map region in z. */
    double m_mapRegionZ[2] {0};
    /** Offset required because the accelerator group defines the Belle center as zero. */
    double m_mapOffset{0};
    /** The grid pitch in z. */
    double m_gridPitchZ[2] {0};

    /** Parameter for the size of the map in Z direction */
    int m_mapSizeZ{0};
    /** Parameter for the size of the map in phi direction  */
    int m_mapSizeRPhi[2] {0};
    /** Parameter for   */
    int m_interSizeX{0};
    /** Grid Offset in Phi direction   */
    int m_offsetGridRPhi[2] {0};
    /** Grid Offset in the Z direction */
    int m_offsetGridZ{0};
    /** Number of grid in the radial direction  */
    int m_nGridR{0};
    /** Number of grid in the phi direction     */
    int m_nGridPhi{0};
    /** Parameter   */
    double m_jointR{0};
    /** Parameter   */
    double m_jointZ{0};
    /** Parameter for maximum Radius   */
    double m_interMaxRadius{0};
    /** Parameter for the grid size    */
    double m_interGridSize{0};

    /** The min and max boundaries of the map region in r. */
    static double s_mapRegionR[2];
    /** The sin and cos of the crossing angle of the beams   */
    static double s_sinBeamCrossAngle, s_cosBeamCrossAngle;
  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTBEAMLINE_H */
