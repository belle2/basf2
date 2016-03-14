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
#include <eklm/geometry/GeometryData.h>

using namespace Belle2;

EKLM::GeometryData::GeometryData(bool global) : m_TransformData(global)
{
}

EKLM::GeometryData::~GeometryData()
{
}

