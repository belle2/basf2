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
#ifndef FRAMEWORK_GEOMETRY_BFIELDCOMPONENT_H
#define FRAMEWORK_GEOMETRY_BFIELDCOMPONENT_H

#include <framework/geometry/B2Vector3.h>

namespace Belle2 {
  /** Abstract base class for BField components.
   * This class is the base class for all magnetic field components. Each
   * concrete component needs to implement the inside() and getField() members
   * which should check if the point is inside the area covered by the
   * component and return the field at that given point respectively.
   *
   * All defined components will be added together to get the total magnetic
   * field. If some of the components are flagged as exclusive the field from
   * the first exclusive component where the point is inside the range will be
   * used as the full field.
   */
  class BFieldComponent {
  public:
    /** Constructor
     * @param exclusive if set to true this component will be exclusive and the
     * field of other components will not be added
     */
    explicit BFieldComponent(bool exclusive): m_exclusive(exclusive) {}
    /** returns whether the field is set to exclusive mode */
    bool exclusive() const { return m_exclusive; }
    /** check whether the point pos is inside the volume covered by the component */
    virtual bool inside(const B2Vector3D& pos) = 0;
    /** return the field at point pos */
    virtual B2Vector3D getField(const B2Vector3D& pos) = 0;
    /** destructor */
    virtual ~BFieldComponent() {}
  private:
    /** whether or not the component is exclusive */
    bool m_exclusive{false};
  };
}
#endif // FRAMEWORK_GEOMETRY_BFIELDCOMPONENT_H
