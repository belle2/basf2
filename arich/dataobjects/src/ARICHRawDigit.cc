/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dataobjects/ARICHRawDigit.h>

#include <string.h>

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

