/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>CombinedPIDPerformance.root</input>
  <contact>jvbennet@olemiss.edu</contact>
  <description>Check PID performance</description>
  <interval>release</interval>
</header>
*/

#include "TFile.h"
#include "TEfficiency.h"
#include <TApplication.h>
#include <TROOT.h>
#include <TSystem.h>

#include <iostream>


void combinedPID_performance(){

  TFile* infile = new TFile("CombinedPIDPerformance.root","READ");
  if(!infile) {
    std::cerr << "Couldn't read file!\n";
    exit(1);
  }
  if(!(infile->GetListOfKeys()->Contains("epik_7"))) {
    std::cerr << "Input file doesn't contain efficiencies, aborting!\n";
    exit(1);
  }

  // efficiencies for all PID detectors
  TEfficiency* epi = (TEfficiency*)infile->Get("epik_7");
  TEfficiency* ek = (TEfficiency*)infile->Get("ekpi_7");
  TEfficiency* ep = (TEfficiency*)infile->Get("eppi_7");
  TEfficiency* epk = (TEfficiency*)infile->Get("epk_7");

  // fake rates for all PID detectors
  TEfficiency* fpi = (TEfficiency*)infile->Get("fpik_7");
  TEfficiency* fk = (TEfficiency*)infile->Get("fkpi_7");
  TEfficiency* fp = (TEfficiency*)infile->Get("fppi_7");
  TEfficiency* fpk = (TEfficiency*)infile->Get("fpk_7");

  TFile* outfile = new TFile("pid_combinedPerformance.root","RECREATE");
  outfile->cd();

  epi->SetTitle("PID efficiency (L_{#pi} > L_{K});p  [GeV/c];Efficiency");
  ek->SetTitle("PID efficiency (L_{K} > L_{#pi});p  [GeV/c];Efficiency");
  ep->SetTitle("PID efficiency (L_{p} > L_{#pi});p  [GeV/c];Efficiency");
  epk->SetTitle("PID efficiency (L_{p} > L_{K});p  [GeV/c];Efficiency");

  fpi->SetTitle("PID fake rate (L_{#pi} > L_{K});p  [GeV/c];Fake Rate");
  fk->SetTitle("PID fake rate (L_{K} > L_{#pi});p  [GeV/c];Fake Rate");
  fp->SetTitle("PID fake rate (L_{p} > L_{#pi});p  [GeV/c];Fake Rate");
  fpk->SetTitle("PID fake rate (L_{p} > L_{K});p  [GeV/c];Fake Rate");

  // write out efficiencies
  epi->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  epi->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible"));
  epi->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  epi->Write();

  ek->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  ek->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible"));
  ek->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  ek->Write();

  ep->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  ep->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible"));
  ep->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  ep->Write();

  epk->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  epk->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible"));
  epk->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  epk->Write();

  // write out fake rates
  fpi->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  fpi->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible"));
  fpi->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  fpi->Write();

  fk->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  fk->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible"));
  fk->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  fk->Write();

  fp->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  fp->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible"));
  fp->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  fp->Write();

  fpk->GetListOfFunctions()->Add(new TNamed("Description", epi->GetTitle()));
  fpk->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible"));
  fpk->GetListOfFunctions()->Add(new TNamed("Contact","jvbennet@olemiss.edu"));
  fpk->Write();

  outfile->Close();
}
