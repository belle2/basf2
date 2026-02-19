#include <top/modules/TOPBackSplashTiming/TOPBackSplashTimingModule.h>
#include <math.h>
#include <list>
#include <framework/logging/Logger.h>
using namespace Belle2;

// Register module so basf2 can find it
REG_MODULE(TOPBackSplashTiming);

TOPBackSplashTimingModule::TOPBackSplashTimingModule()
  : Module(),
    m_eclClusters("ECLClusters")   // DataStore name
{
  setDescription("Print energy of all ECL clusters");
}

void TOPBackSplashTimingModule::initialize()
{
  B2INFO("TOPBackSplashTimingModule initialized");
}

void TOPBackSplashTimingModule::event()
{
  std::list<int> slotsFromClusters;
  std::list<int> cosTheta_nearestFromClusters;
  // Loop over all clusters in this event
  for (const auto& cluster : m_eclClusters) {

    if (cluster.isNeutral() && cluster.getDetectorRegion() == 2) {
      double theta = cluster.getTheta();
      double phi = cluster.getPhi();

      double cosTheta = std::cos(theta);
      int clusterSlot;
      double cosTheta_nearest = std::round(cosTheta * 10) / 10 ;

      if (phi > 0) {
        clusterSlot = int(std::floor(phi / (2 * M_PI / 16)) + 1);
      } else {
        clusterSlot = int(std::floor((phi + 2 * M_PI) / (2 * M_PI / 16)) + 1);
      }


      slotsFromClusters.push_back(clusterSlot);
      cosTheta_nearestFromClusters.push_back(cosTheta_nearest);

      int nDigits = 0;
      for (const auto& digi : m_digits) {
        if (digi.getModuleID() == clusterSlot) {
          //B2INFO("Digits = " << digi.getTime());
          nDigits++;
        }
      }
      B2INFO("ECL Cluster Eraw / theta/cosTheta/nearest cosTheta /phi/ slot /nDigits = " << cluster.getEnergyRaw() << " / " << theta <<
             " / " << cosTheta << " / " << cosTheta_nearest << " / " << phi << " / " << clusterSlot << " / " << nDigits);
    }

  }

}
