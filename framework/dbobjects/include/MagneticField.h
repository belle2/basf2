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
#include <framework/geometry/B2Vector3.h>
#include <framework/dbobjects/MagneticFieldComponent.h>
#include <framework/gearbox/Unit.h>

namespace Belle2 {
  /** Magnetic field map.
   *
   * It can be used directly using a \ref DBObjPtr "DBObjPtr<MagneticField>"
   * instance in your class. Alternatively there is a \ref BFieldManager class
   * which encapsulates the DBObjPtr already. Field evaluation is then a simple
   * call to BFieldManager::getField().
   *
   * This class contains the Magnetic field map to be stored in the Database
   * and used during simulation and reconstruction. The field can be assembled
   * from multiple components (constant, 3D, 2D, ...) in different regions. By
   * default all components valid in a certain region will be added to obtain
   * the final field value but components can also be flagged as exclusive in
   * which case only the field of this particular component will be returned.
   */
  class MagneticField: public TObject {
  public:
    /** Empty Constructor */
    MagneticField(): TObject() {}
    /** Delete all components */
    ~MagneticField()
    {
      for (auto c : m_components) delete c;
      m_components.clear();
    }
    /** Disallow copying the magnetic field */
    MagneticField(const MagneticField&) = delete;
    /** But allow move construction */
    MagneticField(MagneticField&& other) noexcept: m_components(std::move(other.m_components)) {}
    /** Calculate the magnetic field at a given position.
     * @param pos position where the field should be evaluated in framework units.
     * @return magnetic field at pos in framework units.
     * @warning The returned field is in basf2 units which is NOT Tesla. If you
     *     need the field strength in a fixed unit please divide the returned value
     *     by Unit::T or similar. */
    B2Vector3D getField(const B2Vector3D& pos) const;
    /** Convenience function to get the field directly in Tesla
     * @param pos position where the field should be evaluated in framework units.
     * @return magnetic field at pos in framework units.
     */
    B2Vector3D getFieldInTesla(const B2Vector3D& pos) const { return getField(pos) / Unit::T; }
    /** Add a new component to the magnetic field. Ownership is taken over by the MagneticField instance */
    void addComponent(MagneticFieldComponent* component) { m_components.emplace_back(component); }
  private:
    /** Magnetic field components to evaluate the field */
    std::vector<MagneticFieldComponent*> m_components;
    /** ROOT dictionary definition */
    ClassDef(MagneticField, 1);
  };

  inline B2Vector3D MagneticField::getField(const B2Vector3D& pos) const
  {
    B2Vector3D field;
    for (auto c : m_components) {
      if (c->inside(pos)) {
        // is it an exclusive component? If so return the field
        if (c->isExclusive()) return c->getField(pos);
        // else add it to what we already have
        field += c->getField(pos);
      }
    }
    return field;
  }
} //Belle2 namespace
