#include "analysis/particle/Ptype.h"

using namespace Belle2;

Ptype::Ptype(const int lund)
  : m_lund(lund),
    m_charge((lund > 0) - (lund < 0))
{
  switch (abs(lund)) {
    case 22:
      m_mass = 0.0;
      break;
    case 11:
      m_mass = 0.000510999;
      break;
    case 13:
      m_mass = 0.1056584;
      break;
    case 111:
      m_mass = 0.134977;
      break;
    case 211:
      m_mass = 0.139570;
      break;
    case 113:
      m_mass = 0.77549;
      break;
    case 213:
      m_mass = 0.7754;
      break;
    case 221:
      m_mass = 0.54751;
      break;
    case 331:
      m_mass = 0.95778;
      break;
    case 223:
      m_mass = 0.78265;
      break;
    case 333:
      m_mass = 1.019460;
      break;
    case 310:
      m_mass = 0.497648;
      break;
    case 130:
      m_mass = 0.497648;
      break;
    case 311:
      m_mass = 0.497648;
      break;
    case 321:
      m_mass = 0.493677;
      break;
    case 313:
      m_mass = 0.8960;
      break;
    case 323:
      m_mass = 0.89166;
      break;
    case 411:
      m_mass = 1.86962;
      break;
    case 421:
      m_mass = 1.86484;
      break;
    case 413:
      m_mass = 2.01027;
      break;
    case 423:
      m_mass = 2.00697;
      break;
    case 431:
      m_mass = 1.96849;
      break;
    case 433:
      m_mass = 2.1123;
      break;
    case 441:
      m_mass = 2.9798;
      break;
    case 443:
      m_mass = 3.096916;
      break;
    case 20443:
      m_mass = 3.51066;
      break;
    case 100443:
      m_mass = 3.68609;
      break;
    case 30443:
      m_mass = 3.7724;
      break;
    case 9000443:
      m_mass = 4.039;
      break;
    case 9010443:
      m_mass = 4.153;
      break;
    case 9020443:
      m_mass = 4.421;
      break;
    case 120443:
      m_mass = 3.8714;
      break;
    case 90000443:
      m_mass = 3.9430;
      break;
    case 91000443:
      m_mass = 3.9400;
      break;
    case 511:
      m_mass = 5.2795;
      break;
    case 521:
      m_mass = 5.2791;
      break;
    case 513:
      m_mass = 5.3251;
      break;
    case 523:
      m_mass = 5.3249;
      break;
    case 551:
      m_mass = 9.403;
      break;
    case 553:
      m_mass = 9.46030;
      break;
    case 10553:
      m_mass = 9.875;
      break;
    case 100553:
      m_mass = 10.02326;
      break;
    case 200553:
      m_mass = 10.3552;
      break;
    case 300553:
      m_mass = 10.58000;
      break;
    case 9000553:
      m_mass = 10.865;
      break;
    case 2212:
      m_mass = 0.9382720;
      break;
    case 2112:
      m_mass = 0.93956533;
      break;
    case 3122:
      m_mass = 1.115683;
      break;
    case 3222:
      m_mass = 1.18937;
      break;
    case 3212:
      m_mass = 1.192642;
      break;
    case 3112:
      m_mass = 1.197449;
      break;
    case 3334:
      m_mass = 1.67245;
      break;
    case 4222:
      m_mass = 2.4402;
      break;
    case 4212:
      m_mass = 2.4529;
      break;
    case 4112:
      m_mass = 2.45376;
      break;
    default:
      m_mass = 0.;
  }
}
