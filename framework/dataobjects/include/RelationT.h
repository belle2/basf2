/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RELATIONT_H
#define RELATIONT_H

#include <TObject.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreAccessorBase.h>
#include <framework/datastore/DataStore.h>

#include <string>
#include <list>


namespace Belle2 {

  /** Use the constructor to create a connection between two objects, that are stored in the DataStore.
   *
   *  @author <a href=mailto:"martin.heck@kit.edu?subject=RelationT">Martin Heck</a>
   */
  template <class T>
  class RelationT : public TObject {
  public:

    /** Constructor for I/O. */
    RelationT()
        : m_weight(1.0) {}

    /** Constuctor to create the actual relation - one:one.
     *
     * The constructor has 'from' and 'to' sides, but this constructor is actually symmetric.
     *
     * @param from      StoreAccessor object that holds the object of the 'from' side.
     * @param to        StoreAccessor object that holds the object of the 'to' side'.
     * @param fromIndex If the object the Relation shall point to is not the full object hold by the StoreAccessor,
     *                  you can add an index here.
     * @param toIndex   Same as fromIndex for the 'to' side.
     * @param weight    Weight of the RelationT. Sometimes you might want to use this number to encode other information.
     */
    RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
              const int& fromIndex = -1, const int& toIndex = -1,
              const float& weight = 1.0);

    /** Constuctor to create the actual relation - one:many with single weight.
     *
     * @param from      StoreAccessor object that holds the object of the 'from' side.
     * @param to        StoreAccessor object that holds the object of the 'to' side'.
     * @param fromIndex If the object the Relation shall point to is not the full object hold by the StoreAccessor,
     *                  you can add an index here.
     * @param toIndices Same as fromIndex for the 'to' side, but in this case you don't submit a single index, but a list of indices.
     * @param weight    Weight of the RelationT. This constructor takes a single weight for the whole list.
     */
    RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
              const int& fromIndex, std::list<int> toIndices,
              const float& weight = 1.0);

    /** Constructor to create the actual relation - one:many with individual weights.
     *
     * @param from      StoreAccessor object that holds the object of the 'from' side.
     * @param to        StoreAccessor object that holds the object of the 'to' side'.
     * @param fromIndex If the object the Relation shall point to is not the full object hold by the StoreAccessor,
     *                  you can add an index here.
     * @param toIndices In this case pairs of position indices and weights are submitted for the creation of the relation.
     */
    RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
              const int& fromIndex, std::list<std::pair<int, float> > toIndices);


    /** Destructor. */
    ~RelationT() {}

    /** Setter for from.
     *
     * @param from Object for "from" end of relation.
     */
    /*    void setFrom(TObject* from) {
          m_from = from;
        }
    */
    /** Setter for to.
     *
     * @param to Object for "to" end of the relation.
     */
    /*    void setTo(TObject* to) {
          m_to = to;
        }*/

    /** Getter for from.
     *
     * @return Object of "from" end of the relation.
     */
    /*    TObject* getFrom() {
          return m_from.GetObject();
        }*/

    /** Getter for to.
     *
     *  @return Object of "to" end of the relation.
     */
    /*    TObject* getTo() {
          return m_to.GetObject();
        }*/

    /** Setter for weight.
     *
     * @param weight value for the weight of the relation.
     */
    /*    void setWeight(const float& weight) {
          m_weight = weight;
        }*/

    /** Getter for weight.
     *
     * @return Weight of relation.
     */
    /*    float getWeight() {
          return m_weight;
        }*/


  private:
    /** First end of RelationT. */
//    TObject* m_from;

    /** Second end of RelationT. */
//    TObject** m_to;

    /** Collection Name of "from" end of Relation.
     */
    std::string m_fromName;

    /** Collection EDurability of 'from' end of Relation.
     */
    Belle2::DataStore::EDurability m_fromDurability;

    /** Index for "from" end of the Relation.
     */
    T m_index;

    /** Collection Name of "to" end of Relation.
     */
    std::string m_toName;

    /** Collection EDurability of 'to' end of Relation.
     */
    Belle2::DataStore::EDurability m_toDurability;

    /** Indices for "to" end of Relation.
     */
    std::list<T> m_indices;

    /** Weight of the RelationT.
     */
    std::list<float> m_weight;

    /** ROOT Macro to make EventMetaData a ROOT class.*/
    ClassDef(RelationT, 1);
  }; //class
} // namespace Belle2
//-------------------------------------Implementation of template functions--------

template <class T>
Belle2::RelationT<T>::RelationT(Belle2::StoreAccessorBase& from, Belle2::StoreAccessorBase& to,
                                const int& fromIndex, const int& toIndex,
                                const float& weight)
{
  m_fromName       = from.getAccessorParams().first;
  m_fromDurability = from.getAccessorParams().second;

  m_toName       = to.getAccessorParams().first;
  m_toDurability = to.getAccessorParams().second;

  m_index = static_cast<T>(fromIndex);

  m_indices.push_back(static_cast<T>(toIndex));

  m_weight.push_back(weight);
}


template <class T>
Belle2::RelationT<T>::RelationT(Belle2::StoreAccessorBase& from, Belle2::StoreAccessorBase& to,
                                const int& fromIndex, std::list<int> toIndices,
                                const float& weight)
{
  m_fromName       = from.getAccessorParams().first;
  m_fromDurability = from.getAccessorParams().second;

  m_toName       = to.getAccessorParams().first;
  m_toDurability = to.getAccessorParams().second;

  m_index = fromIndex;

  for (std::list<int>::iterator iter = toIndices.begin(); iter != toIndices.end(); iter++) {
    m_indices.push_back(static_cast<T>((*iter)));
  }

  m_weight.push_back(weight);
}

template <class T>
Belle2::RelationT<T>::RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
                                const int& fromIndex, std::list<std::pair<int, float> > toIndices)
{
  m_fromName       = from.getAccessorParams().first;
  m_fromDurability = from.getAccessorParams().second;

  m_toName       = to.getAccessorParams().first;
  m_toDurability = to.getAccessorParams().second;

  m_index = fromIndex;

  for (std::list<std::pair<int, float> >::iterator iter = toIndices.begin(); iter != toIndices.end(); iter++) {
    m_indices.push_back(static_cast<T>((*iter).first));
    m_weight.push_back((*iter).second);
  }
}


#endif // RELATIONT
