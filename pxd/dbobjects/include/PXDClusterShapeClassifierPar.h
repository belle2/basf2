/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <map>
#include <vector>
#include <utility>

#include <pxd/dbobjects/PXDClusterOffsetPar.h>

namespace Belle2 {


  /** The class for PXD cluster shape classifier payload
   */

  class PXDClusterShapeClassifierPar: public TObject {
  public:
    /** Default constructor */
    PXDClusterShapeClassifierPar() {}
    /** Destructor */
    ~ PXDClusterShapeClassifierPar() {}

    /**Returns offsets */
    PXDClusterOffsetPar& getOffset(int shape_index, int feature_index)   { return m_offsets[shape_index][feature_index];}

    /** Returns True if there are valid position corrections available */
    bool hasOffset(int shape_index) const
    {
      if (m_offsets.find(shape_index) == m_offsets.end())
        return false;
      return true;
    }

    /** Set array with percentiles for shape index */
    void setPercentiles(int index, const std::vector<float>& percentiles) {m_percentiles[index] = percentiles;}

    /** Return array with percentiles map  */
    const std::map<int, std::vector<float>>& getPercentilesMap() const { return m_percentiles; }

    /** Set offsets */
    void setOffsetMap(int shape_index, const std::vector<PXDClusterOffsetPar>& offsets) { m_offsets[shape_index] = offsets;}

  private:
    /** Map of position offsets in u-v  */
    std::map<int, std::vector<PXDClusterOffsetPar> > m_offsets;
    /** Map of cluster shape likelyhood, normalized to training data */
    std::map<int, std::vector<float> > m_probs;
    /** Map of percentiles  */
    std::map<int, std::vector<float>>  m_percentiles;

    ClassDef(PXDClusterShapeClassifierPar, 1);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
