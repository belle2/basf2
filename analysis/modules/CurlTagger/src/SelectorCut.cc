#include <analysis/modules/CurlTagger/SelectorCut.h>

using namespace Belle2;
using namespace CurlTagger;

SelectorCut::SelectorCut()
{
}

SelectorCut::~SelectorCut()
{
}

std::vector<float> SelectorCut::getVariables(Particle* iPart, Particle* jPart)
{
  float chargeMult = iPart -> getCharge() * jPart -> getCharge();
  float magDiffP = (iPart->getMomentum() - jPart->getMomentum()).Mag();
  float phi = iPart->getMomentum().Angle(jPart->getMomentum());
  return {chargeMult, magDiffP, phi};
}

float SelectorCut::getProbability(Particle* iPart, Particle* jPart)
{
  //Selection from BN1079
  std::vector<float> variables = getVariables(iPart, jPart);
  float chargeMult = variables[0];
  float magDiffP = variables[1];
  float phi = variables[2];

  if (magDiffP > 0.1) {return 0.;}
  if (chargeMult > 0) {
    if (phi < 15 * TMath::Pi() / 180) {return 1.;}
  }
  if (chargeMult < 0) {
    if (phi > 165 * TMath::Pi() / 180) {return 1.;}
  }

  return 0.;
}
