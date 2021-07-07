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
  if (label == NSMSENDREQ.getLabel()) {
    return NSMSENDREQ;
  } else if (label == NSMONLINE.getLabel()) {
    return NSMONLINE;
  } else if (label == NSMMEMOPEN.getLabel()) {
    return NSMMEMOPEN;
  } else if (label == NSMINIT.getLabel()) {
    return NSMINIT;
  } else if (label == DATABASE.getLabel()) {
    return DATABASE;
  } else {
    return UNKNOWN;
  }
}

const ERRORNo& ERRORNo::operator=(int id)
{
  if (id == NSMSENDREQ.getId()) {
    return NSMSENDREQ;
  } else if (id == NSMONLINE.getId()) {
    return NSMONLINE;
  } else if (id == NSMMEMOPEN.getId()) {
    return NSMMEMOPEN;
  } else if (id == NSMINIT.getId()) {
    return NSMINIT;
  } else if (id == DATABASE.getId()) {
    return DATABASE;
  }
  return UNKNOWN;
}

const ERRORNo& ERRORNo::operator=(const char* label)
{
  if (label != NULL)  *this = std::string(label);
  else *this = Enum::UNKNOWN;
  return *this;
}

