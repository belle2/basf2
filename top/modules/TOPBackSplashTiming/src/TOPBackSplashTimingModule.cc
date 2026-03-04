#include <top/modules/TOPBackSplashTiming/TOPBackSplashTimingModule.h>
#include <math.h>
#include <framework/logging/Logger.h>
#include <Math/Vector3D.h>
using namespace Belle2;

// Register module so basf2 can find it
REG_MODULE(TOPBackSplashTiming);

TOPBackSplashTimingModule::TOPBackSplashTimingModule()
  : Module(),
    m_eclClusters("ECLClusters"),   // DataStore name
    m_digits("TOPDigits"),
    m_MCParticles("MCParticles")
{
  setDescription("Print energy of all ECL clusters");
}

void TOPBackSplashTimingModule::initialize()
{
  B2INFO("TOPBackSplashTimingModule initialized");
}

void TOPBackSplashTimingModule::event()
{
  std::array<bool, 16> slotChecked{};
  std::array<std::vector<float>, 16> slotDigits;

  std::array<std::vector<float>, 16> cosThetaFitResults;
  std::array<std::vector<float>, 16> fittedCosThetas;

  for (const auto& cluster : m_eclClusters) {

    if (cluster.getDetectorRegion() != 2) {
      continue;
    }

    // Derive slot in front of cluster
    float clusterPhi = cluster.getPhi();
    int clusterSlot;
    if (clusterPhi > 0) { // clusterPhi will be between -pi and pi
      clusterSlot = int(std::floor(clusterPhi / (2 * M_PI / 16)) + 1);
    } else {
      clusterSlot = int(std::floor((clusterPhi + 2 * M_PI) / (2 * M_PI / 16)) + 1);
    }

    // Save digits, if they exist and are not already saved
    if (slotChecked[clusterSlot - 1] == true) {
      if (slotDigits[clusterSlot - 1].size() == 0) {
        continue;
      } // else slot has been checked and non-zero digits, print later
    } else {
      for (const auto& digi : m_digits) {
        if (digi.getModuleID() == clusterSlot) {
          slotDigits[clusterSlot - 1].push_back(digi.getTime());
        }
      }
      slotChecked[clusterSlot - 1] = true;
      if (slotDigits[clusterSlot - 1].size() == 0) {
        continue;
      }
    }
    // Determine which fitting model from cosTheta
    float clusterTheta = cluster.getTheta();
    //TODO check if within acceptance? Should be!
    float clusterCosTheta = std::cos(clusterTheta);
    float nearestClusterCosTheta = std::round(clusterCosTheta * 10) / 10; //i.e. round to 0.1

    B2INFO("ECL Cluster Eraw / theta/cosTheta/nearest cosTheta /phi/ slot /nDigits / isTrack = " << cluster.getEnergyRaw() << " / " <<
           clusterTheta <<
           " / " << clusterCosTheta << " / " << nearestClusterCosTheta << " / " << clusterPhi << " / " << clusterSlot << " / " <<
           slotDigits[clusterSlot - 1].size() << " / " << cluster.isTrack());


    //// Check if we've already fitted this cosTheta in this slot
    //int iFittedCosTheta=0
    //bool foundFittedCosTheta = false;
    //for (auto& fittedCosTheta: fittedCosThetas[clusterSlot-1]){
    //  if (nearestClusterCosTheta == fittedCosTheta){
    //    // TODO spit out ECL results for iFittedCosTheta
    //    foundFittedCosTheta == true;
    //    break;
    //  }
    //  iFittedCosTheta++;
    //}
    //if (foundFittedCosTheta == true){
    //  continue;
    //}

    //// TODO: New fits only by this point
    //digitTimes = slotDigits[clusterSlot-1]
    //RooRealVar x("digitTime","digitTime",0,std::max_element(digitTimes.begin(),digitTimes.end()))
    //RooDataSet dataset("dataset","digitTimes",RooArgSet(x))
    //for (float digitTime : digitTimes){
    //    x.setVal(digitTime);
    //    dataset.add(RooArgSet(x));
    //}
    ////

  }
  // Load anti-neutron mc particle info?
  for (const auto& part : m_MCParticles) {
    if (part.getPDG() == int(-2112)) {
      ROOT::Math::XYZVector nbar_mom = part.getMomentum();
      float nbar_phi = nbar_mom.Phi();
      float nbar_theta = nbar_mom.Theta();
      ROOT::Math::XYZVector decayVertex = part.getDecayVertex();
      float decayRho = decayVertex.Rho();

      int nbar_slot;
      if (nbar_phi > 0) { // nbar_phi will be between -pi and pi
        nbar_slot = int(std::floor(nbar_phi / (2 * M_PI / 16)) + 1);
      } else {
        nbar_slot = int(std::floor((nbar_phi + 2 * M_PI) / (2 * M_PI / 16)) + 1);
      }
      float nbarCosTheta = std::cos(nbar_theta);
      float nearestNbarCosTheta = std::round(nbarCosTheta * 10) / 10; //i.e. round to 0.1
      B2INFO("Anti-neutron has theta / phi  / slot / nearest cosTheta / transverse dist:" << nbar_theta << " / " << nbar_phi << " / " <<
             nbar_slot << " / " << nearestNbarCosTheta << " / " << decayRho);
    }
  }
}
