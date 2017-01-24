#include <tracking/dataobjects/ObserverInfo.h>

using namespace Belle2;

ObserverInfo::ObserverInfo() :
  m_results(),
  m_wasAccepted(),
  m_wasUsed(),
  m_hits(),
  m_mainMCParticleID(-1),
  m_mainPurity(-1.)
{
};

ObserverInfo::~ObserverInfo()
{
};
