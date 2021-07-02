/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawTrailer_v1.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawTrailer_v1.h>

using namespace Belle2;

//ClassImp(RawTrailer_v1);

RawTrailer_v1::RawTrailer_v1()
{
  m_buffer = NULL;
}

RawTrailer_v1::~RawTrailer_v1()
{
}
