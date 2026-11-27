/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/ERRORNo.h"

using namespace Belle2;

const ERRORNo ERRORNo::UNKNOWN(0, "UNKNOWN");
const ERRORNo ERRORNo::NSMONLINE(11, "NSMONLINE");
const ERRORNo ERRORNo::NSMSENDREQ(12, "NSMSENDREQ");
const ERRORNo ERRORNo::NSMMEMOPEN(13, "SENOPEN");
const ERRORNo ERRORNo::NSMINIT(14, "NSMINIT");
const ERRORNo ERRORNo::DATABASE(31, "DATABASE");

const ERRORNo& ERRORNo::operator=(const std::string& label)
{
  if (label == NSMSENDREQ.getLabel()) *this = NSMSENDREQ;
  else if (label == NSMONLINE.getLabel()) *this = NSMONLINE;
  else if (label == NSMMEMOPEN.getLabel()) *this = NSMMEMOPEN;
  else if (label == NSMINIT.getLabel()) *this = NSMINIT;
  else if (label == DATABASE.getLabel()) *this = DATABASE;
  else *this = UNKNOWN;
  return *this;
}

const ERRORNo& ERRORNo::operator=(int id)
{
  if (id == NSMSENDREQ.getId()) *this = NSMSENDREQ;
  else if (id == NSMONLINE.getId()) *this = NSMONLINE;
  else if (id == NSMMEMOPEN.getId()) *this = NSMMEMOPEN;
  else if (id == NSMINIT.getId()) *this = NSMINIT;
  else if (id == DATABASE.getId()) *this = DATABASE;
  else *this = UNKNOWN;
  return *this;
}

const ERRORNo& ERRORNo::operator=(const char* label)
{
  if (label != nullptr)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

