/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef GLOBALNAMES_H
#define GLOBALNAMES_H

#include <string>



namespace Belle2 {

  /** bundles filter methods using 2 hits.  */
  class GlobalNames {
  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless reset() is called at least once */
    GlobalNames();

    std::string nameAngles3D;
    std::string nameAnglesRZ;
    std::string nameAnglesXY;
    std::string nameDistance3D;
    std::string nameDistanceXY;
    std::string nameDistanceZ;
    std::string nameHelixFit;
    std::string nameSlopeRZ;
    std::string nameDeltaSlopeRZ;
    std::string namePT;
    std::string nameDeltapT;
    std::string nameNormedDistance3D;
    std::string nameDistance2IP;
    std::string nameDeltaDistance2IP;

    std::string nameAnglesHighOccupancy3D;
    std::string nameAnglesHighOccupancyXY;
    std::string nameAnglesHighOccupancyRZ;
    std::string nameDistanceHighOccupancy2IP;
    std::string nameDeltaSlopeHighOccupancyRZ;
    std::string namePTHighOccupancy;
    std::string nameHelixHighOccupancyFit;
    std::string nameDeltapTHighOccupancy;
    std::string nameDeltaDistanceHighOccupancy2IP;

    std::string m_nameAngles3D; /**< string name of filter a3D */
    std::string m_nameAnglesRZ; /**< string name of filter aRZ */
    std::string m_nameAnglesXY; /**< string name of filter aXY */
    std::string m_nameDistance3D; /**< string name of filter d3D */
    std::string m_nameDistanceXY; /**< string name of filter dXY */
    std::string m_nameDistanceZ; /**< string name of filter dZ */
    std::string m_nameSlopeRZ; /**< string name of filter slopeRZ */
    std::string m_nameDeltaSlopeRZ; /**< string name of filter dslopeRZ */
    std::string m_namePT; /**< string name of filter pT */
    std::string m_nameDeltapT; /**< string name of filter dPt */
    std::string m_nameNormedDistance3D; /**< string name of filter nd3D */
    std::string m_nameHelixFit;  /**< string name of filter hFit */
    std::string m_nameDistance2IP; /**< string name of filter d2IP */
    std::string m_nameDeltaDistance2IP; /**< string name of filter dd2IP */

    std::string m_nameAnglesHighOccupancy3D; /**< string name of filter a3D high occupancy */
    std::string m_nameAnglesHighOccupancyXY;  /**< string name of filter aXY high occupancy */
    std::string m_nameAnglesHighOccupancyRZ; /**< string name of filter aRZ high occupancy */
    std::string m_nameDistanceHighOccupancy2IP; /**< string name of filter d2IP high occupancy */
    std::string m_nameDeltaSlopeHighOccupancyRZ;  /**< string name of filter dslopeRZ high occupancy */
    std::string m_namePTHighOccupancy;  /**< string name of filter pT high occupancy */
    std::string m_nameHelixHighOccupancyFit;  /**< string name of filter hFit high occupancy */
    std::string m_nameDeltapTHighOccupancy; /**< string name of filter dPt high occupancy */
    std::string m_nameDeltaDistanceHighOccupancy2IP; /**< string name of filter dd2IP high occupancy */
  }; //end class GlobalNames
} //end namespace Belle2

#endif //GLOBALNAMES


