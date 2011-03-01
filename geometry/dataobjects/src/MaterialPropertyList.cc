/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/dataobjects/MaterialPropertyList.h>

using namespace std;
using namespace Belle2;


MaterialPropertyList::MaterialPropertyList()
{

}


MaterialProperty& MaterialPropertyList::addProperty(const std::string& name)
{
  MaterialProperty* newProperty = new MaterialProperty(name);
  Add(newProperty);
  return *newProperty;
}


ClassImp(MaterialPropertyList)
