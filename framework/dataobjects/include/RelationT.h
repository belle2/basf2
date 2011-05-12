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

  /** This storable class holds information about the relation of stored information.
   *
   *  You may use this class for 1:1 relations or for 1:many relations.
   *  Many:many relations usually don't make sense.
   *  The structure is in such way, that the 'from' side always points only to a single entity,
   *  while the 'to' side may point to various entities stored within one collection.
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


    /** Getter for StoreAccessor Information of 'from' side.
     */
    std::pair<std::string, Belle2::DataStore::EDurability> getFromAccessorInfo() {
      return (std::make_pair(m_fromName, m_fromDurability));
    }

    /** Getter for StoreAccessor Information of 'to' side.
     */
    std::pair<std::string, Belle2::DataStore::EDurability> getToAccessorInfo() {
      return (std::make_pair(m_toName, m_toDurability));
    }

    /** Getter for 'from' side index.
     */
    T getFromIndex() {
      return(m_index);
    }

    /** Getter for 'to' side index.
     *
     *  The 'to' side can contain more than one index.
     *  This function simply returns the first one.
     */
    T getToIndex() {
      return (*(m_indices.begin()));
    }

    /** Getter for 'to' side index.
     *
     *  Returning the complete list of indices.
     */
    std::list<T> getToIndices() {
      return (m_indices);
    }

    /** Getter for weight.
     */
    float getWeight() {
      return *(m_weight.begin());
    }

    /** Getter for weight list.
     */
    std::list<float> getWeights() {
      return m_weight;
    }

    /** Setter for the 'from' side index.
     */
    void setFromIndex(const int& fromIndex) {
      m_index = static_cast<T>(fromIndex);
    }

    /** Setter for the 'to' side index.
     *
     *  This setter replaces all the indices with just a single number.
     */
    void setToIndex(const int& toIndex) {
      m_indices.clear();
      m_indices.push_back(static_cast<T>(toIndex));
    }

    /** Setter for name of 'from' side StoreAccessor.
     */
    void setFromName(const std::string& fromName) {
      m_fromName = fromName;
    }

    /** Setter for name of 'from' side StoreAccessor.
     */
    void setToName(const std::string& toName) {
      m_toName = toName;
    }

    /** Destructor. */
    ~RelationT() {}

  private:
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
