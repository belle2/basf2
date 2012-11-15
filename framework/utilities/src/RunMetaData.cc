#include <framework/utilities/RunMetaData.h>

#include <framework/gearbox/GearDir.h>


using namespace Belle2;

float RunMetaData::getEnergyLER()
{
  GearDir ler("/Detector/SuperKEKB/LER/");
  return ler.getDouble("energy");
}

float RunMetaData::getEnergyHER()
{
  GearDir her("/Detector/SuperKEKB/HER/");
  return her.getDouble("energy");
}

float RunMetaData::getCrossingAngle()
{
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");
  return her.getDouble("angle") - ler.getDouble("angle");
}

float RunMetaData::getAngleLER()
{
  GearDir ler("/Detector/SuperKEKB/LER/");
  return -ler.getDouble("angle");
}
