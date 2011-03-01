/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MATERIALPROPERTYLIST_H_
#define MATERIALPROPERTYLIST_H_

#include <geometry/dataobjects/MaterialProperty.h>

#include <TObject.h>
#include <TList.h>
#include <TTree.h>
#include <string>

namespace Belle2 {

  /**
   * The MaterialPropertyList class.
   *
   * Stores the various properties of a material.
   */
  class MaterialPropertyList: public TList {

  public:

    /**
     * Default constructor for ROOT.
     */
    MaterialPropertyList();

    /**
     * Adds a new material property to the list.
     *
     * @param name The name of the property (e.g. RINDEX, ABSLENGTH, FASTCOMPONENT, SLOWCOMPONENT) which should be added to the list.
     * @return Returns a reference to the added property.
     */
    MaterialProperty& addProperty(const std::string& name);


  protected:

  private:


    /** Class definition required for the creation of the ROOT dictionary. */
    ClassDef(MaterialPropertyList, 1);

  };

}

#endif /* MATERIALPROPERTYLIST_H_ */
