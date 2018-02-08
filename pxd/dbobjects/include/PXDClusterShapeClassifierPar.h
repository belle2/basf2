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

#include <framework/logging/Logger.h>

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
    const PXDClusterOffsetPar& getOffset(int shape_index, float eta) const
    {
      auto eta_index = getEtaIndex(shape_index, eta);
      return m_offsets.at(shape_index)[eta_index];
    }

    /** Returns True if there are valid position corrections available */
    bool hasOffset(int shape_index, float eta) const
    {
      if (m_offsets.find(shape_index) == m_offsets.end()) {
        return false;
      }
      auto offset_vector = m_offsets.at(shape_index);
      auto eta_index = getEtaIndex(shape_index, eta);
      if (eta_index >= offset_vector.size()) {
        B2WARNING("Invalid eta for calibrated shape index " << shape_index);
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

    /** Add eta percentile to shape*/
    void addEtaPercentile(int shape_index, float percentile) {m_percentiles[shape_index].push_back(percentile);}

    /** Add eta likelyhood to shape*/
    void addEtaLikelyhood(int shape_index, float likelyhood) {m_likelyhoods[shape_index].push_back(likelyhood);}

    /** Add offset to shape  */
    void addEtaOffset(int shape_index, PXDClusterOffsetPar& offset) { m_offsets[shape_index].push_back(offset);}

    /** Get eta index from shape and eta */
    unsigned int getEtaIndex(int shape_index, float eta) const
    {
      auto etaPercentiles = m_percentiles.at(shape_index);
      for (std::size_t i = 0; i != etaPercentiles.size(); ++i) {
        if (eta <= etaPercentiles[i])
          return i;
      }
      return etaPercentiles.size() - 1;
    }

    /** Return percentiles map  */
    const std::map<int, std::vector<float>>& getPercentilesMap() const { return m_percentiles; }

    /** Return likelyhood map  */
    const std::map<int, std::vector<float>>& getLikelyhoodMap() const { return m_likelyhoods; }

    /** Return offset map  */
    const std::map<int, std::vector<PXDClusterOffsetPar> >& getOffsetMap() const { return m_offsets; }

    /** Return shape likelyhood map  */
    const std::map<int, float>& getShapeLikelyhoodMap() const { return m_shape_likelyhoods; }

  private:
    /** Map of position offsets (corrections)  */
    std::map<int, std::vector<PXDClusterOffsetPar> > m_offsets;
    /** Map of percentiles  */
    std::map<int, std::vector<float>>  m_percentiles;
    /** Map of likelyhoods  */
    std::map<int, std::vector<float>>  m_likelyhoods;
    /** Map of shape likelyhoods */
    std::map<int, float> m_shape_likelyhoods;

    ClassDef(PXDClusterShapeClassifierPar, 1);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
