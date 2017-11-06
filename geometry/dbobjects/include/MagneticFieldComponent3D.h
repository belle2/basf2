/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/dbobjects/MagneticFieldComponent.h>

namespace Belle2 {
  /** Describe one component of the Geometry */
  class MagneticFieldComponent3D final: public MagneticFieldComponent {
  public:
    /** empty constructor for ROOT */
    MagneticFieldComponent3D(): MagneticFieldComponent(false) {};
    /** Full constructor to create an object from data */
    MagneticFieldComponent3D(double minR, double maxR, double minZ, double maxZ,
                             int nR, int nPhi, int nZ,
                             std::vector<B2Vector3F>&& fieldmap):
      MagneticFieldComponent(true), m_minR(minR), m_maxR(maxR), m_minZ(minZ), m_maxZ(maxZ), m_mapSize{nR, nPhi, nZ},
      m_gridPitch{static_cast<float>((maxR - minR) / (nR - 1)), static_cast<float>(M_PI / (nPhi - 1)), static_cast<float>((maxZ - minZ) / (nZ - 1))},
      m_invgridPitch{static_cast<float>(1. / m_gridPitch[0]), static_cast<float>(1. / m_gridPitch[1]), static_cast<float>(1. / m_gridPitch[2])},
      m_bmap(fieldmap)
    {}

    /** return whether we are inside the active region for this component */
    bool inside(const B2Vector3D& pos) const override
    {
      const float z = pos.Z();
      if (z < m_minZ || z > m_maxZ) return false;
      const float r = pos.Perp();
      return (r >= m_minR && r <= m_maxR);
    }
    /** return the field assuming we are inside the active region as returned by inside() */
    B2Vector3D getField(const B2Vector3D& pos) const override;
  private:
    /** Linear interpolate the magnetic field inside a bin
     * @param ir number of the bin in r
     * @param iphi number of the bin in phi
     * @param iz number of the bin in z
     * @param wr r weight: fraction we are away from the lower r corner relative to the pitch size
     * @param wphi phi weight: fraction we are away from the lower phi corner relative to the pitch size
     * @param wz phi weight: fraction we are away from the lower z corner relative to the pitch size
     */
    B2Vector3D interpolate(unsigned int ir, unsigned int iphi, unsigned int iz, double wr, double wphi, double wz) const;
    /** minimal R=sqrt(x^2+y^2) for which this field is present */
    float m_minR{0};
    /** maximal R=sqrt(x^2+y^2) for which this field is present */
    float m_maxR{0};
    /** minimal Z for which this field is present */
    float m_minZ{0};
    /** maximal Z for which this field is present */
    float m_maxZ{0};
    /** number of bins in r, phi and z */
    int m_mapSize[3] {0};
    /** grid pitch in r, phi and z */
    float m_gridPitch[3] {0};
    /** inverted grid pitch in r, phi and z */
    float m_invgridPitch[3] {0};
    /** magnetic field strength */
    std::vector<B2Vector3F> m_bmap;
    /** ROOT dictionary */
    ClassDefOverride(MagneticFieldComponent3D, 1);
  };
}; // Belle2 namespace
