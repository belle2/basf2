/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#include <ecl/modules/eclDisplay/EclPainter.h>

using namespace Belle2;

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

void EclPainter::setData(EclData* data)
{
  m_ecl_data = data;
}

EclData* EclPainter::getData()
{
  return m_ecl_data;
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
  return NULL;
}

void EclPainter::setXRange(int, int)
{
}

void EclPainter::getNewRootObjectName(char* buf, int n)
{
  snprintf(buf, n, "ECL DATA_%d", m_obj_counter++);
}
