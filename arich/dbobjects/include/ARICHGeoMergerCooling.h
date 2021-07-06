/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/tessellatedSolidStr.h>
#include <string>

//root
#include <TVector3.h>

namespace Belle2 {

  /**
   * Geometry parameters of Merger Cooling System - version2 (v2).
   * Upgrade of the cooling system have been done after phase 2 (in september - november 2018).
   * This data base object contains information about :
   * Merger cooling bodies.
   * Cooling pipes.
   */
  class ARICHGeoMergerCooling: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoMergerCooling()
    {}

    /**
     * Set material name of merger cooling bodies
     * @param materialName name of merger cooling bodies
     */
    void setMergerCoolingBodiesMaterialName(const std::string& materialName) {m_mergerCoolingBodiesMaterialName = materialName;}

    /**
     * Set vector of structures which holds apexes of the tessellation volumes for (merger cooling bodyes).
     * @param mergerCoolingBodiesInfo vector of structures which holds apexes of the tessellation volumes for (merger cooling bodyes).
     */
    void addMergerCoolingBodiesInfo(tessellatedSolidStr mergerCoolingBodiesStr) {m_mergerCoolingBodiesInfo.push_back(mergerCoolingBodiesStr);}

    /**
     * Set vector of merger cooling body shape id (positionID) for each merger PCB.
     * @param mergerCoolingPositionID vector of merger cooling body shape id (positionID) for each merger PCB.
     */
    void setMergerCoolingPositionID(const std::vector<double>& mergerCoolingPositionID) { m_mergerCoolingPositionID = mergerCoolingPositionID; }

    /**
     * Returns material name of merger cooling bodies
     * @return material name of merger cooling bodies
     */
    const std::string& getMergerCoolingBodiesMaterialName() const { return m_mergerCoolingBodiesMaterialName; }

    /**
     * Returns structur which holds apexes of the tessellation volumes for (merger cooling bodyes).
     * @return structur which holds apexes of the tessellation volumes for (merger cooling bodyes).
     */
    const tessellatedSolidStr getMergerCoolingBodiesInfo(unsigned int iSlot) const { if (iSlot > 12 || iSlot == 0) B2ERROR("ARICHGeoCoolingv2: invalid merger cooling bodies iSlot  number! "); return m_mergerCoolingBodiesInfo[iSlot - 1];}

    /**
     * Returns structur which holds apexes of the tessellation volumes for (merger cooling bodyes).
     * @return structur which holds apexes of the tessellation volumes for (merger cooling bodyes).
     */
    const tessellatedSolidStr getMergerCoolingBodiesInfo_globalMergerID(unsigned int iMergerSlot) const;

    /**
     * Returns vector of merger cooling body shape id (positionID) for each merger PCB.
     * @return vector of merger cooling body shape id (positionID) for each merger PCB.
     */
    const std::vector<double>& getMergerCoolingPositionID() const { return m_mergerCoolingPositionID; }

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Upgraded after phase two cooling system (v2) geometry parameters") const;

    /**
     * Check data consistency of the cooling system (v2) positions
     * In case of failure print the BASF2 ERROR message using B2ASSERT
     */
    void checkMergerCoolingSystemDataConsistency() const;

  private:

    //Merger cooling bodies.
    std::string m_mergerCoolingBodiesMaterialName; /**< Name of the merger cooling bodies material */
    std::vector<double> m_mergerCoolingPositionID; /**< Merger cooling body shape id (positionID) for each merger PCB */
    /**< Vector of structures which holds apexes of the tessellation volumes for (merger cooling bodyes). */
    std::vector<tessellatedSolidStr> m_mergerCoolingBodiesInfo;

    ClassDef(ARICHGeoMergerCooling, 1); /**< ClassDef */

  };

} // end namespace Belle2
