/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <datastore/RelationArray.h>

using namespace std;
using namespace Belle2;

ClassImp(RelationArray)

RelationArray::RelationArray(TObject* from, vector<TObject*> to, float weight)
    : m_weight(weight)
{
  m_from = from;
  for (vector<TObject*>::iterator iter = to.begin(), end = to.end(); iter != end; ++iter) {
    m_to.Add(*iter);
  }
}
