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
#include <algorithm>

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
    PXDClusterOffsetPar& getOffset(int shape_index, int eta_index)   { return m_offsets[shape_index][eta_index];}

    /** Returns True if there are valid position corrections available */
    bool hasOffset(int shape_index, unsigned int eta_index) const
    {
      auto it = m_offsets.find(shape_index);
      if (it == m_offsets.end()) {
        return false;
      } else {
        auto offset_vector = it->second;
        if (eta_index >= offset_vector.size())
          return false;
      }
      return true;
    }

    /** Add shape*/
    void addShape(int shape_index)
    {
      m_percentiles[shape_index] = std::vector<float>();
      m_likelyhoods[shape_index] = std::vector<float>();
      m_offsets[shape_index] = std::vector<PXDClusterOffsetPar>();
    }

    /** Add shape likelyhood*/
    void addShapeLikelyhood(int shape_index, float likelyhood) {m_shape_likelyhoods[shape_index] = likelyhood;}

    /** Add percentile*/
    void addPercentile(int shape_index, float percentile) {m_percentiles[shape_index].push_back(percentile);}

    /** Add likelyhood*/
    void addLikelyhood(int shape_index, float likelyhood) {m_likelyhoods[shape_index].push_back(likelyhood);}

    /** Add offset */
    void addOffset(int shape_index, PXDClusterOffsetPar& offset) { m_offsets[shape_index].push_back(offset);}

    /** Return percentiles map  */
    const std::map<int, std::vector<float>>& getPercentilesMap() const { return m_percentiles; }

    /** Return likelyhood map  */
    const std::map<int, std::vector<float>>& getLikelyhoodMap() const { return m_likelyhoods; }

    /** Return offset map  */
    const std::map<int, std::vector<PXDClusterOffsetPar> >& getOffsetMap() const { return m_offsets; }

    /** Return shape likelyhood map  */
    const std::map<int, float>& getShapeLikelyhoodMap() const { return m_shape_likelyhoods; }



  private:
    /** Map of position offsets in u-v  */
    std::map<int, std::vector<PXDClusterOffsetPar> > m_offsets;
    /** Map of cluster shape + eta likelyhoods */
    std::map<int, std::vector<float> > m_likelyhoods;
    /** Map of percentiles  */
    std::map<int, std::vector<float>>  m_percentiles;
    /** Map of shape only likelyhoods */
    std::map<int, float> m_shape_likelyhoods;

    ClassDef(PXDClusterShapeClassifierPar, 1);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
