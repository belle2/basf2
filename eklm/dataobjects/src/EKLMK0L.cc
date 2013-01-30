/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMK0L.h>

using namespace Belle2;

ClassImp(EKLMK0L);

EKLMK0L::EKLMK0L()
{
}

EKLMK0L::~EKLMK0L()
{
}

void EKLMK0L::setTime(float time)
{
  m_time = time;
}

float EKLMK0L::getTime()
{
  return m_time;
}

void EKLMK0L::setLayers(int l)
{
  m_layers = l;
}

int EKLMK0L::getLayers()
{
  return m_layers;
}

