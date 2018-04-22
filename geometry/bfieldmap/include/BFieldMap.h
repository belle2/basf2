/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDMAP_H_
#define BFIELDMAP_H_

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <framework/geometry/B2Vector3.h>
#include <framework/logging/Logger.h>

#include <string>
#include <list>


namespace Belle2 {
  /**
   * This class represents the magnetic field of the Belle II detector.
   * Its main method is getBField() which returns the Bfield vector at the
   * specified space point.
   *
   * It is designed as a singleton.
   */
  class BFieldMap {

  public:

    /**
     * Static method to get a reference to the BFieldMap instance.
     *
     * @return A reference to an instance of this class.
     */
    static BFieldMap& Instance();

  private:
    /**
     * Returns the magnetic field of the Belle II detector at the specified space point.
     * The space point is given in Cartesian coordinates (x,y,z) in [cm].
     *
     * @param point The space point in Cartesian coordinates.
     * @return A three vector of the magnetic field in [T] at the specified space point.
     */
    B2Vector3D getBField(const B2Vector3D& point) const;
  public:

    /**
     * Adds a new BField component to the Belle II magnetic field.
     * The class of the magnetic field component has to inherit from BFieldComponentAbs.
     * @return A reference to the added magnetic field component.
     */
    template<class BFIELDCOMP> BFIELDCOMP& addBFieldComponent();

    /** Initialize the magnetic field after adding all components */
    void initialize();

    /**
     * Clear the existing components
     */
    void clear();

  protected:

    std::list<BFieldComponentAbs*> m_components; /**< The components of the magnetic field. */


  private:

    bool m_isMapInitialized; /**< If false the map hasn't been initialized yet.*/

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    BFieldMap();

    /** Disable/Hide the copy constructor. */
    BFieldMap(const BFieldMap&);

    /** Disable/Hide the copy assignment operator. */
    BFieldMap& operator=(const BFieldMap&);

    /** The destructor of the BFieldMap class. */
    virtual ~BFieldMap();

    /** allow destruction of instance */
    friend struct std::default_delete<BFieldMap>;
    /** only allow lookup by framework payload interface from now on */
    friend class BFieldFrameworkInterface;
  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------
  template<class BFIELDCOMP> BFIELDCOMP&  BFieldMap::addBFieldComponent()
  {
    BFIELDCOMP* newComponent = new BFIELDCOMP;
    m_components.push_back(newComponent);
    return *newComponent;
  }


  inline B2Vector3D BFieldMap::getBField(const B2Vector3D& point) const
  {
    B2Vector3D magFieldVec(0.0, 0.0, 0.0);
    //Check that the point makes sense
    if (std::isnan(point.X()) || std::isnan(point.Y()) || std::isnan(point.Z())) {
      B2ERROR("Bfield requested for a position containing NaN, returning field 0");
      return magFieldVec;
    }
    //Loop over all magnetic field components and add their magnetic field vectors
    for (const BFieldComponentAbs* comp : m_components) {
      magFieldVec += comp->calculate(point);
    }
    return magFieldVec;
  }


} //end of namespace Belle2

#endif /* BFIELDMAP_H_ */
