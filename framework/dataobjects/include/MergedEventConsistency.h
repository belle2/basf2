/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <string>
#include <map>

namespace Belle2 {
  /** Mark if we want to merge the DataStore of a combination of events or not.
   *  This object is necessary as usually the decision is made using analysis level
   *  objects, which we do not want to import in the framework package.
   */
  class MergedEventConsistency : public TObject {
  public:

    /** Constructor. */
    explicit MergedEventConsistency() {}

    /** Destructor. */
    ~MergedEventConsistency() {}

    /** Set if we want to merge these events. */
    void setConsistent(bool isConsistent) { m_isConsistentMerge = isConsistent; }

    /** Get if we want to merge these events. */
    int getConsistent() const { return m_isConsistentMerge; }

  private:
    bool m_isConsistentMerge = true; /**< Bool to indicate if events should be merged. */

    ClassDef(MergedEventConsistency, 1); /**< Bool to indicate if events should be merged. */
  }; //class
} // namespace Belle2
