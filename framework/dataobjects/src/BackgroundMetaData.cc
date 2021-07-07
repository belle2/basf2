/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/BackgroundMetaData.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


bool BackgroundMetaData::canBeMerged(const BackgroundMetaData* otherObj)
{

  if (otherObj->getBackgroundType() != m_backgroundType) {
    B2ERROR("BackgroundMetaData: objects cannot be merged (different backgroundType)");
    return false;
  }

  if (otherObj->getBackgroundTag() != m_backgroundTag) {
    B2ERROR("BackgroundMetaData: objects cannot be merged (different backgroundTag)");
    return false;
  }

  if (otherObj->getFileType() != m_fileType) {
    B2ERROR("BackgroundMetaData: objects cannot be merged (different fileType)");
    return false;
  }

  return true;
}


void BackgroundMetaData::merge(const Mergeable* other)
{
  auto otherObj = static_cast<const BackgroundMetaData*>(other);

  if (otherObj->getBackgroundType().empty()) return; // no merge for empty object

  if (m_backgroundType.empty()) { // empty, replace it with other
    *this = *otherObj;
    return;
  }

  if (!canBeMerged(otherObj)) throw BackgroundMetaDataNotMergeable();

  m_realTime += otherObj->getRealTime();

}

void BackgroundMetaData::clear()
{
  m_realTime = 0;
}
