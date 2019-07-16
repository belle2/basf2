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
     * Index level (loop over KLM elements at the specified level).
     */
    enum IndexLevel {

      /** Forward. */
      c_IndexLevelForward = 1,

      /** Sector. */
      c_IndexLevelSector = 2,

      /** Layer. */
      c_IndexLevelLayer = 3,

      /** Plane. */
      c_IndexLevelPlane = 4,

      /** Strip. */
      c_IndexLevelStrip = 5,

    };

    /**
     * Constructor.
     * @param[in] indexLevel Index level.
     */
    explicit BKLMChannelIndex(enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Constructor.
     * @param[in] forward    Forward (1) or backward (0) BKLM.
     * @param[in] sector     Sector (1-based).
     * @param[in] layer      Layer (1-based).
     * @param[in] plane      Plane (0-based).
     * @param[in] strip      Strip (1-based).
     * @param[in] indexLevel Index level.
     */
    BKLMChannelIndex(int forward, int sector, int layer, int plane, int strip,
                     enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Destructor.
     */
    ~BKLMChannelIndex();

    /**
     * Set index level.
     */
    void setIndexLevel(enum IndexLevel indexLevel);

    /**
     * Get forward.
     */
    int getForward() const
    {
      return m_Forward;
    }

    /**
     * Get sector.
     */
    int getSector() const
    {
      return m_Sector;
    }

    /**
     * Get layer.
     */
    int getLayer() const
    {
      return m_Layer;
    }

    /**
     * Get plane.
     */
    int getPlane() const
    {
      return m_Plane;
    }

    /**
     * Get strip.
     */
    int getStrip() const
    {
      return m_Strip;
    }

    /**
     * Get KLM channel number.
     */
    uint16_t getKLMChannelNumber() const;

    /**
     * Get KLM module number.
     */
    uint16_t getKLMModuleNumber() const;

    /**
     * Get KLM sector number.
     */
    uint16_t getKLMSectorNumber() const;

    /**
     * First channel.
     */
    BKLMChannelIndex begin();

    /**
     * Last channel.
     */
    BKLMChannelIndex& end();

    /**
     * Operator ++.
     */
    BKLMChannelIndex& operator++();

    /**
     * Increment (to use in Python).
     */
    BKLMChannelIndex& increment();

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

    /**
     * Increment the index.
     */
    void increment(enum IndexLevel indexLevel);

    /** Index level. */
    enum IndexLevel m_IndexLevel;

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
