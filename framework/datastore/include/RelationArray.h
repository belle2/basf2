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

#include <utility>
#include <list>

#include <TRef.h>
#include <TRefArray.h>
#include <TObject.h>
#include <TClonesArray.h>

namespace Belle2 {

  /*! Use the constructor to create a connection between an object and more than one objects, that are stored in the DataStore.
      \author <a href=mailto:"martin.heck@kit.edu?subject=RelationArray">Martin Heck</a>
  */
  class RelationArray : public TObject {
  public:

    /*! Paramter free Constructor for I/O purposes.
    */
    RelationArray()
        : m_weight(1.0) {}

    /*! Constructor with single weight for all relations together.
        \param from      Pointer to the TObject on the "from" side of the RelationArray.
        \param to        Pointer to the TClonesArray, which contains the objects to be pointed to.
        \param indexList list with integers, that define at which positions the objects for the "to"
                         side can be found in the given TClonesArray.
        \param weight    Single weight for all the relations. The default value of 1, ensures, that
                         you don't have to bother with weights, if you don't like to.
    */
    RelationArray(TObject* from, TClonesArray* to, std::list<int>& indexList, float weight = 1.0);

    /*! Constructor with weight for all relations separated.
        \param from      Pointer to the TObject on the "from" side of the RelationArray.
        \param to        Pointer to the TClonesArray, which contains the objects to be pointed to.
        \param indexWeight List list of pairs with integers, that define at which positions the objects for the "to"
                           side can be found in the given TClonesArray, and floats for the weights of the relations.
    */
    RelationArray(TObject* from, TClonesArray* to, std::list<std::pair<int, float> > indexWeightList);

    /*! Destructor.
    */
    ~RelationArray() {}

    /*! Setter for "from" part of the RelationArray.
        \param from This has to be a pointer to a TObject.
    */
    void setFrom(TObject* from) {m_from = from;}

    /*! Setter for "to" part of the RelationArray with single weight for all relations.
        \param
    */

  private:
    //! First end of Relation.
    TRef m_from;

    //! Second end of Relation.
    TRefArray m_to;

    /*! Weight of the RelationArray
    */
    std::vector<float> m_weight;

    ClassDef(RelationArray, 1);
  }; //class
} // namespace Belle2
#endif // RELATION
