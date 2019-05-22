/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <klm/dataobjects/KLMElementNumbers.h>

namespace Belle2 {

  /**
   * BKLM channel index.
   */
  class BKLMChannelIndex {

  public:

    /**
     * Constructor.
     */
    BKLMChannelIndex();

    /**
     * Constructor.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    BKLMChannelIndex(int forward, int sector, int layer, int plane, int strip);

    /**
     * Destructor.
     */
    ~BKLMChannelIndex();

    /**
     * Get forward.
     */
    int getForward()
    {
      return m_Forward;
    };

    /**
     * Get sector.
     */
    int getSector()
    {
      return m_Sector;
    };

    /**
     * Get layer.
     */
    int getLayer()
    {
      return m_Layer;
    };

    /**
     * Get plane.
     */
    int getPlane()
    {
      return m_Plane;
    };

    /**
     * Get strip.
     */
    int getStrip()
    {
      return m_Strip;
    };

    /**
     * Get KLM channel number.
     */
    uint16_t getKLMChannelNumber();

    /**
     * First channel.
     */
    BKLMChannelIndex& begin();

    /**
     * Last channel.
     */
    BKLMChannelIndex& end();

    /**
     * Operator ++.
     */
    BKLMChannelIndex& operator++();

    /**
     * Operator ==.
     */
    bool operator==(BKLMChannelIndex& index);

    /**
     * Operator !=.
     */
    bool operator!=(BKLMChannelIndex& index);

    /**
     * Operator *.
     */
    BKLMChannelIndex& operator*();

  protected:

    /** Forward. */
    int m_Forward;

    /** Sector. */
    int m_Sector;

    /** Layer. */
    int m_Layer;

    /** Plane. */
    int m_Plane;

    /** Strip. */
    int m_Strip;

    /** Number of strips in current plane. */
    int m_NStripsPlane;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

  };

}
