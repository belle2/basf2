/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <framework/pcore/RootMergeable.h>
#include <framework/pcore/MapMergeable.h> // just for the linkdef file to be happy

#include <tracking/trackFindingVXD/analyzingTools/KeyValBox.h>

#include <TFile.h>
#include <TList.h>
#include <TTree.h>
#include <string>


namespace Belle2 {

  /** To be used in a MapMergeable<...>. */
//   template<class DataType, class TreeType>
  class TreeContainer : public KeyValBox<std::string, RootMergeable<TTree> > {
  public:




    /** for given key a pointer to the value is returned. If key was invalid, a NULL-ptr will be returned */
    KeyValBox<std::string, RootMergeable<TTree> >::Iterator find(const std::string& aKey)
    {
      Iterator foundPos = std::find_if(
                            begin(),
                            end(),
                            [&](const BoxEntry & entry) -> bool
      { return entry.first == aKey; });
      return ((foundPos == end()) ? end() : foundPos);
    }

    /** for clearing the Ttrees */
    void clear()
    {
      for (auto& entry : KeyValBox<std::string, RootMergeable<TTree>>::m_container) {
        entry.second.clear();
      }
    }


    /** push_back for new pair given, wrapped in an insert to be compatible with MapMergeable */
    void insert(const std::pair<std::string, RootMergeable<TTree>>& newPair)
    {
      m_container.push_back(newPair);
    }


  protected:
    //  ClassDef(TreeContainer, 1); /**< Wrap many ttrees into one mergeable container */
  };

}

