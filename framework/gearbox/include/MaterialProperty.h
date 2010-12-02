/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MATERIALPROPERTY_H_
#define MATERIALPROPERTY_H_

#include <TObject.h>
#include <TTree.h>
#include <string>

namespace Belle2 {

  /**
   * The MaterialProperty class.
   *
   * Defines a single material property .
   */
  class MaterialProperty: public TObject {

  public:

    /**
     * Default constructor for ROOT.
     */
    MaterialProperty();

    /**
     * Constructor which sets the name of the property.
     * @param name The name of the material property (e.g. RINDEX, ABSLENGTH, FASTCOMPONENT, SLOWCOMPONENT).
     */
    MaterialProperty(const std::string& name);

    /**
     * Sets the name of the material property.
     * @param name The name of the material property (e.g. RINDEX, ABSLENGTH, FASTCOMPONENT, SLOWCOMPONENT).
     */
    void setName(const std::string& name) { m_name = name; };

    /**
     * Adds a new energy and value pair to the property.
     * @param energy The energy value for which the specified value is valid.
     * @param value The value for the given energy.
     */
    void addValue(double energy, double value);


  protected:

  private:

    std::string m_name; /**< The name of the material property. */
    TTree m_valueTree;  /**< The tree containing the photon energies and the corresponding property values. */

    double m_energy; //! Local variable to store the energy value for the ROOT tree.
    double m_value; //! Local variable to store the property value for the ROOT tree.



    /** Class definition required for the creation of the ROOT dictionary. */
    ClassDef(MaterialProperty, 1);

  };

}

#endif /* MATERIALPROPERTY_H_ */
