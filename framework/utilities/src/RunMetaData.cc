#include <framework/utilities/RunMetaData.h>

using namespace Belle2;
float RunMetaData::m_energyLER = 4.0;
float RunMetaData::m_energyHER = 7.0;
const float RunMetaData::m_crossingAngle = Unit::crossingAngleHER + Unit::crossingAngleLER;
const float RunMetaData::m_angleLER = Unit::crossingAngleLER;
