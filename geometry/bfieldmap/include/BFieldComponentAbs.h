/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTABS_H
#define BFIELDCOMPONENTABS_H

#include <TVector3.h>

namespace Belle2 {

  /**
   * The BFieldComponentAbs class.
   *
   * This abstract class represents a component of the Belle II detector
   * magnetic field. Usually, a component represents the magnetic field
   * in a certain region of the Belle II detector. The check if a specified
   * space point lies inside the region described by the component, has to
   * be done by the component class itself.
   *
   * All magnetic field components have to inherit from this class.
   */
  class BFieldComponentAbs {

  public:

    /** The BFieldComponentAbs constructor. */
    BFieldComponentAbs() {};

    /** The BFieldComponentAbs destructor. */
    virtual ~BFieldComponentAbs() {};

    /**
     * Initializes the magnetic field component.
     * This method should be used to open and load files
     * containing the magnetic field data.
     */
    virtual void initialize() {};

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * All magnetic field component classes have to overwrite this method.
     * Please note: The magnetic field component class has to perform the
     * check if the space point lies within the space region it describes
     * by itself. If the space point given doesn't lie inside the space
     * region this method should return TVector(0,0,0).
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Return a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual TVector3 calculate(const TVector3& point) const = 0;

    /**
     * Terminates the magnetic field component.
     * This method should be used to close files that have
     * been opened in the initialize() method.
     */
    virtual void terminate() {};


  protected:

  private:

  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTABS_H */
