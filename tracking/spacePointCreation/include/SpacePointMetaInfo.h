/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// root:
#include <TObject.h>
// stl:
#include <vector>
#include <string>

namespace Belle2 {
  /** The SpacePoint Meta Info class.
   *
   *  This class stores meta data relevant for all spacePoints used.
   */
  class SpacePointMetaInfo: public TObject {
  public:

    /** standard constructor for root IO.
     *
     * If this one is used, use addName(s) to store the Names you want to store.
     */
    SpacePointMetaInfo() {}



    /** destructor */
    ~SpacePointMetaInfo() {}



    /** constructor - expects a vector of names for storeArrays to be stored.
     * */
    SpacePointMetaInfo(std::vector<std::string>& names):
      m_names4StoreArrays(names) {}



    /** to add a single name, use this setter. it returns the position of the added name in its container */
    unsigned int addName(std::string aName) {
      m_names4StoreArrays.push_back(aName);
      return m_names4StoreArrays.size() - 1;
    }



    /** to add several names at once, use this setter. */
    void addNames(std::vector<std::string>& moreNames) {
      m_names4StoreArrays.insert(m_names4StoreArrays.end(), moreNames.begin(), moreNames.end());
    }



    /** to get the name of a storeArray to a given index. */
    std::string getName(unsigned index) const { return m_names4StoreArrays.at(index); }



    /** returns number of names stored in metaInfo */
    unsigned int getnNames() const { return m_names4StoreArrays.size(); }
  protected:


    /** Stores all storeArray-names relevant for relations to Clusters.
     *
     * Since the names of StoreArrays for XYZClusters are set in RunTime,
     * they have to be stored to be able to retrieve the relations to the original Clusters again.
     */
    std::vector<std::string> m_names4StoreArrays;

    /** counts how many times a constructor was called, having metaInfo as member, destructors reduce this counter. */
//  static int m_usageCounter;

    ClassDef(SpacePointMetaInfo, 0) // last member added: m_names4StoreArrays;
  };
}
