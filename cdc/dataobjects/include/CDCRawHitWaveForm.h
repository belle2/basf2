/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRAWHITWAVEFORM_H
#define CDCRAWHITWAVEFORM_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationsObject.h>
#include <TObject.h>

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

    inline void setTriggerNumber(int trgNumber)
    {
      m_triggerNumber = trgNumber;
    }

    inline int getTriggerNumber(void) const
    {
      return m_triggerNumber;
    }

    inline void setNode(unsigned short node)
    {
      m_node = node;
    }

    inline unsigned short getNode(void) const
    {
      return m_node;
    }

    inline void setFiness(unsigned short finess)
    {
      m_finess = finess;
    }

    inline unsigned short getFiness(void) const
    {
      return m_finess;
    }


  private:
    unsigned short m_status; /// Status of data.
    int m_triggerNumber;              /// Event number.
    unsigned short m_node;    /// Copper node ID.
    unsigned short m_finess;  /// Finess Slot ID.
    unsigned short m_board;   /// FEE boad ID.
    unsigned short m_channel; /// Channel in a FEE (0-47).
    unsigned short m_triggerTime; /// Trigger Timing.
    unsigned short m_sample; /// index of FADC sampling.

    unsigned short m_adc; /// FADC count.
    unsigned short m_tdc; /// TDC count (not subtracted by trigger time).

    /// To derive from TObject
    ClassDef(CDCRawHitWaveForm, 1);


  };
}

#endif // End of RAWCDCWAVEFORM_H
