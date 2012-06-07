/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPDIGIT_H
#define TOPDIGIT_H

#include <TObject.h>

namespace Belle2 {


  /*! Class to store TOP digitized hits (output of TOPDigi)
   * relation to TOPSimHit
   * filled in top/modules/TOPDigitizer/src/TOPDigiModule.cc
   */

  class TOPDigit : public TObject {
  public:

    /*! Default constructor */

    TOPDigit():
      m_barID(0),
      m_channelID(0),
      m_TDC(0) {
    }

    /*!  Full constructor
     * @param barID     bar ID
     * @param channelID channel ID
     * @param TDC       digitized detection time
     */

    TOPDigit(int barID, int channelID, int TDC):
      m_barID(barID),
      m_channelID(channelID),
      m_TDC(TDC) {
    }

    /*! Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /*! Get channel ID
     * @return channel ID
     */
    int getChannelID() const { return m_channelID; }

    /*! Get digitized time
     * @return digitized time
     */
    int getTDC() const { return m_TDC; }

  private:
    int m_barID;             /**< Bar ID */
    int m_channelID;         /**< Channel ID */
    int m_TDC;               /**< digitized time */

    ClassDef(TOPDigit, 1); /**< ClassDef */

  };

} // end namespace Belle2

#endif
