/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/PathManager.h>


using namespace Belle2;
using namespace std;


PathManager::PathManager()
{

}


PathManager::~PathManager()
{

}


PathPtr PathManager::createPath()
{
  PathPtr newPath(new Path());
  m_createdPathList.push_back(newPath);
  return newPath;
}
