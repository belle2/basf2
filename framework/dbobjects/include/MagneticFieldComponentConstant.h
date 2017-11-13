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
  class MagneticFieldComponentConstant final: public MagneticFieldComponent {
  public:
    /** empty constructor for ROOT */
    MagneticFieldComponentConstant(): MagneticFieldComponent(false) {};
    /** full constructor
     * @param field magnetic field strength inside the defined region
     * @param minR minimal R=sqrt(x^2+y^2) for which this field is present
     * @param maxR maximal R=sqrt(x^2+y^2) for which this field is present
     * @param minZ minimal Z for which this field is present
     * @param maxZ maximal Z for which this field is present
     */
    MagneticFieldComponentConstant(const B2Vector3D& field, float minR, float maxR, float minZ,
                                   float maxZ): MagneticFieldComponent(false),
      m_field(field), m_minR(minR), m_maxR(maxR), m_minZ(minZ), m_maxZ(maxZ) {}
    /** constructor for unlimited field */
    explicit MagneticFieldComponentConstant(const B2Vector3D& field): MagneticFieldComponentConstant(field, 0,
          std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(),
          std::numeric_limits<float>::infinity()) {}
    /** return whether we are inside the active region for this component */
    bool inside(const B2Vector3D& pos) const override
    {
      const float r = pos.Perp();
      const float z = pos.Z();
      return (r >= m_minR && r <= m_maxR && z >= m_minZ && z <= m_maxZ);
    }
    /** return the field assuming we are inside the active region as returned by inside() */
    B2Vector3D getField(__attribute((unused)) const B2Vector3D& pos) const override { return m_field; }
  private:
    /** magnetic field strength */
    B2Vector3D m_field;
    /** minimal R=sqrt(x^2+y^2) for which this field is present */
    float m_minR{0};
    /** maximal R=sqrt(x^2+y^2) for which this field is present */
    float m_maxR{0};
    /** minimal Z for which this field is present */
    float m_minZ{0};
    /** maximal Z for which this field is present */
    float m_maxZ{0};
    /** ROOT dictionary */
    ClassDefOverride(MagneticFieldComponentConstant, 1);
  };
}; // Belle2 namespace
