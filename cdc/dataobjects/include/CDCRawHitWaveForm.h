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
   * The CDCRawHitWaveForm (Full output mode) class.
   *
   */
  class CDCRawHitWaveForm : public RelationsObject {
  public:
    /**
     * Default constructor.
     */
    CDCRawHitWaveForm();

    /**
     * Constructor.
     */
    CDCRawHitWaveForm(unsigned short status,
                      int trgNumber, unsigned short node,
                      unsigned short finess, unsigned short board,
                      unsigned short channel, unsigned short sample,
                      unsigned short trgTime,
                      unsigned short adc, unsigned short tdc);


    /**
     * Getter for status.
     */
    inline int getStatus(void) const
    {
      return m_status;
    }


    /**
     * Destructor.
     */
    virtual ~CDCRawHitWaveForm();


    /**
     * Setter for trigger number.
     */
    inline void setTriggerNumber(int trgNumber)
    {
      m_triggerNumber = trgNumber;
    }


    /**
     * Getter for trigger number.
     */
    inline int getTriggerNumber(void) const
    {
      return m_triggerNumber;
    }

    /**
     * Setter for Node ID.
     */
    inline void setNode(unsigned short node)
    {
      m_node = node;
    }


    /**
     * Getter for Node ID.
     */
    inline unsigned short getNode(void) const
    {
      return m_node;
    }

    /**
     * Setter for Finess ID.
     */
    inline void setFiness(unsigned short finess)
    {
      m_finess = finess;
    }

    /**
     * Getter for Finess ID.
     */
    inline unsigned short getFiness(void) const
    {
      return m_finess;
    }


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
     * Index of FADC sampling.
     */
    unsigned short m_sample;
    /**
     * FADC count.
     */
    unsigned short m_adc;
    /**
     * TDC count (not subtracted by trigger time).
     */
    unsigned short m_tdc;

    /// To derive from TObject
    ClassDef(CDCRawHitWaveForm, 1);


  };
}
