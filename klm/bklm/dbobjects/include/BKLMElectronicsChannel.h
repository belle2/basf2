/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * BKLM electronics channel.
   */
  class BKLMElectronicsChannel : public TObject {

  public:

    /**
     * Constructor.
     */
    BKLMElectronicsChannel() :
      m_Copper(0),
      m_Slot(0),
      m_Lane(0),
      m_Axis(0),
      m_Channel(0)
    {
    }

    /**
     * Constructor.
     * @param[in] copper  Copper.
     * @param[in] slot    Slot.
     * @param[in] lane    Lane.
     * @param[in] axis    Axis.
     * @param[in] channel Channel.
     */
    BKLMElectronicsChannel(
      int copper, int slot, int lane, int axis, int channel) :
      m_Copper(copper),
      m_Slot(slot),
      m_Lane(lane),
      m_Axis(axis),
      m_Channel(channel)
    {
    }

    /**
     * Destructor.
     */
    ~BKLMElectronicsChannel()
    {
    }

    /**
     * Get copper.
     */
    int getCopper() const
    {
      return m_Copper;
    }

    /**
     * Set copper.
     * @param[in] copper Copper.
     */
    void setCopper(int copper)
    {
      m_Copper = copper;
    }

    /**
     * Get slot.
     */
    int getSlot() const
    {
      return m_Slot;
    }

    /**
     * Set slot.
     * @param[in] slot Slot.
     */
    void setSlot(int slot)
    {
      m_Slot = slot;
    }

    /**
     * Get lane.
     */
    int getLane() const
    {
      return m_Lane;
    }

    /**
     * Set lane.
     * @param[in] lane Lane.
     */
    void setLane(int lane)
    {
      m_Lane = lane;
    }

    /**
     * Get axis.
     */
    int getAxis() const
    {
      return m_Axis;
    }

    /**
     * Set axis.
     * @param[in] axis Axis.
     */
    void setAxis(int axis)
    {
      m_Axis = axis;
    }

    /**
     * Get channel.
     */
    int getChannel() const
    {
      return m_Channel;
    }

    /**
     * Set channel.
     * @param[in] channel Channel.
     */
    void setChannel(int channel)
    {
      m_Channel = channel;
    }

    /**
     * Operator <.
     */
    bool operator<(const BKLMElectronicsChannel& channel) const;

  private:

    /** Copper. */
    int m_Copper;

    /** Slot. */
    int m_Slot;

    /** Lane. */
    int m_Lane;

    /** Axis. */
    int m_Axis;

    /** Channel. */
    int m_Channel;

    /** Class version. */
    ClassDef(BKLMElectronicsChannel, 1);

  };

}
