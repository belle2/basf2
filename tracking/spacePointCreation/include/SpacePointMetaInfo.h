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



    /** the typical constructor to be used.
     *
     * expects a vector of names for storeArrays.
     * */
    SpacePointMetaInfo(std::vector<std::string>& names):
      m_names4StoreArrays(names) {}



    /** to add a single name, use this setter. */
    void addName(std::string aName) { m_names4StoreArrays.push_back(aName); }



    /** to add several names at once, use this setter. */
    void addNames(std::vector<std::string>& moreNames) {
      m_names4StoreArrays.insert(m_names4StoreArrays.end(), moreNames.begin(), moreNames.end());
    }



    /** to get the name of a storeArray to a given index. */
    std::string getName(unsigned index) const { return m_names4StoreArrays.at(index); }



  protected:


    /** Stores all storeArray-names relevant for relations to Clusters.
     *
     * Since the names of StoreArrays for XYZClusters are set in RunTime,
     * they have to be stored to be able to retrieve the relations to the original Clusters again.
     */
    std::vector<std::string> m_names4StoreArrays;


    ClassDef(SpacePointMetaInfo, 0) // last member added: m_names4StoreArrays;
  };
}
