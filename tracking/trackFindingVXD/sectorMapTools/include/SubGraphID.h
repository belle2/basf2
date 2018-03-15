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
    std::vector<unsigned> m_idChain; /**< ids of this SubGraphID. */


    /** checks if two given ids are the same sector (while ignoring the sublayerID). */
    bool areTheSameSector(unsigned a, unsigned b) const
    {
      FullSecID idA(a);
      FullSecID idB(b);
      return idA.getVxdID() == idB.getVxdID() and idA.getSecID() == idB.getSecID();
    }

  public:

    /** overloaded '=='-operator */
    bool operator==(const SubGraphID& b) const
    {
      unsigned size = m_idChain.size();
      if (b.size() != size) { return false; }

      for (unsigned i = 0; i < size; i++) {
        if (b.m_idChain[i] != m_idChain[i]) { return false; }
      }

      return true;
    }


    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const SubGraphID& b)  const
    {
      // sorting: if outermost (== first) sector > of b.outermost this > b. If outermost == b.outermost, step is repeated until end of chain length is reached. last straw: longer chain is bigger.
      unsigned size = m_idChain.size() < b.size() ? m_idChain.size() : b.size();
      for (unsigned i = 0 ; i < size; i++) {
        if (m_idChain[i] < b.m_idChain[i]) return true;
        if (m_idChain[i] == b.m_idChain[i]) continue;
        return false;
      }
      return m_idChain.size() < b.size();
    }


    /** constructor, mandatory iDChain musst at least contain one iD. Sectors should be sorted from outer to inner sectors, please take care of that yourself since this is not checked internally. */
    SubGraphID(const std::vector<unsigned>& idChain) : m_idChain(idChain)
    {
      if (m_idChain.empty()) { B2FATAL("SubGraphID-Constructor, given idChain is empty - illegal usage!"); }
    }


    /** returns chainLength of this id. */
    unsigned size() const { return m_idChain.size(); }


    /** returns first entry of IDchain. */
    std::vector<unsigned>::const_iterator begin() const { return m_idChain.begin(); }


    /** returns last entry of IDchain. */
    std::vector<unsigned>::const_iterator end() const { return m_idChain.end(); }


    /** if both graphs have got the same IDs except the last one, they share a trunk. */
    bool checkSharesTrunk(const SubGraphID& b) const
    {
      unsigned size = m_idChain.size();
      if (b.size() != size) { return false; }

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
      return out;
    }


    /** checks if given sectorPack ( >= 1 sector) is part of this, ignores subLayerID. returns number of identical sectors */
    unsigned isElementOf(const SubGraphID& b) const
    { return isElementOf(b.m_idChain); }


    /** checks if given sectorPack ( >= 1 sector) is part of this, ignores subLayerID. returns number of identical sectors */
    unsigned isElementOf(const std::vector<unsigned>& b) const
    {
      if (size() < b.size()) return 0;

      unsigned nIdentical = 0;
      for (unsigned idA : m_idChain) {
        for (unsigned idB : b) {
          if (areTheSameSector(idA, idB)) nIdentical++;
        }
      }
      return nIdentical;
    }


    /** checks if given raw secID is part of this, ignores subLayerID. returns number of identical sectors */
    bool hasElement(unsigned b) const
    {
      for (unsigned idA : m_idChain) {
        if (areTheSameSector(idA, b)) return true;
      }
      return false;
    }


    /** checks if sectors are identical (while ignoring the subLayerID) */
    bool areTheSameSectors(const SubGraphID& b) const
    {
      if (size() != b.size()) return false;
      for (unsigned i = 0; i < size(); i++) {
        if (areTheSameSector(m_idChain[i], b.m_idChain[i]) == false) return false;
      }
      return true;
    }


    /** returns raw IDs of entries being outer friend of sectors on same layer. This function assumes that the ids are sorted from outermost (= first) to innermost (= last) iD. */
    std::vector<unsigned> hasSharedLayer() const
    {
      B2DEBUG(1, "hasSharedLayer-start with iD " << print() << ":");
      std::vector<unsigned> outerOnes;
      for (unsigned i = 0; i < size() - 1; i++) { // i: outer iD i+1: inner iD
        B2DEBUG(1, "found layerIDs for outer/inner: " << FullSecID(m_idChain[i]).getLayerID() << "/" << FullSecID(
                  m_idChain[i + 1]).getLayerID());
        if (FullSecID(m_idChain[i]).getLayerID() == FullSecID(m_idChain[i + 1]).getLayerID()) {
          outerOnes.push_back(m_idChain[i]);
          B2DEBUG(1, "hasSharedLayer good combi found with inner: " << FullSecID(m_idChain[i + 1]).getFullSecString() << ", storing outer: "
                  << FullSecID(m_idChain[i]).getFullSecString());
        }
      }
      return outerOnes;
    }


    /// if newID is element of this (ignoring subLayerID), oldID will be replaced by newID. returns true, if an ID was replaced.
    bool updateID(unsigned newID)
    {
      for (unsigned& secID : m_idChain) {
        if (areTheSameSector(newID, secID) == false) continue;
        FullSecID tempID(secID);
        secID = FullSecID(tempID.getVxdID(), true, tempID.getSecID()).getFullSecID();
        return true;
      }

      return false;
    }


    /// returns SecIDs coded as FullSecIDs:
    std::vector<FullSecID> getFullSecIDs() const
    {
      std::vector<FullSecID> ids;

      for (auto& id : m_idChain) {
        ids.push_back(FullSecID(id));
      }
      return ids;
    }
  }; // class SubGraphID


} // namespace Belle2

namespace std {
  /** customized hash-function for being able to use unordered_map. */
  template <> struct hash<Belle2::SubGraphID> {
    /** Create Hash value from graphID
     * @param graphID : the subgraph ID
     * @return the hash value
     */
    std::size_t operator()(const Belle2::SubGraphID& graphID) const
    {
      std::size_t hashVal = 0;
      for (unsigned iD : graphID) {
        hashVal = std::hash<unsigned>()(iD) ^ hashVal;
        ++hashVal;
      }
      return hashVal;
    }
  };
}

