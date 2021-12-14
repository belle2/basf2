/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <map>
#include <string>

namespace Belle2 {
  /** Store indices where a StoreArray has been merged.
   *  Number i corresponds to number of elements of first StoreArray,
   *  i.e. position [i] is where the second StoreArray starts.
   */
  class MergedArrayIndices : public TObject {
  public:

    /** Constructor. */
    explicit MergedArrayIndices() {}

    /** Destructor. */
    ~MergedArrayIndices() {}

    /** Reset the content. */
    void resetIndices() { m_indices.clear(); }

    /** Add a new index. Throws exception if it already exists. */
    void addIndex(std::pair<std::string, int> item);

    /** Set an index, don't check if it is already defined. */
    void setIndex(std::pair<std::string, int> item);

    /** Get all stored indices. */
    std::map<std::string, int> getIndices() const { return m_indices; }

    /** Get index for a given name. Returns -1 if not found. */
    int getIndex(std::string name) const;

  private:
    std::map<std::string, int> m_indices; /**< Index of position where second StoreArray starts. */

    ClassDef(MergedArrayIndices, 1); /**< Store indices of merged arrays. */
  }; //class
} // namespace Belle2
