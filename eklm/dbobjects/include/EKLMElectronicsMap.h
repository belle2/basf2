/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMELECTRONICSMAP_H
#define EKLMELECTRONICSMAP_H

/* C++ headers. */
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMChannelData.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * EKLM data concentrator lane identifier.
     */
    struct DataConcentratorLane {
      int copper;           /**< Copper number - EKLM_ID (7000). */
      int dataConcentrator; /**< Data concentrator number. */
      int lane;             /**< Lane number. */
    };

    /**
     * Class for comparison of DataConcentratorLane.
     */
    class DataConcentratorLaneComparison {

    public:

      /**
       * Comparison function.
       */
      bool operator()(const struct DataConcentratorLane& l1,
                      const struct DataConcentratorLane& l2) const
      {
        if (l1.copper < l2.copper)
          return true;
        else if (l1.copper > l2.copper)
          return false;
        if (l1.dataConcentrator < l2.dataConcentrator)
          return true;
        else if (l1.dataConcentrator > l2.dataConcentrator)
          return false;
        return l1.lane < l2.lane;
      }

    };

  }

  /**
   * Class to store EKLM alignment data in the database.
   */
  class EKLMElectronicsMap : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMElectronicsMap();

    /**
     * Destructor.
     */
    ~EKLMElectronicsMap();

    /**
     * Get global sector number by lane identifier.
     * @param[in] lane Lane identifier.
     * @return Sector number, NULL if not found.
     */
    const int* getSectorByLane(struct EKLM::DataConcentratorLane* lane) const;

    /**
     * Get lane identifier by global sector number.
     * @param[in] sector Global sector number.
     * @return Lane identifier, NULL if not found.
     */
    const struct EKLM::DataConcentratorLane* getLaneBySector(int sector) const;

    /**
     * Add sector-lane pair.
     * @param[in] endcap           Endcap number.
     * @param[in] layer            Layer number.
     * @param[in] sector           Sector number.
     * @param[in] copper           Copper identifier.
     * @param[in] dataConcentrator Data concentrator (= finesse) number.
     * @param[in] lane             Lane number.
     */
    void addSectorLane(int endcap, int layer, int sector,
                       int copper, int dataConcentrator, int lane);

  private:

    /** Sector - lane map. */
    std::map<int, struct EKLM::DataConcentratorLane> m_MapSectorLane;

    /** Lane - sector map. */
    std::map<struct EKLM::DataConcentratorLane, int,
          EKLM::DataConcentratorLaneComparison> m_MapLaneSector;

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMElectronicsMap, 1);

  };

}

#endif

