/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dataobjects/ARICHRawDigit.h>

using namespace Belle2;

void ARICHRawDigit::set(int type, int ver, int boardid, int febno, unsigned int length, unsigned int trgno)
{
  m_type = type;
  m_ver = ver;
  m_boardid = boardid;
  m_febno = febno;
  m_length = length;
  m_trgno = trgno;
}

