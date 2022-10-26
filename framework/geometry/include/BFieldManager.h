/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <Math/Vector3D.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/MagneticField.h>

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
     * @param[in] pos position in framework units, needs to be of at least size 3
     * @param[out] field magnetic field field value at position pos in framework units
     */
    static void getField(const double* pos, double* field);
    /** return the magnetic field at a given position.
     * @param x x coordinate of the position where to evaluate the magnetic field
     * @param y y coordinate of the position where to evaluate the magnetic field
     * @param z z coordinate of the position where to evaluate the magnetic field
     * @returns magnetic field value at position pos in framework units
     */
    static ROOT::Math::XYZVector getField(double x, double y, double z)
    {
      return getField(ROOT::Math::XYZVector(x, y, z));
    }
    /** return the magnetic field at a given position.
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos in framework units
     */
    static ROOT::Math::XYZVector getField(const ROOT::Math::XYZVector& pos)
    {
      return BFieldManager::getInstance().calculate(pos);
    }
    /** return the magnetic field at a given position in Tesla.
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos in Tesla
     */
    static ROOT::Math::XYZVector getFieldInTesla(const ROOT::Math::XYZVector& pos)
    {
      return getField(pos) / Unit::T;
    }
    /** Return the instance of the magnetic field manager */
    static BFieldManager& getInstance();
  private:
    /** Singleton: private constructor */
    BFieldManager() = default;
    /** Singleton: no copy constructor */
    BFieldManager(BFieldManager&) = delete;
    /** Singleton: no assignment operator */
    BFieldManager& operator=(const BFieldManager&) = delete;
    /** Empty destructor: this is just a wrapper to access the database object */
    ~BFieldManager() = default;
    /** Calculate the field value at a given position and return it
     * @param pos position where to evaluate the magnetic field
     * @returns magnetic field value at position pos
     */
    ROOT::Math::XYZVector calculate(const ROOT::Math::XYZVector& pos) const;
    /** Pointer to the actual magnetic field in the database */
    DBObjPtr<MagneticField> m_magfield;
  };

  inline ROOT::Math::XYZVector BFieldManager::calculate(const ROOT::Math::XYZVector& pos) const
  {
    if (!m_magfield) B2FATAL("Could not load magnetic field configuration from database");
    return m_magfield->getField(pos);
  };

  inline void BFieldManager::getField(const double* pos, double* field)
  {
    ROOT::Math::XYZVector fieldvec = getField(ROOT::Math::XYZVector(pos[0], pos[1], pos[2]));
    field[0] = fieldvec.X();
    field[1] = fieldvec.Y();
    field[2] = fieldvec.Z();
  }
}
