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
#include <vector>

#include <vtx/dbobjects/VTXClusterOffsetPar.h>


namespace Belle2 {


  /** The class for VTX cluster shape classifier payload
   */

  class VTXClusterShapeClassifierPar: public TObject {
  public:
    /** Default constructor */
    VTXClusterShapeClassifierPar() {}

    /** Add shape likelyhood*/
    void addShapeLikelyhood(int shape_index, float likelyhood) {m_shape_likelyhoods[shape_index] = likelyhood;}

    /** Return shape likelyhood map  */
    const std::map<int, float>& getShapeLikelyhoodMap() const { return m_shape_likelyhoods; }

    /**Returns position offset if available, otherwise returns nullptr */
    const VTXClusterOffsetPar* getOffset(int shape_index, float eta) const
    {
      if (m_offsets.find(shape_index) == m_offsets.end()) {
        return nullptr;
      }
      auto eta_index = getEtaIndex(shape_index, eta);
      return &m_offsets.at(shape_index)[eta_index];
    }

    /** Add shape for position correction*/
    void addShape(int shape_index)
    {
      m_percentiles[shape_index] = std::vector<float>();
      m_likelyhoods[shape_index] = std::vector<float>();
      m_offsets[shape_index] = std::vector<VTXClusterOffsetPar>();
    }

    /** Add eta percentile to shape for position correction*/
    void addEtaPercentile(int shape_index, float percentile) {m_percentiles[shape_index].push_back(percentile);}

    /** Add eta likelyhood to shape for position correction*/
    void addEtaLikelyhood(int shape_index, float likelyhood) {m_likelyhoods[shape_index].push_back(likelyhood);}

    /** Add offset to shape for position correction */
    void addEtaOffset(int shape_index, VTXClusterOffsetPar& offset) { m_offsets[shape_index].push_back(offset);}

    /** Get eta index for position correction */
    unsigned int getEtaIndex(int shape_index, float eta) const
    {
      auto etaPercentiles = m_percentiles.at(shape_index);
      for (int i = etaPercentiles.size() - 1; i >= 0; --i) {
        if (eta >= etaPercentiles[i])
          return i;
      }
      return 0;
    }

    /** Return percentiles map for position correction */
    const std::map<int, std::vector<float>>& getPercentilesMap() const { return m_percentiles; }

    /** Return likelyhood map for position correction */
    const std::map<int, std::vector<float>>& getLikelyhoodMap() const { return m_likelyhoods; }

    /** Return offset map for position correction */
    const std::map<int, std::vector<VTXClusterOffsetPar> >& getOffsetMap() const { return m_offsets; }

  private:
    /** Map of position offsets (corrections)  */
    std::map<int, std::vector<VTXClusterOffsetPar> > m_offsets;
    /** Map of percentiles  */
    std::map<int, std::vector<float>>  m_percentiles;
    /** Map of likelyhoods  */
    std::map<int, std::vector<float>>  m_likelyhoods;
    /** Map of shape likelyhoods */
    std::map<int, float> m_shape_likelyhoods;

    ClassDef(VTXClusterShapeClassifierPar, 2);   /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
