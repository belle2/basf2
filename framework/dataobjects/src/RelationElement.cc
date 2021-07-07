/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/RelationElement.h>

#include <framework/logging/Logger.h>

#include <climits>

using namespace std;
using namespace Belle2;

RelationElement::RelationElement(index_type from, const std::vector<index_type>& to, const std::vector<weight_type>& weights):
  TObject(),
  m_from(from),
  m_to(to.begin(), to.end()),
  m_weights(weights.begin(), weights.end())
{
  if (to.size() != weights.size()) {
    B2FATAL("Index and weight vector sizes differ!");
  }
}
RelationElement::RelationElement():
  TObject(), m_from(UINT_MAX), m_to(), m_weights()
{

}
