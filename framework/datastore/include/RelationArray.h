/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RELATIONARRAY_H
#define RELATIONARRAY_H

#include <TRef.h>
#include <TRefArray.h>
#include <TObject.h>

namespace Belle2 {

  /*! Use the constructor to create a connection between an object and more than one objects, that are stored in the DataStore.
      \author <a href=mailto:"martin.heck@kit.edu?subject=RelationArray">Martin Heck</a>
  */
  class RelationArray : public TObject {
  public:

    RelationArray()
        : m_weight(1.0) {}

    RelationArray(TObject* from, std::vector<TObject*> to, float weight = 1.0);

    ~RelationArray() {}

  private:
    //! First end of Relation.
    TRef m_from;

    //! Second end of Relation.
    TRefArray m_to;

    /*! Weight of the RelationArray
    */
    float m_weight;

    ClassDef(RelationArray, 1);
  }; //class
} // namespace Belle2
#endif // RELATION
