/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTQUAD_H
#define BFIELDCOMPONENTQUAD_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <boost/array.hpp>
#include <string>

namespace Belle2 {

  /**
   * The BFieldComponentQuad class.
   *
   * This class represents a magnetic field map from quadrupole magnets QC[1/2]*.
   * Only the field from QC[1/2]* inside beam pipes is described.
   * Leak field inside the beam pipe from the quadruple magnet on the other beam line is also included,
   * however, leak field outside both beam pipe are not described.
   * Therefore, we should turn on this field component only when we use
   * TouschekTURTLEReader and Synchrotron radiation study,
   * which needs accurate propagation of beam particles.
   * Field strength are calculated from the magnet parameters (K0,K1,SK0,SK1) used in accelerator simulation.
   * These parameters are provided for each 4cm slice in the beam direction.
   */
  class BFieldComponentQuad : public BFieldComponentAbs {

  public:

    /** Aperture data structure. */
    struct ApertPoint {
      double s; /**< length along beamline in [cm] */
      double r; /**< aperture in [cm] */
    };

    /** Quadrupole lense data structure. This is a flat structure so
    magnetic field has only X and Y components */
    struct ParamPoint3 {
      double s;   /**< s in [cm] */
      double L;   /**< element length in [cm] */
      double mxx, mxy, mx0; /**< coefficents to calculate Bx */
      double myx, myy, my0; /**< coefficents to calculate By */
      /**
       * Calculates the X component of the magnetic field vector at
       * the specified space point from a quadrupole lense.
       *
       * @param x The X component of the space point in Cartesian coordinates (x,y) in [cm]
       * @param y The Y component of the space point in Cartesian coordinates (x,y) in [cm]
       * @return  The X component of magnetic field vector at the given space point in [T].
       */
      inline double getBx(double x, double y) const {return x * mxx + y * mxy + mx0;}
      /**
       * Calculates the Y component of the magnetic field vector at
       * the specified space point from a quadrupole lense.
       *
       * @param x The X component of the space point in Cartesian coordinates (x,y) in [cm]
       * @param y The Y component of the space point in Cartesian coordinates (x,y) in [cm]
       * @return  The Y component of magnetic field vector at the given space point in [T].
       */
      inline double getBy(double x, double y) const {return x * myx + y * myy + my0;}
    };

    /** start and stop indicies to narrow search in array */
    struct irange_t { short int i0, i1;};

    /** The BFieldComponentQuad constructor. */
    BFieldComponentQuad() = default;

    /** The BFieldComponentQuad destructor. */
    virtual ~BFieldComponentQuad() = default;

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
     * Returns the HER beam pipe aperture at given position.
     * @param s The position in HER beam-axis coordinate.
     * @return The beam pipe aperture at given position.
     */
    double getApertureHER(double s) const;

    /**
     * Returns the LER beam pipe aperture at given position.
     * @param s The position in LER beam-axis coordinate.
     * @return The beam pipe aperture at given position.
     */
    double getApertureLER(double s) const;

    /**
     * Sets the filename of the magnetic field map.
     * @param filenameHER The filename of the magnetic field map for HER.
     * @param filenameLER The filename of the magnetic field map for LER.
     * @param filenameHERleak The filename of the magnetic field map for HER (leak field from LER).
     */
    void setMapFilename(const std::string& filenameHER, const std::string& filenameLER, const std::string& filenameHERleak) { m_mapFilenameHER = filenameHER; m_mapFilenameLER = filenameLER; m_mapFilenameHERleak = filenameHERleak; }

    /**
     * Sets the filename of aperture definition file.
     * @param filenameHER The filename of the aperture definition for HER.
     * @param filenameLER The filename of the aperture definition for LER.
     */
    void setApertFilename(const std::string& filenameHER, const std::string& filenameLER) { m_apertFilenameHER = filenameHER; m_apertFilenameLER = filenameLER; }

    /**
     * Sets the size of the magnetic field map.
     * @param mapSizeHER The number of points in the HER field parameters.
     * @param mapSizeLER The number of points in the LER field parameters.
     * @param mapSizeHERleak The number of points in the HER field parameters (leak field from LER).
     */
    void setMapSize(int sizeHER, int sizeLER, int sizeHERleak) { m_mapSizeHER = sizeHER; m_mapSizeLER = sizeLER; m_mapSizeHERleak = sizeHERleak;}

    /**
     * Sets the size of the aperture map
     * @param apertSizeHER The number of points in the HER aperture file.
     * @param apertSizeLER The number of points in the LER aperture file.
     */
    void setApertSize(int sizeHER, int sizeLER) { m_apertSizeHER = sizeHER; m_apertSizeLER = sizeLER; }

  private:
    /** Magnetic field map of HER   */
    std::string m_mapFilenameHER{""};
    /** Magnetic field map of LER   */
    std::string m_mapFilenameLER{""};
    /** The filename of the magnetic field map */
    std::string m_mapFilenameHERleak{""};
    /** Filename of the aperture for HER */
    std::string m_apertFilenameHER{""};
    /** The filename of the aperture for LER */
    std::string m_apertFilenameLER{""};
    /** The size of the map for HER */
    int m_mapSizeHER{0};
    /** The size of the map for LER */
    int m_mapSizeLER{0};
    /** The size of the map */
    int m_mapSizeHERleak{0};
    /** The size of the aperture for HER */
    int m_apertSizeHER{0};
    /** The size of the aperture for LER */
    int m_apertSizeLER{0};
    /** The square of maximal aperture for fast rejection */
    double m_maxr2;

    /** The the aperture parameters for HER and LER. */
    std::vector<ApertPoint> m_ah, m_al;

    /** The map for HER and LER */
    std::vector<ParamPoint3> m_h3, m_l3;

    /** Indecies for HER and LER to accelerate binary search in std::vector<ParamPoint3> */
    std::vector<irange_t> m_indexh, m_indexl;
  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTQUAD_H */
