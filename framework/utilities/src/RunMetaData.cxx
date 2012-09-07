#include <framework/utilities/RunMetaData.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;
float RunMetaData::m_energyLER = 4.0;
float RunMetaData::m_energyHER = 7.0;
float RunMetaData::m_crossingAngle = Unit::crossingAngleHER + Unit::crossingAngleLER;
float RunMetaData::m_angleLER = Unit::crossingAngleLER;
