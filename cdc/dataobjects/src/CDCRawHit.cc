/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dataobjects/CDCRawHit.h>

using namespace std;
using namespace Belle2;

CDCRawHit::CDCRawHit() :
  m_status(0), m_triggerNumber(0), m_node(0), m_finess(0), m_board(0),
  m_channel(0), m_triggerTime(0), m_adc(0), m_tdc(0), m_tdc2nd(0), m_tot(0)
{
}


CDCRawHit::CDCRawHit(unsigned short status,
                     int trgNumber,
                     unsigned short node,
                     unsigned short finess,
                     unsigned short board,
                     unsigned short channel,
                     unsigned short trgTime,
                     unsigned short adc,
                     unsigned short tdc,
                     unsigned short tdc2,
                     unsigned short tot)
{
  m_status = status;
  m_triggerNumber = trgNumber;
  m_node = node;
  m_finess = finess;
  m_board = board;
  m_channel = channel;
  m_triggerTime = trgTime;
  m_adc = adc;
  m_tdc = tdc;
  m_tdc2nd = tdc2;
  m_tot = tot;
}

CDCRawHit::~CDCRawHit()
{
}
