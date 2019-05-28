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
     * Constructor.
     */
    EKLMChannelIndex();

    /**
     * Constructor.
     * @param[in] endcap Endcap.
     * @param[in] sector Sector.
     * @param[in] layer  Layer.
     * @param[in] plane  Plane.
     * @param[in] strip  Strip.
     */
    EKLMChannelIndex(int endcap, int sector, int layer, int plane, int strip);

    /**
     * Destructor.
     */
    ~EKLMChannelIndex();

    /**
     * Get endcap.
     */
    int getEndcap()
    {
      return m_Endcap;
    }

    /**
     * Get sector.
     */
    int getSector()
    {
      return m_Sector;
    }

    /**
     * Get layer.
     */
    int getLayer()
    {
      return m_Layer;
    }

    /**
     * Get plane.
     */
    int getPlane()
    {
      return m_Plane;
    }

    /**
     * Get strip.
     */
    int getStrip()
    {
      return m_Strip;
    }

    /**
     * Get KLM channel number.
     */
    uint16_t getKLMChannelNumber();

    /**
     * First channel.
     */
    EKLMChannelIndex& begin();

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
