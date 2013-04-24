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

    std::string nameAngles3D;/**< string name of filter a3D */
    std::string nameAnglesRZ; /**< string name of filter aRZ */
    std::string nameAnglesXY; /**< string name of filter aXY */
    std::string nameDistance3D; /**< string name of filter d3D */
    std::string nameDistanceXY; /**< string name of filter dXY */
    std::string nameDistanceZ; /**< string name of filter dZ */
    std::string nameHelixFit; /**< string name of filter hFit */
    std::string nameSlopeRZ; /**< string name of filter slopeRZ */
    std::string nameDeltaSlopeRZ; /**< string name of filter dslopeRZ */
    std::string namePT; /**< string name of filter pT */
    std::string nameDeltapT; /**< string name of filter dPt */
    std::string nameNormedDistance3D; /**< string name of filter nd3D */
    std::string nameDistance2IP; /**< string name of filter d2IP */
    std::string nameDeltaDistance2IP; /**< string name of filter dd2IP */

    std::string nameAnglesHighOccupancy3D; /**< string name of filter a3D high occupancy */
    std::string nameAnglesHighOccupancyXY;  /**< string name of filter aXY high occupancy */
    std::string nameAnglesHighOccupancyRZ; /**< string name of filter aRZ high occupancy */
    std::string nameDistanceHighOccupancy2IP; /**< string name of filter d2IP high occupancy */
    std::string nameDeltaSlopeHighOccupancyRZ; /**< string name of filter dslopeRZ high occupancy */
    std::string namePTHighOccupancy; /**< string name of filter pT high occupancy */
    std::string nameHelixHighOccupancyFit; /**< string name of filter hFit high occupancy */
    std::string nameDeltapTHighOccupancy; /**< string name of filter dPt high occupancy */
    std::string nameDeltaDistanceHighOccupancy2IP; /**< string name of filter dd2IP high occupancy */

  }; //end class GlobalNames
} //end namespace Belle2

#endif //GLOBALNAMES


