/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/dbobjects/SVDDQMPlotsConfiguration.h>

std::string Belle2::SVDDQMPlotsConfiguration::name = std::string("SVDDQMPlotsConfiguration");

Belle2::SVDDQMPlotsConfiguration::SVDDQMPlotsConfiguration(const TString& uniqueID)
  : TObject(), m_uniqueID(uniqueID)
{
  m_svdListOfSensors.clear();
}
