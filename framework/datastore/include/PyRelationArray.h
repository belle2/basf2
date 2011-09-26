#ifndef PYRELATIONARRAY_H
#define PYRELATIONARRAY_H

#include <framework/datastore/RelationArray.h>

#include "TObject.h"

#include <string>

namespace Belle2 {
  /** a python wrapper for RelationArray.
   *
   */
  class PyRelationArray : public TObject {
  public:
    /** Constructor
    * @param name Name of the relation array
    * @param durability 0: event, 1: run, 2: persistent
    */
    PyRelationArray(const std::string &name, int durability = 0):
        TObject(),
        m_relations(name, DataStore::EDurability(durability)) { }

    /** returns number of relations */
    int getEntries() const { return m_relations.getEntries(); }

    /** returns the RelationElement at index i. */
    const RelationElement& operator [](int i) const {return m_relations[i];}

  private:
    RelationArray m_relations; /**< wrapped object */

    ClassDef(PyRelationArray, 0);
  };
}
#endif
