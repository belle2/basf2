/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef PXD_DATAOBJECTS_PXDOCCUPANCYEVENT_H
#define PXD_DATAOBJECTS_PXDOCCUPANCYEVENT_H

#include <framework/datastore/RelationsObject.h>
#include <algorithm>

namespace Belle2 {
  /** Class PXDOccupancyEvent:
   * PXDCluster data container for background studies.
   *
   * This class holds cluster data from background reconstruction, and adds several
   * quantities for easy analysis of background studies. In particular, the class
   * is self-contained in that no relations or sensor data lookup is needed for
   * background analysis.
   */
  class PXDOccupancyEvent : public RelationsObject  {

  public:

    /** default constructor for ROOT */
    PXDOccupancyEvent(): m_layer(0), m_ladder(0), m_sensor(0),
      m_posU(0.0), m_posV(0.0), m_size(0), m_charge(0.0),
      m_occupancy(0.0)
    {}

    /** Standard constructor
     * @param layer Layer number
     * @param ladder Ladder number
     * @param sensor Sensor number
     * @param posU Local coordinate of the hit
     * @param posV Local coordinate of the hit
     * @param size Cluster size
     * @param charge Cluster charge in ADU
     * @param occupancy Contribution to occupancy by the current hit
     */
    PXDOccupancyEvent(unsigned short layer, unsigned short ladder, unsigned short sensor,
                      float posU, float posV, unsigned short size, float charge,
                      float occupancy):
      m_layer(layer), m_ladder(ladder), m_sensor(sensor),  m_posU(posU), m_posV(posV),
      m_size(size), m_charge(charge), m_occupancy(occupancy)
    {}

    unsigned short m_layer; /**< layer number */
    unsigned short m_ladder;  /**< ladder number */
    unsigned short m_sensor; /**< sensor number */
    float m_posU;      /**< local coordinate of cluster */
    float m_posV;      /**< local coordinate of cluster */
    unsigned short m_size;   /**< Cluster size */
    float m_charge;  /**< cluster charge */
    float m_occupancy;   /**< contribution to occupancy */

    ClassDef(PXDOccupancyEvent, 2)
  };
} // end namespace Belle2

#endif
