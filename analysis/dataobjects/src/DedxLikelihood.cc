#include <analysis/dataobjects/DedxLikelihood.h>
#include <framework/logging/Logger.h>

#include <TH1F.h>
#include <TFile.h>

#include <cmath>

using namespace Belle2;

TH1F** DedxLikelihood::s_momentumPrior = 0;

double DedxLikelihood::getProbability(DedxParticle type) const
{
  double numerator = 0.0;
  double denominator = 0.0;
  const Int_t p_bin = s_momentumPrior ? (s_momentumPrior[0]->GetXaxis()->FindFixBin(m_p)) : 0;
  for (int i = 0; i < c_Dedx_num_particles; i++) {
    double likelihood = exp((double)getLogLikelihood((DedxParticle)i));

    //get momentum prior
    if (s_momentumPrior) {
      if (p_bin < 1 or p_bin > s_momentumPrior[i]->GetNbinsX()) { //overflow?
        likelihood *= s_momentumPrior[i]->GetBinContent(p_bin);
      } else { //in normal histogram range
        likelihood *= s_momentumPrior[i]->Interpolate(m_p);
      }
    }
    if (type == i)
      numerator = likelihood;

    denominator += likelihood;
  }

  return numerator / denominator;
}


void DedxLikelihood::loadMomentumPriorsFromFile(const std::string& filename)
{
  TFile* pdf_file = new TFile(filename.c_str(), "READ");
  if (!pdf_file->IsOpen())
    B2FATAL("Couldn't open pdf file: " << filename);

  s_momentumPrior = new TH1F*[c_Dedx_num_particles];
  //load momentum PDFs
  for (int particle = 0; particle < c_Dedx_num_particles; particle++) {
    const int pdg_code = c_Dedx_pdg_codes[particle];
    s_momentumPrior[particle] = dynamic_cast<TH1F*>(pdf_file->Get(TString::Format("momentum_%i", pdg_code)));

    if (!s_momentumPrior[particle]) {
      B2WARNING("Couldn't find momentum PDF for PDG " << pdg_code);
    }
  }
}


ClassImp(DedxLikelihood)
