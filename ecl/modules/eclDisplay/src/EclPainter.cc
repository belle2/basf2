/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclDisplay/EclPainter.h>

//Root
#include <TString.h>

//ECL
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>

using namespace Belle2;
using namespace ECL;

int EclPainter::m_obj_counter = 0;

EclPainter::EclPainter(EclData* data) :
  m_mapper(0)
{
  setData(data);
  displayed_subsys = EclData::ALL;
}

EclPainter::~EclPainter()
{

}

void EclPainter::setMapper(ECL::ECLChannelMapper* mapper)
{
  m_mapper = mapper;
}

ECL::ECLChannelMapper* EclPainter::getMapper()
{
  return m_mapper;
}

void EclPainter::setDisplayedSubsystem(EclData::EclSubsystem sys)
{
  displayed_subsys = sys;
}

EclData::EclSubsystem EclPainter::getDisplayedSubsystem()
{
  return displayed_subsys;
}

TString EclPainter::getSubsystemTitle(EclData::EclSubsystem subsys)
{
  switch (subsys) {
    case EclData::BARR:
      return TString("Barrel");
    case EclData::FORW:
      return TString("Forward endcap");
    case EclData::BACKW:
      return TString("Backward endcap");
    case EclData::ALL:
      return TString("Full ECL");
    default:
      return TString();
  }
}

void EclPainter::getInformation(int, int, MultilineWidget* panel)
{
  char info[255];
  sprintf(info, "energy emission = %.2f MeV", m_ecl_data->getEnergyTotal());
  panel->setLine(0, info);
}

EclPainter* EclPainter::handleClick(int, int)
{
  return nullptr;
}

void EclPainter::setXRange(int, int)
{
}

void EclPainter::getNewRootObjectName(char* buf, int n)
{
  snprintf(buf, n, "ECL DATA_%d", m_obj_counter++);
}

void EclPainter::cloneFrom(const EclPainter& other)
{
  m_ecl_data = new EclData(*other.m_ecl_data);
  m_mapper = other.m_mapper;
  displayed_subsys = other.displayed_subsys;
}

