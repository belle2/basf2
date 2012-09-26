/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONENTRY_H
#define RELATIONENTRY_H

#include <vector>
#include <string>
#include <TObject.h>

namespace Belle2 {

  struct RelationEntry {
    RelationEntry(TObject* obj, double w = 1): object(obj), weight(w) {}
    TObject* object;
    double   weight;
  };

  template <class T> class RelationVector: public std::vector<RelationEntry> {
  public:
    T*     object(int i) const {return static_cast<T*>(std::vector<RelationEntry>::operator [](i).object);}
    T*     operator[](int i) const {return object(i);}
    double weight(int i) const {return static_cast<T*>(std::vector<RelationEntry>::operator [](i).weight);}
  };
}

#endif /* RELATIONENTRY_H */
