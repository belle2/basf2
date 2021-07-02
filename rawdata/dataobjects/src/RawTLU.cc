/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawTLU.cc
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#include <rawdata/dataobjects/RawTLU.h>
using namespace std;
using namespace Belle2;

RawTLU::RawTLU()
{
  m_access = new RawTLUFormat;
}

RawTLU::~RawTLU()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}
