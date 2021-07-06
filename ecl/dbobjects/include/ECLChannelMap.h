/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace ECL {
    enum {
      ECL_BARREL_SHAPERS_IN_CRATE = 12,
      ECL_FWD_SHAPERS_IN_CRATE    = 10,
      ECL_BKW_SHAPERS_IN_CRATE    = 8,
      ECL_TOTAL_SHAPERS      = 576,
      ECL_FINESSES_IN_COPPER = 2,
      ECL_CHANNELS_IN_SHAPER = 16,
      ECL_CRATES             = 52,
      ECL_BARREL_CRATES      = 36,
      ECL_FWD_CRATES         = 8,
      ECL_BKW_CRATES         = 8,
      ECL_COPPERS            = 26,
      ECL_BARREL_COPPERS     = 18,
      ECL_ENDCAP_COPPERS     = 8,
      ECL_TOTAL_CHANNELS     = 8736,
      ECL_BARREL_CHANNELS    = 6624,
      ECL_FWD_CHANNELS       = 1152,
      ECL_BKW_CHANNELS       = 960
    };
  }

  /**
   * DB object to store correspondence table of type
   * (Crate id, ShaperDSP id, Channel id) <-> (ECL CellID)
   */
  class ECLChannelMap: public TObject {

  public:

    /** Constructor. */
    ECLChannelMap() : m_MappingBAR(), m_MappingFWD(), m_MappingBWD() { };

    /** Get vector of map entries for ECL barrel */
    const std::vector<int>& getMappingBAR() const {return m_MappingBAR;}
    /** Get vector of map entries for ECL forward endcap */
    const std::vector<int>& getMappingFWD() const {return m_MappingFWD;}
    /** Get vector of map entries for ECL backward endcap */
    const std::vector<int>& getMappingBWD() const {return m_MappingBWD;}

    /** Get value for specific (crate, shaper, channel) */
    int get(int crate, int shaper, int channel) const
    {
      using namespace ECL;

      int id;
      if (crate <= ECL_BARREL_CRATES) {
        id = (crate - 1) * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
             + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
        return m_MappingBAR[id];
      }
      crate -= ECL_BARREL_CRATES;

      if (crate <= ECL_FWD_CRATES) {
        id = (crate - 1) * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
             + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
        return m_MappingFWD[id];
      }
      crate -= ECL_FWD_CRATES;

      if (crate <= ECL_BKW_CRATES) {
        id = (crate - 1) * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
             + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
        return m_MappingBWD[id];
      }

      return -1;
    }

    /**
     * Set three vectors of map entries
     * @param mappingBAR Map entries for barrel.
     * @param mappingFWD Map entries for forward endcap.
     * @param mappingBWD Map entries for backward endcap.
     */
    void setMappingVectors(const std::vector<int>& mappingBAR,
                           const std::vector<int>& mappingFWD,
                           const std::vector<int>& mappingBWD)
    {
      using namespace ECL;
      //== Determine correct vector sizes from ECL-related constants
      const unsigned int mappingBAR_size = ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER;
      const unsigned int mappingFWD_size = ECL_FWD_CRATES    * ECL_FWD_SHAPERS_IN_CRATE    * ECL_CHANNELS_IN_SHAPER;
      const unsigned int mappingBWD_size = ECL_BKW_CRATES    * ECL_BKW_SHAPERS_IN_CRATE    * ECL_CHANNELS_IN_SHAPER;
      //== Compare given vector sizes with the correct ones
      if (mappingBAR.size() != mappingBAR_size ||
          mappingFWD.size() != mappingFWD_size ||
          mappingBWD.size() != mappingBWD_size) {
        B2FATAL("ECLChannelMap: wrong sizes for mapping vectors. Got ("
                << mappingBAR.size() << ","
                << mappingFWD.size() << ","
                << mappingBWD.size() << "), "
                << "Expected ("
                << mappingBAR_size << ","
                << mappingFWD_size << ","
                << mappingBWD_size << ")");
      }
      //==
      m_MappingBAR = mappingBAR;
      m_MappingFWD = mappingFWD;
      m_MappingBWD = mappingBWD;
    };

  private:
    std::vector<int> m_MappingBAR; /**< Map entries for ECL barrel */
    std::vector<int> m_MappingFWD; /**< Map entries for ECL forward endcap */
    std::vector<int> m_MappingBWD; /**< Map entries for ECL backward endcap */

    ClassDef(ECLChannelMap, 1); /**< ClassDef */
  };
}

