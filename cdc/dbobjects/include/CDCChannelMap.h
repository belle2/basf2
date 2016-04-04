/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object of CDC channel map.
   */
  class CDCChannelMap: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCChannelMap():
      m_wireID(), m_board(0), m_channel(0)
    {}

    /**
     * Constructor.
     */
    CDCChannelMap(unsigned short slayer, unsigned short layer,
                  unsigned short wire,
                  unsigned short board, unsigned short channel):
      m_board(board), m_channel(channel)
    {
      m_wireID = WireID(slayer, layer, wire);
    }

    /**
     * Get Board ID.
     */
    unsigned getBoardID() const {return m_board;}

    /**
     * Get channel.
     */
    unsigned getBoardChannel() const {return m_channel;}

    /**
     * Get super layer ID.
     */
    unsigned short getISuperLayer() const
    {
      return m_wireID.getISuperLayer();
    }

    /**
     * Get layer ID.
     */
    unsigned short getILayer() const
    {
      return m_wireID.getILayer();
    }

    /**
     * Get wire ID.
     */
    unsigned short getIWire() const
    {
      return m_wireID.getIWire();
    }

  private:
    WireID m_wireID; /**< Wire ID. */
    unsigned short m_board; /**< Board ID. */
    unsigned short m_channel; /** Channel ID */

    ClassDef(CDCChannelMap, 1); /**< ClassDef */

  };

} // end namespace Belle2
