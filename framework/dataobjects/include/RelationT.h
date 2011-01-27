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

    /** Constuctor to create the actual relation.
     *
     * The constructor has 'from' and 'to' sides, but the RelationT is in principle completely symmetric.
     *
     * @param from   first side of the RelationT.
     * @param to     second side of the RelationT.
     * @param weight weight of the RelationT. Sometimes you might want to use this number to encode other information.
     */
    RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
              const int& fromIndex = -1, const int& toIndex = -1,
              const float& weight = 1.0);


    RelationT(StoreAccessorBase& from, StoreAccessorBase& to,
              const int& fromIndex, std::list<int> toIndices,
              std::list<float> weight = std::list<float>());


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

    /** Accessor Params of "from" end of Relation.
     */
    std::pair<std::string, Belle2::DataStore::EDurability> m_fromAccessorParams;

    /** Index for "from" end of the Relation.
     */
    T m_index;

    /** Accessor Params of "to" end of Relation.
     */
    std::pair<std::string, Belle2::DataStore::EDurability> m_toAccessorParams;

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
  m_fromAccessorParams = from.getAccessorParams();
  m_toAccessorParams   = to.getAccessorParams();

  m_index = static_cast<T>(fromIndex);

  m_indices.push_back(static_cast<T>(toIndex));

  m_weight.push_back(weight);
}


template <class T>
Belle2::RelationT<T>::RelationT(Belle2::StoreAccessorBase& from, Belle2::StoreAccessorBase& to,
                                const int& fromIndex, std::list<int> toIndices,
                                std::list<float> weight)
{
  m_fromAccessorParams = from.getAccessorParams();
  m_toAccessorParams   = to.getAccessorParams();

  m_index = fromIndex;

  for (std::list<int>::iterator iter = toIndices.begin(); iter != toIndices.end(); iter++) {
    m_indices.push_back(static_cast<T>((*iter)));
  }

  for (std::list<int>::iterator iter = toIndices.begin(); iter != toIndices.end(); iter++) {
    m_indices.push_back(static_cast<T>((*iter)));
  }

  m_weight = weight;
}

#endif // RELATIONT
