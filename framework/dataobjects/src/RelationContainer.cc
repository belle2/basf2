/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/RelationContainer.h>

#include <framework/dataobjects/RelationElement.h>

using namespace std;
using namespace Belle2;

ClassImp(RelationContainer);

RelationContainer::RelationContainer():
  m_elements(RelationElement::Class()),
  m_fromName(""), m_fromDurability(-1),
  m_toName(""), m_toDurability(-1), m_modified(true)
{
}

const RelationElement& RelationContainer::getElement(int i) const
{
  return *static_cast<RelationElement*>(m_elements.At(i));
}
void RelationContainer::Clear(Option_t*)
{
  m_elements.Delete();
  m_fromName.clear();
  m_toName.clear();
  m_modified = true;
}
