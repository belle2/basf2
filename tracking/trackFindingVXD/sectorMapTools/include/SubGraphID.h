/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <tracking/dataobjects/FullSecID.h>

#include <string>
#include <vector>
#include <utility> // std::pair, std::move


namespace Belle2 {

  /// stores the ID of a subgraph, which is basically a chain of FullSecID coded as unsigned ints.
  class SubGraphID {
  protected:
    const std::vector<unsigned> m_idChain;
  public:

    /** overloaded '=='-operator */
    bool operator==(const SubGraphID& b) const
    {
      unsigned size = m_idChain.size();
      if (b.m_idChain.size() != size) { return false; }

      for (unsigned i = 0; i < size; i++) {
        if (b.m_idChain[i] != m_idChain[i]) { return false; }
      }

      return true;
    }

    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const SubGraphID& b)  const
    {
      // sorting: if outermost (== first) sector > of b.outermost this > b. If outermost == b.outermost, step is repeated until end of chain length is reached. last straw: longer chain is bigger.
      unsigned size = m_idChain.size() < b.m_idChain.size() ? m_idChain.size() : b.m_idChain.size();
      for (unsigned i = 0 ; i < size; i++) {
        if (m_idChain[i] < b.m_idChain[i]) return true;
        if (m_idChain[i] == b.m_idChain[i]) continue;
        return false;
      }
      return m_idChain.size() < b.m_idChain.size();
    }

    /** constructor, mandatory iDChain musst at least contain one iD. */
    SubGraphID(const std::vector<unsigned>& idChain) : m_idChain(idChain)
    {
      if (m_idChain.empty()) { B2FATAL("SubGraphID-Constructor, given idChain is empty - illegal usage!") }
    }

    /** returns chainLength of this id. */
    unsigned size() const { return m_idChain.size(); }

    /** if both graphs have got the same IDs except the last one, they share a trunk. */
    bool checkSharesTrunk(const SubGraphID& b) const
    {
      unsigned size = m_idChain.size();
      if (b.m_idChain.size() != size) { return false; }

      for (unsigned i = 0; i < size - 1; i++) {
        if (b.m_idChain[i] != m_idChain[i]) { return false; }
      }

      return true;
    }

    /** returns string of entries. */
    std::string print() const
    {
      std::string out;
      for (unsigned iD : m_idChain) {
        out += FullSecID(iD).getFullSecString() + " ";
      }
      return std::move(out);
    }

  };
}

