/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/RelationArray.h>

using namespace std;
using namespace Belle2;

ClassImp(RelationArray)


RelationArray::RelationArray(TObject* from, TClonesArray* to, std::list<int>& indexList, float weight)
{
  m_from = from;
  m_weight.push_back(weight);
  for (list<int>::iterator iter = indexList.begin(), end = indexList.end(); iter != end; ++iter) {
    m_to.Add(to->At(*iter));
  }
}

RelationArray::RelationArray(TObject* from, TClonesArray* to, std::list<std::pair<int, float> > indexWeightList)
{

}
