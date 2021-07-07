/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/geometry/GeoKLMCreator.h>

/* Belle 2 headers. */
#include <geometry/CreatorFactory.h>

using namespace Belle2;

/* Register the creator */
geometry::CreatorFactory<KLM::GeoKLMCreator> GeoKLMFactory("KLMCreator");

KLM::GeoKLMCreator::GeoKLMCreator()
{
}

KLM::GeoKLMCreator::~GeoKLMCreator()
{
}

void KLM::GeoKLMCreator::create(const GearDir& content,
                                G4LogicalVolume& topVolume,
                                geometry::GeometryTypes type)
{
  m_GeoBKLMCreator.create(content, topVolume, type);
  m_GeoEKLMCreator.create(content, topVolume, type);
}

void KLM::GeoKLMCreator::createFromDB(const std::string& name,
                                      G4LogicalVolume& topVolume,
                                      geometry::GeometryTypes type)
{
  m_GeoBKLMCreator.createFromDB(name, topVolume, type);
  m_GeoEKLMCreator.createFromDB(name, topVolume, type);
}

void KLM::GeoKLMCreator::createPayloads(const GearDir& content,
                                        const IntervalOfValidity& iov)
{
  m_GeoBKLMCreator.createPayloads(content, iov);
  m_GeoEKLMCreator.createPayloads(content, iov);
}

