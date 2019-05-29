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
   * EKLM channel index.
   */
  class EKLMChannelIndex {

  public:

    /**
     * Index level (loop over KLM elements at the specified level).
     */
    enum IndexLevel {

      /** Endcap. */
      c_IndexLevelEndcap = 1,

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
    explicit EKLMChannelIndex(enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Constructor.
     * @param[in] endcap     Endcap.
     * @param[in] sector     Sector.
     * @param[in] layer      Layer.
     * @param[in] plane      Plane.
     * @param[in] strip      Strip.
     * @param[in] indexLevel Index level.
     */
    EKLMChannelIndex(int endcap, int sector, int layer, int plane, int strip,
                     enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Destructor.
     */
    ~EKLMChannelIndex();

    /**
     * Set index level.
     */
    void setIndexLevel(enum IndexLevel indexLevel)
    {
      m_IndexLevel = indexLevel;
    }

    /**
     * Get endcap.
     */
    int getEndcap() const
    {
      return m_Endcap;
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
     * First channel.
     */
    EKLMChannelIndex begin();

    /**
     * Last channel.
     */
    EKLMChannelIndex& end();

    /**
     * Operator ++.
     */
    EKLMChannelIndex& operator++();

    /**
     * Operator ==.
     */
    bool operator==(EKLMChannelIndex& index);

    /**
     * Operator !=.
     */
    bool operator!=(EKLMChannelIndex& index);

    /**
     * Operator *.
     */
    EKLMChannelIndex& operator*();

  protected:

    /**
     * Increment the index.
     */
    void increment(enum IndexLevel indexLevel);

    /** Index level. */
    enum IndexLevel m_IndexLevel;

    /** Endcap. */
    int m_Endcap;

    /** Sector. */
    int m_Sector;

    /** Layer. */
    int m_Layer;

    /** Plane. */
    int m_Plane;

    /** Strip. */
    int m_Strip;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbersEKLM;

  };

}
