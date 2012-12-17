/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMOBJECTNUMBERS_H
#define EKLMOBJECTNUMBERS_H

namespace Belle2 {
  namespace EKLM {

    /**
     * Get layer number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @return Number of layer.
     * @details
     * Number is from 1 to 14*2 = 28.
     */
    int layerNumber(int endcap, int layer);

    /**
     * Get sector number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @return Number of sector.
     * @details
     * Number is from 1 to 4*14*2 = 112.
     */
    int sectorNumber(int endcap, int layer, int sector);

    /**
     * Get plane number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @return Number of plane.
     * @details
     * Number is from 1 to 2*4*14*2 = 224.
     */
    int planeNumber(int endcap, int layer, int sector, int plane);

    /**
     * Get strip number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @param[in] strip  Strip number.
     * @return Number of strip.
     * @details
     * Number is from 1 to 75*2*4*14*2 = 16800.
     */
    int stripNumber(int endcap, int layer, int sector, int plane, int strip);

    /**
     * Get board number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @param[in] board  Board number.
     * @return Number of board.
     * @details
     * Number is from 1 to 5*2*4*14*2 = 1200.
     */
    int boardNumber(int endcap, int layer, int sector, int plane, int board);

    /**
     * Get layer local number.
     * @param[in] gnum Layer global number.
     * @return Local number.
     */
    int layerLocalNumber(int gnum);

    /**
     * Get sector local number.
     * @param[in] gnum Sector global number.
     * @return Local number.
     */
    int sectorLocalNumber(int gnum);

    /**
     * Get plane local number.
     * @param[in] gnum Plane global number.
     * @return Local number.
     */
    int planeLocalNumber(int gnum);

    /**
     * Get strip local number.
     * @param[in] gnum Strip global number.
     * @return Local number.
     */
    int stripLocalNumber(int gnum);

  }
}

#endif

