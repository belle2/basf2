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
#include <eklm/dbobjects/EKLMDataConcentratorLane.h>

namespace Belle2 {

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
    const int* getSectorByLane(EKLMDataConcentratorLane* lane) const;

    /**
     * Get lane identifier by global sector number.
     * @param[in] sector Global sector number.
     * @return Lane identifier, NULL if not found.
     */
    const EKLMDataConcentratorLane* getLaneBySector(int sector) const;

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
    std::map<int, EKLMDataConcentratorLane> m_MapSectorLane;

    /** Lane - sector map. */
    std::map<EKLMDataConcentratorLane, int> m_MapLaneSector;

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMElectronicsMap, 1);

  };

}

#endif

