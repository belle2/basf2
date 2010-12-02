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
#include <map>

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
    MaterialProperty() : m_name("") {};

    /**
     * Constructor which sets the name of the property.
     * @param name The name of the material property (e.g. RINDEX, ABSLENGTH, FASTCOMPONENT, SLOWCOMPONENT).
     */
    MaterialProperty(const std::string& name)  : m_name(name) {};

    /**
     * Sets the name of the material property.
     * @param name The name of the material property (e.g. RINDEX, ABSLENGTH, FASTCOMPONENT, SLOWCOMPONENT).
     */
    void setName(const std::string& name) { m_name = name; };

    /**
     * Returns the name of the property.
     * @return The name of the property.
     */
    const std::string& getName() const { return m_name; };

    /**
     * Adds a new energy and value pair to the property.
     * @param energy The energy value for which the specified value is valid.
     * @param value The value for the given energy.
     * @return True if the value could be added. False if the specified energy was already set.
     */
    bool addValue(double energy, double value);

    /**
     * Returns the number of the values that are conatined in this property.
     * @return The number of the values in this property.
     */
    int getNumberValues() const {return m_valueMap.size(); };

    /**
     * Fills arrays for energies and values with the data stored in the property.
     * Please note: The arrays have to be allocated outside this method and have to
     * be large enough to store all data. The number of data available can be
     * retrieved by the method getNumberValues().
     * @param energies Pointer to the array which will store the energies.
     * @param values Pointer to the array which will store the values.
     */
    void fillArrays(double* energies, double* values);


  protected:

  private:

    std::string m_name; /**< The name of the material property. */
    std::map<double, double> m_valueMap; /**< The map containing the photon energies and the corresponding property values. */

    /** Class definition required for the creation of the ROOT dictionary. */
    ClassDef(MaterialProperty, 1);

  };

}

#endif /* MATERIALPROPERTY_H_ */
