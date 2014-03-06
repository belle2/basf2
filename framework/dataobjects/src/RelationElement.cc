/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/RelationElement.h>

#include <framework/logging/Logger.h>

#include <climits>

using namespace std;
using namespace Belle2;

ClassImp(RelationElement)

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
