/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/RelationContainer.h>

#include <framework/dataobjects/RelationElement.h>

using namespace std;
using namespace Belle2;

RelationContainer::RelationContainer():
  // Deliberately small initial capacity (see also the StoreEntry struct):
  // the array grows on demand, but Clear()/Delete() cost scales with the capacity.
  m_elements(RelationElement::Class(), 10),
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
  m_fromDurability = m_toDurability = -1;
}
