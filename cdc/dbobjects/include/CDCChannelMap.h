/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      m_wire(65535), m_board(0), m_channel(0)
    {}

    /**
     * Constructor.
     */
    CDCChannelMap(unsigned short slayer, unsigned short layer,
                  unsigned short wire,
                  unsigned short board, unsigned short channel):
      m_wire(WireID(slayer, layer, wire).getEWire()),
      m_board(board), m_channel(channel)
    {

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
      return (m_wire / 4096);
    }

    /**
     * Get layer ID.
     */
    unsigned short getILayer() const
    {
      return ((m_wire % 4096) / 512);
    }

    /**
     * Get wire ID.
     */
    unsigned short getIWire() const
    {
      return (m_wire % 512);
    }

  private:
    unsigned short m_wire; /**< Wire ID */
    unsigned short m_board; /**< Board ID. */
    unsigned short m_channel; /**< Channel ID */

    ClassDef(CDCChannelMap, 2); /**< ClassDef */

  };

} // end namespace Belle2
