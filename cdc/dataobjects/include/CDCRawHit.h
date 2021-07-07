/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// Includes
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {


  /**
   * The CDCRawHit (suppressed mode) class.
   *
   */
  class CDCRawHit : public RelationsObject {
  public:
    /**
     * Default constructor.
     */
    CDCRawHit();
    /**
     * Consturctor.
     */
    CDCRawHit(unsigned short status, int trgNumber, unsigned short node,
              unsigned short finess, unsigned short board,
              unsigned short channel, unsigned short trgTime,
              unsigned short adc, unsigned short tdc,
              unsigned short tdc2, unsigned short tot);

    /**
     * Getter for status.
     */
    inline int getStatus(void) const
    {
      return m_status;
    }

    /**
     * Getter for Trigger number.
     */
    inline int getTriggerNumber(void) const
    {
      return m_triggerNumber;
    }

    /**
     * Getter for Node ID.
     */
    inline unsigned short getNode(void) const
    {
      return m_node;
    }

    /**
     * Getter fot Finess ID.
     */

    inline unsigned short getFiness(void) const
    {
      return m_finess;
    }

    /**
     * Getter for trigger time.
     */
    inline unsigned short getTriggerTime(void) const
    {
      return m_triggerTime;
    }

    /**
     * Getter for FE channel.
     */
    inline unsigned short getFEChannel(void) const
    {
      return m_channel;
    }

    /**
     * Getter for boar ID.
     */
    inline unsigned short getBoardId(void) const
    {
      return m_board;
    }

    /**
     * Getter for FADC value.
     */
    inline unsigned short getFADC(void) const
    {
      return m_adc;
    }

    /**
     * Getter for TDC value.
     */
    inline unsigned short getTDC(void) const
    {
      return m_tdc;
    }

    /**
     * Getter for TOT value.
     */
    inline unsigned short getTOT(void) const
    {
      return m_tot;
    }

    /**
     * Destructor.
     */
    virtual ~CDCRawHit();
    // To derive from TObject.

  private:
    /**
     * Status of data block.
     */
    unsigned short m_status;
    /**
     * Trigger number.
     */
    int m_triggerNumber;
    /**
     * Copper node ID.
     */
    unsigned short m_node;
    /**
     * Finess Slot ID.
     */
    unsigned short m_finess;
    /**
     * FEE boad ID.
     */
    unsigned short m_board;
    /**
     * Channel in a FEE (0-47).
     */
    unsigned short m_channel;
    /**
     * Trigger Timing.
     */
    unsigned short m_triggerTime;
    /**
     * FADC count.
     */
    unsigned short m_adc;
    /**
     * TDC count (not subtracted by trigger time).
     */
    unsigned short m_tdc;

    /**
     * 2nd TDC count (not subtracted by trigger time).
     */
    unsigned short m_tdc2nd;

    /**
     * Time over threshold.
     */
    unsigned short m_tot;

    /// To derive from TObject
    ClassDef(CDCRawHit, 2);
  };
}
