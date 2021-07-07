/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef SVD_DATAOBJECTS_SVDOCCUPANCYEVENT_H
#define SVD_DATAOBJECTS_SVDOCCUPANCYEVENT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** Class SVDOccupancyEvent:
   * SVDCluster data container for background studies.
   *
   * This class holds cluster data from background reconstruction, and adds several
   * quantities for easy analysis of background studies. In particular, the class
   * is self-contained in that no relations or sensor data lookup is needed for
   * background analysis.
   */
  class SVDOccupancyEvent : public RelationsObject  {

  public:

    /** default constructor for ROOT */
    SVDOccupancyEvent(): m_layer(0), m_ladder(0), m_sensor(0),
      m_time(0.0), m_isU(true), m_pos(0.0), m_size(0), m_charge(0.0),
      m_snr(0), m_wAcceptance(0.0), m_occupancy(0.0), m_occupancy_APVcycle(0.0)
    {}

    /** Standard constructor
     * @param layer Layer number
     * @param ladder Ladder number
     * @param sensor Sensor number
     * @param time Time of hit
     * @param isU True for a u-cluster
     * @param pos Local coordinate of the hit
     * @param size Cluster size
     * @param charge Cluster charge in ADU
     * @param snr Cluster signal-to-noise ratio
     * @param wAcceptance Probability of hit acceptance as being signal
     * @param occupancy Contribution to occupancy by the current hit
     * @param occupancy_APVcycle Contribution to occupancy for APV cycle time (31.44 ns)
     */
    SVDOccupancyEvent(unsigned short layer, unsigned short ladder, unsigned short sensor,
                      float time, bool isU, float pos, unsigned short size, float charge, float snr,
                      float wAcceptance, float occupancy, float occupancy_APVcycle):
      m_layer(layer), m_ladder(ladder), m_sensor(sensor), m_time(time), m_isU(isU), m_pos(pos),
      m_size(size), m_charge(charge), m_snr(snr),  m_wAcceptance(wAcceptance),
      m_occupancy(occupancy), m_occupancy_APVcycle(occupancy_APVcycle)
    {}

    unsigned short m_layer; /**< layer number */
    unsigned short m_ladder;  /**< ladder number */
    unsigned short m_sensor; /**< sensor number */
    float m_time;   /**< time of particle crossing NOT FILLED currently */
    bool m_isU;      /**< true if this is a u-cluster */
    float m_pos;      /**< local coordinate of cluster */
    unsigned short m_size;   /**< Cluster size */
    float m_charge;  /**< cluster charge */
    float m_snr;    /**< cluster S/N ratio */
    float m_wAcceptance;   /**< acceptance rate */
    float m_occupancy;   /**< contribution to occupancy */
    float m_occupancy_APVcycle; /**< contribution to cycle occupancy */

    ClassDef(SVDOccupancyEvent, 2)
  };
} // end namespace Belle2

#endif
