/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  namespace CDC {


    /**
     * CDCChannelData.
     * CDC channel data contains the real data for
     * 1 hit channel.
     */

    class CDCChannelData {
    public:

      CDCChannelData() : m_board(0), m_channel(0), m_length(0), m_tot(0),
        m_adc(0), m_tdc(0), m_tdc2(0), m_f2ndHit(false) {}


      /**
       * Constructor of the class fo the "2 hits" event.
       */

      inline CDCChannelData(int board, int ch, int len, int tot,
                            int adc, int tdc, int tdc2)
      {
        m_board = board;
        m_channel = ch;
        m_length = len;
        m_tot = tot;
        m_adc = adc;
        m_tdc = tdc;
        m_tdc2 = tdc2;
        m_f2ndHit = true;
      }

      /**
       * Constructor of the class fo the "1 hit" event.
       */
      inline CDCChannelData(int board, int ch, int len, int tot,
                            int adc, int tdc)
      {
        m_board = board;
        m_channel = ch;
        m_length = len;
        m_tot = tot;
        m_adc = adc;
        m_tdc = tdc;
        m_tdc2 = 0;
        m_f2ndHit = false;
      }

      /**
       * Set 2nd hit timing.
       */
      inline void setTDC2ndHit(int tdc)
      {
        m_length += 2;
        m_tdc2 = tdc;
        m_f2ndHit = true;
      }
      /**
       * Get the channel ID.
       */
      inline unsigned short getChannel() const
      {
        return m_channel;
      }

      /**
       * Get the board ID.
       */
      inline unsigned short getBoard() const
      {
        return m_board;
      }


      /**
       * Get the TDC.
       */
      inline unsigned short getTDCCount() const
      {
        return m_tdc;
      }

      /**
       * Get the 2nd TDC.
       */
      inline unsigned short getTDCCount2ndHit() const
      {
        return m_tdc2;
      }

      /**
       * Get the ADC.
       */
      inline unsigned short getADCCount() const
      {
        return m_adc;
      }

      /**
       * Get the flag of 2nd hit.
       * true : 2 hit event, false : 1 hit event.
       */
      inline bool is2ndHit() const
      {
        return m_f2ndHit;
      }

      /**
       * Get data length.
       */
      inline unsigned short getDataLength() const
      {
        return m_length;
      }

      /**
       * Get time over threshold.
       */
      inline unsigned short getTOT() const
      {
        return m_tot;
      }

    private:

      /**
       * Board ID (0-300).
       */
      unsigned short m_board;

      /**
       * Channel ID (0-47).
       */
      unsigned short m_channel;

      /**
       * Data length.
       */
      unsigned short m_length;

      /**
       * Time over threshold.
       */
      unsigned short m_tot;

      /**
       * FADC count.
       */
      unsigned short m_adc;

      /**
       * TDC count.
       */
      unsigned short m_tdc;

      /**
       * TDC count of 2nd hit.
       */
      unsigned short m_tdc2;

      /**
       * Flag for 2nd hit.
       */
      bool m_f2ndHit;
    };


  } //end CDC namespace;
} // end namespace Belle2
