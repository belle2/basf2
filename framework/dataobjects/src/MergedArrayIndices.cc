/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/MergedArrayIndices.h>
#include <stdexcept>

using namespace Belle2;


void MergedArrayIndices::addIndex(std::pair<std::string, int> item)
{
  if (getIndex(item.first) == -1) {
    m_indices[item.first] = item.second;
  } else {
    throw std::out_of_range(std::string("Key with name ") + item.first + " already exists in MergedArrayIndices.");
  }
}

void MergedArrayIndices::setIndex(std::pair<std::string, int> item)
{
  m_indices[item.first] = item.second;
}

int MergedArrayIndices::getIndex(std::string name) const
{
  return m_indices.find(name) != m_indices.end() ? m_indices.at(name) : -1;
}
