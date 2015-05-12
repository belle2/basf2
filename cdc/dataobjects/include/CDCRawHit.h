/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRAWHIT_H
#define CDCRAWHIT_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <framework/datastore/RelationsObject.h>

#include <TObject.h>

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
              unsigned short adc, unsigned short tdc);

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
     * Destructor.
     */
    virtual ~CDCRawHit();
    // To derive from TObject.

  private:
    unsigned short m_status;  /// Status.
    int m_triggerNumber;      /// Trigger number.
    unsigned short m_node;    /// Copper node ID.
    unsigned short m_finess;  /// Finess Slot ID.
    unsigned short m_board;   /// FEE boad ID.
    unsigned short m_channel; /// Channel in a FEE (0-47).
    unsigned short m_triggerTime; /// Trigger Timing.

    unsigned short m_adc; /// FADC count.
    unsigned short m_tdc; /// TDC count (not subtracted by trigger time).

    ClassDef(CDCRawHit, 1);
  };
}

#endif
