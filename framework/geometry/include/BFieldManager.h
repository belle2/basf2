/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef FRAMEWORK_GEOMETRY_BFIELDMANAGER_H
#define FRAMEWORK_GEOMETRY_BFIELDMANAGER_H

#include <vector>
#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldComponent.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/gearbox/Unit.h>

namespace Belle2 {
  /** Bfield manager to obtain the magnetic field at any point.
   * This class is implemented as singleton, there can only be one magnetic
   * field. It has static members to get the field at a given point.
   *
   * @note For internal use in the simulation, there is also a BFieldMap class
   * in the geometry package with similar behaviour. To ensure consistent
   * output, the BFieldFrameworkInterface component provides a wrapper around
   * BFieldMap and is used by default.
   */
  class BFieldManager {
  public:
    /** return the magnetic field at a given position.
     * @param[in] pos position, needs to be of at least size 3
     * @param[out] field magnetic field field value at position pos
     */
    static void getField(const double* pos, double* field);
    /** return the magnetic field at a given position.
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos
     */
    static TVector3 getField(const TVector3& pos)
    {
      return getField(B2Vector3D(pos)).GetTVector3();
    }
    /** return the magnetic field at a given position.
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos
     */
    static B2Vector3D getField(const B2Vector3D& pos)
    {
      return BFieldManager::getInstance().calculate(pos);
    }
    /** Return the instance of the magnetic field manager */
    static BFieldManager& getInstance();

    /** Remove all registered components */
    void clearComponents();
    /** Add a magnetic field component. Ownership is taken over by
     * BFieldManager and the component is deleted when clearComponents() is
     * called. */
    void addComponent(BFieldComponent* component)
    {
      m_components.push_back(component);
    }
    /** Set a constant magnetic field override which will be used instead of
     * the defined components.
     * @param field field value to return on all calls of getField()
     */
    void setConstantOverride(const B2Vector3D& field);
    /** Set a constant magnetic field override which will be used instead of
     * the defined components.
     * @param Bx x component of the field value to return on all calls of getField()
     * @param By y component of the field value to return on all calls of getField()
     * @param Bz z component of the field value to return on all calls of getField()
     */
    void setConstantOverride(double Bx, double By, double Bz)
    {
      setConstantOverride(B2Vector3D(Bx, By, Bz));
    }
    /** Clear a previously set override and return to normal field evaluation */
    void clearOverride()
    {
      m_overrideActive = false;
    }
  private:
    /** Singleton: private constructor */
    BFieldManager() = default;
    /** Singleton: no copy constructor */
    BFieldManager(BFieldManager&) = delete;
    /** Singleton: no assignment operator */
    BFieldManager& operator=(const BFieldManager&) = delete;
    /** Clean up the field components */
    ~BFieldManager() { clearComponents(); }
    /** Calculate the field value at a given position and return it
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos
     */
    B2Vector3D calculate(const B2Vector3D& pos);

    /** list of BField Components for field caluclation */
    std::vector<BFieldComponent*> m_components;
    /** field value to be returned if override is active */
    B2Vector3D m_overrideField;
    /** if true, m_overrideField will be returned instead of the actual field values */
    bool m_overrideActive{false};
    /** if true, somebody tried to access the magnetic field when no components
     * were defined so a fallback around the IP is in use */
    bool m_fallbackActive{false};
  };

  inline B2Vector3D BFieldManager::calculate(const B2Vector3D& pos)
  {
    // if override is active just return the override field
    if (m_overrideActive) return m_overrideField;
    // if somebody tries to access the magnetic field when no components are
    // defined we show an error (once) and return 1.5T along z if the position
    // was close enoough to the IP
    if (m_components.empty()) {
      if (!m_fallbackActive) {
        B2ERROR("Magnetic field is not initialized, are you missing the Geometry module? "
                "Assuming constant magnetic field of 1.5T close to the IP");
        m_fallbackActive = true;
      }
      return B2Vector3D(0, 0, 1.5 * Unit::T);
    }
    // loop through all components and see if we find one which is responsible for
    // the position where we want the field.
    B2Vector3D field(0, 0, 0);
    for (BFieldComponent* c : m_components) {
      if (c->inside(pos)) {
        // is it an exclusive component? if so return the field
        if (c->exclusive()) return c->getField(pos);
        // else add it to what we already have
        field += c->getField(pos);
      }
    }
    // and return the sum of all components
    return field;
  };

  inline void BFieldManager::getField(const double* pos, double* field)
  {
    B2Vector3D fieldvec = getField(B2Vector3D(pos));
    fieldvec.GetXYZ(field);
  }

  inline void BFieldManager::setConstantOverride(const B2Vector3D& field)
  {
    m_overrideActive = true;
    m_overrideField = field;
    B2WARNING("Overriding magnetic field to a constant value of " << field.PrintStringXYZ());
  }
}

#endif // FRAMEWORK_GEOMETRY_BFIELDMANAGER_H
