/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <iostream>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"

using namespace std;

// Constructor functions to plot histograms and their difference
void drawHist(const string& fileName, TFile* file1, TFile* file2, const string& name1, const string& name2, const string& axisName, const int opt);
void DrawUnderOverflow(TH1F *h, string opt);

void plotComparisonBASF1vs2()
{
  // Remove StatBox
  TStyle* m_gStyle = new TStyle();
  m_gStyle->SetOptStat(0);
  
  // ADD BASF1 FILE
  TFile *gBASF = new TFile("BASFMonitors-testSample.root"); 

  // ADD BASF2 FILES
  TFile *gNeutralsGamma = new TFile("b2biiGammaConversionMonitors.root");
  TFile *gNeutralsPi0 = new TFile("b2biiPi0ConversionMonitors.root");
  TFile *gNeutralsMCPi0 = new TFile("b2biiMCPi0ConversionMonitors.root");

  TFile *gBeamParams = new TFile("b2biiBeamParamsConversionMonitors.root");
  TFile *gKSMonitors = new TFile("b2biiKshortConversionMonitors.root");
  TFile *gKLMonitors = new TFile("b2biiKlongConversionMonitors.root");
  TFile *gKSAVFMonitors = new TFile("b2biiKshortConversionMonitors-AVF.root");
  TFile *gL0Monitors = new TFile("b2biiLambda0ConversionMonitors.root");
  TFile *gL0AVFMonitors = new TFile("b2biiLambda0ConversionMonitors-AVF.root");
  TFile *gGammaV0Monitors = new TFile("b2biiConvertedPhotonConversionMonitors.root");
  TFile *gGammaV0AVFMonitors = new TFile("b2biiConvertedPhotonConversionMonitors-AVF.root");
  
  TFile *gTracks = new TFile("b2biiTrackConversionMonitors.root");

  TFile *gMCParticleMonitors = new TFile("b2biiMCParticlesConversionMonitors.root");

  // ADD COMPARISON PLOT LINES
  // This will put all plots in the same pdf file
  // Template:
  // drawHist(name of output pdf, BASgBASF file, BASgBASF file, BASgBASF histogram, BASgBASF histogram, x-axis title, 0/1/2 -> first/intermediate/last plot in file)
 
  // ---------------------------------------------------------------------------------
  // Beam Parameters Monitor
  // ---------------------------------------------------------------------------------

  string fileName;

  if(!gBASF || !gBeamParams || gBASF->IsZombie() || gBeamParams->IsZombie())
    cout << "Error opening beam parameter monitor files!" << endl;
  else 
    {
      fileName = "BeamParameters-MonitorPlots";
      drawHist(fileName, gBASF, gBeamParams, "h101",   "expNum",               "Exp No.",               0);
      drawHist(fileName, gBASF, gBeamParams, "h102",   "modulo__borunNum__cm1000__bc",               "Run No. % 1000",               1);
      drawHist(fileName, gBASF, gBeamParams, "h103",   "modulo__boevtNum__cm10000__bc",              "Evt No. % 10000",              1);

      drawHist(fileName, gBASF, gBeamParams, "h104",   "Eher",              "Eher",              1);
      drawHist(fileName, gBASF, gBeamParams, "h105",   "Eler",              "Eler",              1);
      drawHist(fileName, gBASF, gBeamParams, "h106",   "Ecms",              "Ecms",              1);
      drawHist(fileName, gBASF, gBeamParams, "h107",   "XAngle",            "Xangle",            1);
      
      drawHist(fileName, gBASF, gBeamParams, "h108",   "IPX",              "IP.X",              1);
      drawHist(fileName, gBASF, gBeamParams, "h109",   "IPY",              "IP.Y",              1);
      drawHist(fileName, gBASF, gBeamParams, "h110",   "IPZ",              "IP.Z",              1);

      drawHist(fileName, gBASF, gBeamParams, "h111",   "IPCov__bo0__cm0__bc",              "IP.error[0][0]",              1);
      drawHist(fileName, gBASF, gBeamParams, "h112",   "IPCov__bo0__cm1__bc",              "IP.error[0][1]",              1);
      drawHist(fileName, gBASF, gBeamParams, "h113",   "IPCov__bo0__cm2__bc",              "IP.error[0][2]",              1);
      drawHist(fileName, gBASF, gBeamParams, "h114",   "IPCov__bo1__cm1__bc",              "IP.error[1][1]",              1);
      drawHist(fileName, gBASF, gBeamParams, "h115",   "IPCov__bo1__cm2__bc",              "IP.error[1][2]",              1);
      drawHist(fileName, gBASF, gBeamParams, "h116",   "IPCov__bo2__cm2__bc",              "IP.error[2][2]",              2);
    }

  // ---------------------------------------------------------------------------------
  // KS Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gKSMonitors || gBASF->IsZombie() || gKSMonitors->IsZombie())
    cout << "Error opening K-short monitor files!" << endl;
  else 
    {
      fileName = "KS-MonitorPlots";
      drawHist(fileName, gBASF, gKSMonitors, "h201",   "M",               "M (BVF)",               0);
      drawHist(fileName, gBASF, gKSMonitors, "h202",   "px",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h203",   "py",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h204",   "pz",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h205",   "E",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h206",   "x",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h207",   "y",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h208",   "z",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h212",   "daughter__bo0__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h213",   "daughter__bo0__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h214",   "daughter__bo0__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h215",   "daughter__bo0__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h216",   "daughter__bo0__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h217",   "daughter__bo0__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h218",   "daughter__bo0__cmz__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h222",   "daughter__bo1__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h223",   "daughter__bo1__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h224",   "daughter__bo1__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h225",   "daughter__bo1__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h226",   "daughter__bo1__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h227",   "daughter__bo1__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h228",   "daughter__bo1__cmz__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h291",   "extraInfo__bogoodKs__bc",              "goodKs flag",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h292",   "extraInfo__boksnbVLike__bc",           "nisKS.nbVlike",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h293",   "extraInfo__boksnbNoLam__bc",           "nisKs.nbNoLam",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h294",   "extraInfo__boksnbStandard__bc",        "nisKs.standard",              2);
    }

  if(!gBASF || !gKSAVFMonitors || gBASF->IsZombie() || gKSAVFMonitors->IsZombie())
    cout << "Error opening K-short (AVF) monitor files!" << endl;
  else 
    {
      fileName = "KS-MonitorPlots-AVF";
      drawHist(fileName, gBASF, gKSAVFMonitors, "h231",   "M",               "M (AVF)",               0);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h232",   "px",              "px (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h233",   "py",              "py (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h234",   "pz",              "pz (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h235",   "E",               "E (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h236",   "x",               "x (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h237",   "y",               "y (AVF)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h238",   "z",               "z (AVF)",              1);

      drawHist(fileName, gBASF, gKSAVFMonitors, "h295",   "chiProb",               "fit pValue",     1);

      drawHist(fileName, gBASF, gKSAVFMonitors, "h241",   "momVertCovM__bo0__cm0__bc",              "Ks.errorMatrix(0,0)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h242",   "momVertCovM__bo0__cm1__bc",              "Ks.errorMatrix(0,1)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h243",   "momVertCovM__bo0__cm2__bc",              "Ks.errorMatrix(0,2)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h244",   "momVertCovM__bo0__cm3__bc",              "Ks.errorMatrix(0,3)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h245",   "momVertCovM__bo0__cm4__bc",              "Ks.errorMatrix(0,4)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h246",   "momVertCovM__bo0__cm5__bc",              "Ks.errorMatrix(0,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h247",   "momVertCovM__bo0__cm6__bc",              "Ks.errorMatrix(0,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h248",   "momVertCovM__bo1__cm1__bc",              "Ks.errorMatrix(1,1)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h249",   "momVertCovM__bo1__cm2__bc",              "Ks.errorMatrix(1,2)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h250",   "momVertCovM__bo1__cm3__bc",              "Ks.errorMatrix(1,3)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h251",   "momVertCovM__bo1__cm4__bc",              "Ks.errorMatrix(1,4)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h252",   "momVertCovM__bo1__cm5__bc",              "Ks.errorMatrix(1,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h253",   "momVertCovM__bo1__cm6__bc",              "Ks.errorMatrix(1,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h254",   "momVertCovM__bo2__cm2__bc",              "Ks.errorMatrix(2,2)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h255",   "momVertCovM__bo2__cm3__bc",              "Ks.errorMatrix(2,3)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h256",   "momVertCovM__bo2__cm4__bc",              "Ks.errorMatrix(2,4)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h257",   "momVertCovM__bo2__cm5__bc",              "Ks.errorMatrix(2,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h258",   "momVertCovM__bo2__cm6__bc",              "Ks.errorMatrix(2,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h259",   "momVertCovM__bo3__cm3__bc",              "Ks.errorMatrix(3,3)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h260",   "momVertCovM__bo3__cm4__bc",              "Ks.errorMatrix(3,4)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h261",   "momVertCovM__bo3__cm5__bc",              "Ks.errorMatrix(3,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h262",   "momVertCovM__bo3__cm6__bc",              "Ks.errorMatrix(3,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h263",   "momVertCovM__bo4__cm4__bc",              "Ks.errorMatrix(4,4)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h264",   "momVertCovM__bo4__cm5__bc",              "Ks.errorMatrix(4,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h265",   "momVertCovM__bo4__cm6__bc",              "Ks.errorMatrix(4,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h266",   "momVertCovM__bo5__cm5__bc",              "Ks.errorMatrix(5,5)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h267",   "momVertCovM__bo5__cm6__bc",              "Ks.errorMatrix(5,6)",              1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h268",   "momVertCovM__bo6__cm6__bc",              "Ks.errorMatrix(6,6)",              1);

      drawHist(fileName, gBASF, gKSAVFMonitors, "h271",   "abs__bodaughter__bo0__cmmcPDG__bc__bc",  "Ks.child(0).mcPDG",                1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h273",   "abs__bodaughter__bo1__cmmcPDG__bc__bc",  "Ks.child(1).mcPDG",                1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h272",   "daughter__bo0__cmmcP__bc",               "Ks.child(0).mcPtot",               1);
      drawHist(fileName, gBASF, gKSAVFMonitors, "h274",   "daughter__bo1__cmmcP__bc",               "Ks.child(1).mcPtot",                2);
    }

  // ---------------------------------------------------------------------------------
  // KL Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gKSAVFMonitors || gBASF->IsZombie() || gKSAVFMonitors->IsZombie())
    cout << "Error opening K-long monitor files!" << endl;
  else
    {
      fileName = "KL-MonitorPlots";
      drawHist(fileName, gBASF, gKLMonitors, "h901",   "klmClusterPositionX",       "klmClusterPositionX",               0);
      drawHist(fileName, gBASF, gKLMonitors, "h902",   "klmClusterPositionY",       "klmClusterPositionY",               1);
      drawHist(fileName, gBASF, gKLMonitors, "h903",   "klmClusterPositionZ",       "klmClusterPositionZ",               1);
      drawHist(fileName, gBASF, gKLMonitors, "h904",   "klmClusterLayers",          "klmClusterLayers",                  1);
      drawHist(fileName, gBASF, gKLMonitors, "h905",   "klmClusterInnermostLayer",  "klmClusterInnermostLayer",          1);
      drawHist(fileName, gBASF, gKLMonitors, "h906",   "mcPDG", "mcPDG",   1);
      drawHist(fileName, gBASF, gKLMonitors, "h907",   "mcE",   "mcE",     1);
      drawHist(fileName, gBASF, gKLMonitors, "h908",   "mcPX",  "mcPX",    1);
      drawHist(fileName, gBASF, gKLMonitors, "h909",   "mcPY",  "mcPY",    1);
      drawHist(fileName, gBASF, gKLMonitors, "h910",   "mcPZ",  "mcPZ",    2);

    }

  // ---------------------------------------------------------------------------------
  // Converted Photons Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gGammaV0Monitors || gBASF->IsZombie() || gGammaV0Monitors->IsZombie())
    cout << "Error opening Converted Photons monitor files!" << endl;
  else 
    {
      fileName = "Gamma-V0-MonitorPlots";
      drawHist(fileName, gBASF, gGammaV0Monitors, "h801",   "M",               "M (BVF)",               0);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h802",   "px",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h803",   "py",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h804",   "pz",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h805",   "E",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h806",   "x",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h807",   "y",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h808",   "z",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gGammaV0Monitors, "h812",   "daughter__bo0__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h813",   "daughter__bo0__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h814",   "daughter__bo0__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h815",   "daughter__bo0__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h816",   "daughter__bo0__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h817",   "daughter__bo0__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h818",   "daughter__bo0__cmz__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gGammaV0Monitors, "h822",   "daughter__bo1__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h823",   "daughter__bo1__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h824",   "daughter__bo1__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h825",   "daughter__bo1__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h826",   "daughter__bo1__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h827",   "daughter__bo1__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gGammaV0Monitors, "h828",   "daughter__bo1__cmz__bc",               "z (BVF)",              2);
    }

  if(!gBASF || !gGammaV0AVFMonitors || gBASF->IsZombie() || gGammaV0AVFMonitors->IsZombie())
    cout << "Error opening Converted Photons (AVF) monitor files!" << endl;
  else 
    {
      fileName = "Gamma-V0-MonitorPlots-AVF";
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h831",   "M",               "M (AVF)",               0);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h832",   "px",              "px (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h833",   "py",              "py (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h834",   "pz",              "pz (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h835",   "E",               "E (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h836",   "x",               "x (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h837",   "y",               "y (AVF)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h838",   "z",               "z (AVF)",              1);

      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h895",   "chiProb",               "fit pValue",     1);

      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h841",   "momVertCovM__bo0__cm0__bc",              "gammaV0.errorMatrix(0,0)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h842",   "momVertCovM__bo0__cm1__bc",              "gammaV0.errorMatrix(0,1)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h843",   "momVertCovM__bo0__cm2__bc",              "gammaV0.errorMatrix(0,2)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h844",   "momVertCovM__bo0__cm3__bc",              "gammaV0.errorMatrix(0,3)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h845",   "momVertCovM__bo0__cm4__bc",              "gammaV0.errorMatrix(0,4)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h846",   "momVertCovM__bo0__cm5__bc",              "gammaV0.errorMatrix(0,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h847",   "momVertCovM__bo0__cm6__bc",              "gammaV0.errorMatrix(0,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h848",   "momVertCovM__bo1__cm1__bc",              "gammaV0.errorMatrix(1,1)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h849",   "momVertCovM__bo1__cm2__bc",              "gammaV0.errorMatrix(1,2)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h850",   "momVertCovM__bo1__cm3__bc",              "gammaV0.errorMatrix(1,3)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h851",   "momVertCovM__bo1__cm4__bc",              "gammaV0.errorMatrix(1,4)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h852",   "momVertCovM__bo1__cm5__bc",              "gammaV0.errorMatrix(1,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h853",   "momVertCovM__bo1__cm6__bc",              "gammaV0.errorMatrix(1,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h854",   "momVertCovM__bo2__cm2__bc",              "gammaV0.errorMatrix(2,2)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h855",   "momVertCovM__bo2__cm3__bc",              "gammaV0.errorMatrix(2,3)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h856",   "momVertCovM__bo2__cm4__bc",              "gammaV0.errorMatrix(2,4)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h857",   "momVertCovM__bo2__cm5__bc",              "gammaV0.errorMatrix(2,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h858",   "momVertCovM__bo2__cm6__bc",              "gammaV0.errorMatrix(2,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h859",   "momVertCovM__bo3__cm3__bc",              "gammaV0.errorMatrix(3,3)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h860",   "momVertCovM__bo3__cm4__bc",              "gammaV0.errorMatrix(3,4)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h861",   "momVertCovM__bo3__cm5__bc",              "gammaV0.errorMatrix(3,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h862",   "momVertCovM__bo3__cm6__bc",              "gammaV0.errorMatrix(3,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h863",   "momVertCovM__bo4__cm4__bc",              "gammaV0.errorMatrix(4,4)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h864",   "momVertCovM__bo4__cm5__bc",              "gammaV0.errorMatrix(4,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h865",   "momVertCovM__bo4__cm6__bc",              "gammaV0.errorMatrix(4,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h866",   "momVertCovM__bo5__cm5__bc",              "gammaV0.errorMatrix(5,5)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h867",   "momVertCovM__bo5__cm6__bc",              "gammaV0.errorMatrix(5,6)",              1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h868",   "momVertCovM__bo6__cm6__bc",              "gammaV0.errorMatrix(6,6)",              1);

      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h871",   "abs__bodaughter__bo0__cmmcPDG__bc__bc",  "gammaV0.child(0).mcPDG",                1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h873",   "abs__bodaughter__bo1__cmmcPDG__bc__bc",  "gammaV0.child(1).mcPDG",                1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h872",   "daughter__bo0__cmmcP__bc",               "gammaV0.child(0).mcPtot",               1);
      drawHist(fileName, gBASF, gGammaV0AVFMonitors, "h874",   "daughter__bo1__cmmcP__bc",               "gammaV0.child(1).mcPtot",                2);
    }

  // ---------------------------------------------------------------------------------
  // Lambda0
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gL0Monitors || gBASF->IsZombie() || gL0Monitors->IsZombie())
    cout << "Error opening Lambda0 monitor files!" << endl;
  else 
    {
      fileName = "Lambda0-V0-MonitorPlots";
      drawHist(fileName, gBASF, gL0Monitors, "h701",   "M",               "M (BVF)",               0);
      drawHist(fileName, gBASF, gL0Monitors, "h702",   "px",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h703",   "py",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h704",   "pz",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h705",   "E",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h706",   "x",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h707",   "y",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h708",   "z",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gL0Monitors, "h712",   "daughter__bo0__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h713",   "daughter__bo0__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h714",   "daughter__bo0__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h715",   "daughter__bo0__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h716",   "daughter__bo0__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h717",   "daughter__bo0__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h718",   "daughter__bo0__cmz__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gL0Monitors, "h722",   "daughter__bo1__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h723",   "daughter__bo1__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h724",   "daughter__bo1__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h725",   "daughter__bo1__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h726",   "daughter__bo1__cmx__bc",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h727",   "daughter__bo1__cmy__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gL0Monitors, "h728",   "daughter__bo1__cmz__bc",               "z (BVF)",              2);
    }

  if(!gBASF || !gL0AVFMonitors || gBASF->IsZombie() || gL0AVFMonitors->IsZombie())
    cout << "Error opening Lambda (AVF) monitor files!" << endl;
  else 
    {
      fileName = "Lambda0-V0-MonitorPlots-AVF";
      drawHist(fileName, gBASF, gL0AVFMonitors, "h731",   "M",               "M (AVF)",               0);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h732",   "px",              "px (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h733",   "py",              "py (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h734",   "pz",              "pz (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h735",   "E",               "E (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h736",   "x",               "x (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h737",   "y",               "y (AVF)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h738",   "z",               "z (AVF)",              1);

      drawHist(fileName, gBASF, gL0AVFMonitors, "h795",   "chiProb",               "fit pValue",     1);

      drawHist(fileName, gBASF, gL0AVFMonitors, "h741",   "momVertCovM__bo0__cm0__bc",              "gammaV0.errorMatrix(0,0)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h742",   "momVertCovM__bo0__cm1__bc",              "gammaV0.errorMatrix(0,1)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h743",   "momVertCovM__bo0__cm2__bc",              "gammaV0.errorMatrix(0,2)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h744",   "momVertCovM__bo0__cm3__bc",              "gammaV0.errorMatrix(0,3)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h745",   "momVertCovM__bo0__cm4__bc",              "gammaV0.errorMatrix(0,4)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h746",   "momVertCovM__bo0__cm5__bc",              "gammaV0.errorMatrix(0,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h747",   "momVertCovM__bo0__cm6__bc",              "gammaV0.errorMatrix(0,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h748",   "momVertCovM__bo1__cm1__bc",              "gammaV0.errorMatrix(1,1)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h749",   "momVertCovM__bo1__cm2__bc",              "gammaV0.errorMatrix(1,2)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h750",   "momVertCovM__bo1__cm3__bc",              "gammaV0.errorMatrix(1,3)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h751",   "momVertCovM__bo1__cm4__bc",              "gammaV0.errorMatrix(1,4)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h752",   "momVertCovM__bo1__cm5__bc",              "gammaV0.errorMatrix(1,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h753",   "momVertCovM__bo1__cm6__bc",              "gammaV0.errorMatrix(1,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h754",   "momVertCovM__bo2__cm2__bc",              "gammaV0.errorMatrix(2,2)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h755",   "momVertCovM__bo2__cm3__bc",              "gammaV0.errorMatrix(2,3)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h756",   "momVertCovM__bo2__cm4__bc",              "gammaV0.errorMatrix(2,4)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h757",   "momVertCovM__bo2__cm5__bc",              "gammaV0.errorMatrix(2,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h758",   "momVertCovM__bo2__cm6__bc",              "gammaV0.errorMatrix(2,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h759",   "momVertCovM__bo3__cm3__bc",              "gammaV0.errorMatrix(3,3)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h760",   "momVertCovM__bo3__cm4__bc",              "gammaV0.errorMatrix(3,4)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h761",   "momVertCovM__bo3__cm5__bc",              "gammaV0.errorMatrix(3,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h762",   "momVertCovM__bo3__cm6__bc",              "gammaV0.errorMatrix(3,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h763",   "momVertCovM__bo4__cm4__bc",              "gammaV0.errorMatrix(4,4)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h764",   "momVertCovM__bo4__cm5__bc",              "gammaV0.errorMatrix(4,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h765",   "momVertCovM__bo4__cm6__bc",              "gammaV0.errorMatrix(4,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h766",   "momVertCovM__bo5__cm5__bc",              "gammaV0.errorMatrix(5,5)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h767",   "momVertCovM__bo5__cm6__bc",              "gammaV0.errorMatrix(5,6)",              1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h768",   "momVertCovM__bo6__cm6__bc",              "gammaV0.errorMatrix(6,6)",              1);

      drawHist(fileName, gBASF, gL0AVFMonitors, "h771",   "abs__bodaughter__bo0__cmmcPDG__bc__bc",  "gammaV0.child(0).mcPDG",                1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h773",   "abs__bodaughter__bo1__cmmcPDG__bc__bc",  "gammaV0.child(1).mcPDG",                1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h772",   "daughter__bo0__cmmcP__bc",               "gammaV0.child(0).mcPtot",               1);
      drawHist(fileName, gBASF, gL0AVFMonitors, "h774",   "daughter__bo1__cmmcP__bc",               "gammaV0.child(1).mcPtot",                2);
    }


  // ---------------------------------------------------------------------------------
  // Gammas Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gNeutralsGamma || gBASF->IsZombie() || gNeutralsGamma->IsZombie())
    cout << "Error opening gamma monitor files!" << endl;
  else 
    {
      fileName = "gammaMonitorPlots";
      drawHist(fileName, gBASF, gNeutralsGamma, "h301",   "x",               "X [cm]",               0);
      drawHist(fileName, gBASF, gNeutralsGamma, "h302",   "y",               "Y [cm]",               1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h303",   "z",               "Z [cm]",               1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h304",   "px",              "Px [GeV/c]",           1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h305",   "py",              "Py [GeV/c]",           1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h306",   "pz",              "Pz [GeV/c]",           1);
      
      drawHist(fileName, gBASF, gNeutralsGamma, "h307",   "mcPX",            "True_Px [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h308",   "mcPY",            "True_Py [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h309",   "mcPZ",            "True_Pz [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h310",   "mcE",             "True_E [GeV]",         1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h311",   "E",               "E [GeV]",              1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h312",   "clusterTheta",    "#theta",               1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h313",   "clusterPhi",      "#phi",                 1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h314",   "clusterR",        "R [cm]",               1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h315",   "clusterUncorrE",  "E_{depSum} [GeV]",     1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h316",   "clusterHighestE", "Highest E [GeV]",      1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h317",   "clusterE9E25",    "E_{9}/E_{25}",         1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h318",   "clusterNHits",    "N_{hits}",             1);

      drawHist(fileName, gBASF, gNeutralsGamma, "h319",   "momVertCovM__bo0__cm0__bc",    "Err00",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h320",   "momVertCovM__bo1__cm0__bc",    "Err10",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h321",   "momVertCovM__bo1__cm1__bc",    "Err11",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h322",   "momVertCovM__bo2__cm0__bc",    "Err20",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h323",   "momVertCovM__bo2__cm1__bc",    "Err21",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h324",   "momVertCovM__bo2__cm2__bc",    "Err22",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h325",   "momVertCovM__bo3__cm0__bc",    "Err30",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h326",   "momVertCovM__bo3__cm1__bc",    "Err31",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h327",   "momVertCovM__bo3__cm2__bc",    "Err32",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h328",   "momVertCovM__bo3__cm3__bc",    "Err33",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h329",   "momVertCovM__bo4__cm0__bc",    "Err40",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h330",   "momVertCovM__bo4__cm1__bc",    "Err41",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h331",   "momVertCovM__bo4__cm2__bc",    "Err42",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h332",   "momVertCovM__bo4__cm3__bc",    "Err43",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h333",   "momVertCovM__bo4__cm4__bc",    "Err44",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h334",   "momVertCovM__bo5__cm0__bc",    "Err50",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h335",   "momVertCovM__bo5__cm1__bc",    "Err51",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h336",   "momVertCovM__bo5__cm2__bc",    "Err52",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h337",   "momVertCovM__bo5__cm3__bc",    "Err53",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h338",   "momVertCovM__bo5__cm4__bc",    "Err54",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h339",   "momVertCovM__bo5__cm5__bc",    "Err55",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h340",   "momVertCovM__bo6__cm0__bc",    "Err60",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h341",   "momVertCovM__bo6__cm1__bc",    "Err61",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h342",   "momVertCovM__bo6__cm2__bc",    "Err62",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h343",   "momVertCovM__bo6__cm3__bc",    "Err63",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h344",   "momVertCovM__bo6__cm4__bc",    "Err64",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h345",   "momVertCovM__bo6__cm5__bc",    "Err65",   1);
      drawHist(fileName, gBASF, gNeutralsGamma, "h346",   "momVertCovM__bo6__cm6__bc",    "Err66",   2);
    }

  // ---------------------------------------------------------------------------------
  // Pi0 Monitor
  // ---------------------------------------------------------------------------------
  
  if(!gBASF || !gNeutralsPi0 || !gNeutralsMCPi0 || gBASF->IsZombie() || gNeutralsPi0->IsZombie() || gNeutralsMCPi0->IsZombie())
    cout << "Error opening pi0 monitor files!" << endl;
  else 
    {
      fileName = "pi0MonitorPlots";
      drawHist(fileName, gBASF, gNeutralsPi0, "h401",   "x",               "X [cm]",               0);
      drawHist(fileName, gBASF, gNeutralsPi0, "h402",   "y",               "Y [cm]",               1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h403",   "z",               "Z [cm]",               1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h404",   "px",              "Px [GeV/c]",           1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h405",   "py",              "Py [GeV/c]",           1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h406",   "pz",              "Pz [GeV/c]",           1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h407",   "E",               "E [GeV]",              1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h408",   "InvM",            "InvM [GeV/c^{2}]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h409",   "M",               "M [GeV/c^{2}]",        1);

      drawHist(fileName, gBASF, gNeutralsMCPi0, "h410",   "mcPX",            "True_Px [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsMCPi0, "h411",   "mcPY",            "True_Py [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsMCPi0, "h412",   "mcPZ",            "True_Pz [GeV/c]",      1);
      drawHist(fileName, gBASF, gNeutralsMCPi0, "h413",   "mcE",             "True_E [GeV]",         1);

      drawHist(fileName, gBASF, gNeutralsPi0, "h414",   "daughter__bo0__cmx__bc",  "d_{1}: X [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h415",   "daughter__bo0__cmy__bc",  "d_{1}: Y [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h416",   "daughter__bo0__cmz__bc",  "d_{1}: Z [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h417",   "daughter__bo0__cmpx__bc", "d_{1}: Px [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h418",   "daughter__bo0__cmpy__bc", "d_{1}: Py [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h419",   "daughter__bo0__cmpz__bc", "d_{1}: Pz [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h420",   "daughter__bo0__cmE__bc",  "d_{1}: E [GeV]",    1);

      drawHist(fileName, gBASF, gNeutralsPi0, "h421",   "daughter__bo1__cmx__bc",  "d_{2}: X [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h422",   "daughter__bo1__cmy__bc",  "d_{2}: Y [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h423",   "daughter__bo1__cmz__bc",  "d_{2}: Z [cm]",     1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h424",   "daughter__bo1__cmpx__bc", "d_{2}: Px [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h425",   "daughter__bo1__cmpy__bc", "d_{2}: Py [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h426",   "daughter__bo1__cmpz__bc", "d_{2}: Pz [GeV/c]", 1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h427",   "daughter__bo1__cmE__bc",  "d_{2}: E [GeV]",    1);

      drawHist(fileName, gBASF, gNeutralsPi0, "h428",   "momVertCovM__bo0__cm0__bc",    "Err00",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h429",   "momVertCovM__bo1__cm0__bc",    "Err10",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h430",   "momVertCovM__bo1__cm1__bc",    "Err11",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h431",   "momVertCovM__bo2__cm0__bc",    "Err20",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h432",   "momVertCovM__bo2__cm1__bc",    "Err21",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h433",   "momVertCovM__bo2__cm2__bc",    "Err22",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h434",   "momVertCovM__bo3__cm0__bc",    "Err30",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h435",   "momVertCovM__bo3__cm1__bc",    "Err31",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h436",   "momVertCovM__bo3__cm2__bc",    "Err32",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h437",   "momVertCovM__bo3__cm3__bc",    "Err33",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h438",   "momVertCovM__bo4__cm0__bc",    "Err40",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h439",   "momVertCovM__bo4__cm1__bc",    "Err41",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h440",   "momVertCovM__bo4__cm2__bc",    "Err42",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h441",   "momVertCovM__bo4__cm3__bc",    "Err43",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h442",   "momVertCovM__bo4__cm4__bc",    "Err44",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h443",   "momVertCovM__bo5__cm0__bc",    "Err50",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h444",   "momVertCovM__bo5__cm1__bc",    "Err51",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h445",   "momVertCovM__bo5__cm2__bc",    "Err52",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h446",   "momVertCovM__bo5__cm3__bc",    "Err53",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h447",   "momVertCovM__bo5__cm4__bc",    "Err54",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h448",   "momVertCovM__bo5__cm5__bc",    "Err55",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h449",   "momVertCovM__bo6__cm0__bc",    "Err60",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h450",   "momVertCovM__bo6__cm1__bc",    "Err61",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h451",   "momVertCovM__bo6__cm2__bc",    "Err62",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h452",   "momVertCovM__bo6__cm3__bc",    "Err63",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h453",   "momVertCovM__bo6__cm4__bc",    "Err64",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h454",   "momVertCovM__bo6__cm5__bc",    "Err65",   1);
      drawHist(fileName, gBASF, gNeutralsPi0, "h455",   "momVertCovM__bo6__cm6__bc",    "Err66",   2);
    }


  // ---------------------------------------------------------------------------------
  // MC Particles Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gMCParticleMonitors || gBASF->IsZombie() || gMCParticleMonitors->IsZombie())
    cout << "Error opening MC Particle monitor files!" << endl;
  else 
    {
      fileName = "MCParticleMonitorPlots";
      drawHist(fileName, gBASF, gMCParticleMonitors, "h501",   "mcPDG",           "",                     0);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h502",   "mcM",             "M [GeV/c^{2}]",        1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h503",   "mcPx",            "Px [GeV/c]",           1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h504",   "mcPy",            "Py [GeV/c]",           1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h505",   "mcPz",            "Pz [GeV/c]",           1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h506",   "mcE",             "E [GeV]",              1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h507",   "mcVx",            "x [cm]",               1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h508",   "mcVy",            "y [cm]",               1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h509",   "mcVz",            "z [cm]",               1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h510",   "mcPiPlusMother",  "",                     1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h511",   "mcPiMinusMother", "",                     1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h512",   "mcPi0Mother",     "",                     1);
      drawHist(fileName, gBASF, gMCParticleMonitors, "h513",   "mcNDau",          "",                     2);
    } 

  // ---------------------------------------------------------------------------------
  // Track Monitor
  // ---------------------------------------------------------------------------------

  if(!gBASF || !gTracks || gBASF->IsZombie() || gTracks->IsZombie())
    cout << "Error opening track monitor files!" << endl;
  else 
    {      
      fileName = "TrackMonitorPlots";
      drawHist(fileName, gBASF, gTracks, "h601", "atcPIDBelle__bo3__cm2__bc", "PID: kapi",    0);
      drawHist(fileName, gBASF, gTracks, "h602", "atcPIDBelle__bo4__cm3__bc", "PID: prKa",    1);
      drawHist(fileName, gBASF, gTracks, "h603", "atcPIDBelle__bo4__cm2__bc", "PID: prpi",    1);
      drawHist(fileName, gBASF, gTracks, "h604", "eIDBelle",                  "PID: e",       1);
      drawHist(fileName, gBASF, gTracks, "h605", "muIDBelle",                 "PID: mu",      1);
      drawHist(fileName, gBASF, gTracks, "h606", "muIDBelleQuality",          "muID Quality", 1);

      drawHist(fileName, gBASF, gTracks, "h611", "px",                        "Px [GeV/c]",   1);
      drawHist(fileName, gBASF, gTracks, "h612", "py",                        "Py [GeV/c]",   1);
      drawHist(fileName, gBASF, gTracks, "h613", "pz",                        "Pz [GeV/c]",   1);
      drawHist(fileName, gBASF, gTracks, "h614", "E",                         "E [GeV]",      1);
      drawHist(fileName, gBASF, gTracks, "h615", "x",                         "X [cm]",       1);
      drawHist(fileName, gBASF, gTracks, "h616", "y",                         "Y [cm]",       1);
      drawHist(fileName, gBASF, gTracks, "h617", "z",                         "Z [cm]",       1);

      drawHist(fileName, gBASF, gTracks, "h621", "momVertCovM__bo0__cm0__bc", "Err00",        1);
      drawHist(fileName, gBASF, gTracks, "h622", "momVertCovM__bo0__cm1__bc", "Err10",        1);
      drawHist(fileName, gBASF, gTracks, "h623", "momVertCovM__bo0__cm2__bc", "Err11",        1);
      drawHist(fileName, gBASF, gTracks, "h624", "momVertCovM__bo0__cm3__bc", "Err20",        1);
      drawHist(fileName, gBASF, gTracks, "h625", "momVertCovM__bo0__cm4__bc", "Err21",        1);
      drawHist(fileName, gBASF, gTracks, "h626", "momVertCovM__bo0__cm5__bc", "Err22",        1);
      drawHist(fileName, gBASF, gTracks, "h627", "momVertCovM__bo0__cm6__bc", "Err30",        1);
      drawHist(fileName, gBASF, gTracks, "h628", "momVertCovM__bo1__cm1__bc", "Err31",        1);
      drawHist(fileName, gBASF, gTracks, "h629", "momVertCovM__bo1__cm2__bc", "Err32",        1);
      drawHist(fileName, gBASF, gTracks, "h630", "momVertCovM__bo1__cm3__bc", "Err33",        1);
      drawHist(fileName, gBASF, gTracks, "h631", "momVertCovM__bo1__cm4__bc", "Err40",        1);
      drawHist(fileName, gBASF, gTracks, "h632", "momVertCovM__bo1__cm5__bc", "Err41",        1);
      drawHist(fileName, gBASF, gTracks, "h633", "momVertCovM__bo1__cm6__bc", "Err42",        1);
      drawHist(fileName, gBASF, gTracks, "h634", "momVertCovM__bo2__cm2__bc", "Err43",        1);
      drawHist(fileName, gBASF, gTracks, "h635", "momVertCovM__bo2__cm3__bc", "Err44",        1);
      drawHist(fileName, gBASF, gTracks, "h636", "momVertCovM__bo2__cm4__bc", "Err50",        1);
      drawHist(fileName, gBASF, gTracks, "h637", "momVertCovM__bo2__cm5__bc", "Err51",        1);
      drawHist(fileName, gBASF, gTracks, "h638", "momVertCovM__bo2__cm6__bc", "Err52",        1);
      drawHist(fileName, gBASF, gTracks, "h639", "momVertCovM__bo3__cm3__bc", "Err53",        1);
      drawHist(fileName, gBASF, gTracks, "h640", "momVertCovM__bo3__cm4__bc", "Err54",        1);
      drawHist(fileName, gBASF, gTracks, "h641", "momVertCovM__bo3__cm5__bc", "Err55",        1);
      drawHist(fileName, gBASF, gTracks, "h642", "momVertCovM__bo3__cm6__bc", "Err60",        1);
      drawHist(fileName, gBASF, gTracks, "h643", "momVertCovM__bo4__cm4__bc", "Err61",        1);
      drawHist(fileName, gBASF, gTracks, "h644", "momVertCovM__bo4__cm5__bc", "Err62",        1);
      drawHist(fileName, gBASF, gTracks, "h645", "momVertCovM__bo4__cm6__bc", "Err63",        1);
      drawHist(fileName, gBASF, gTracks, "h646", "momVertCovM__bo5__cm5__bc", "Err64",        1);
      drawHist(fileName, gBASF, gTracks, "h647", "momVertCovM__bo5__cm6__bc", "Err65",        1);
      drawHist(fileName, gBASF, gTracks, "h648", "momVertCovM__bo6__cm6__bc", "Err66",        2);
    }


  exit(1);
  //gApplication->Terminate();

}

void drawHist(const string& fileName, TFile* file1, TFile* file2, const string& name1, const string& name2, const string& axisName, const int opt)
{
  // Create canvas
  TCanvas *C = new TCanvas("c1","c1",1000,600);

  // Load histograms
  TH1F *h1 = (TH1F*) file1->Get(name1.c_str());
  TH1F *h2 = (TH1F*) file2->Get(name2.c_str());

  if(!h1 || !h2) 
    cout << "WARNING! Did not find histogram: " << name1 << endl;
  else if(h1->GetSize() != h2->GetSize() || h1->GetBinWidth(0) != h2->GetBinWidth(0))
    cout << "WARNING! Histograms with different binnings: " << name1 << endl;
  else
    {
      // Paint histograms
      h1->GetXaxis()->SetTitle(axisName.c_str());
      h1->SetFillStyle(3004);
      h1->SetFillColor(kBlue);
      h2->SetFillStyle(3005);
      h2->SetFillColor(kRed);
      h1->SetLineColor(kBlack);
      h2->SetLineColor(kBlack);

      // Title for PDF
      string title;
      string t = "Title:";
      title = t+h1->GetTitle();

      // Create legend
      TLegend *leg = new TLegend(0.6,0.7,0.8,0.9);
      leg->AddEntry(h1,"basf1","f");
      leg->AddEntry(h2,"basf2","f");

      C->cd();

      // Create top part (both histograms)
      TPad *pad1 = new TPad("pad1","pad1",0,0.2,1,1);
      pad1->SetBottomMargin(-0.1);
      pad1->Draw();
      pad1->cd();

      DrawUnderOverflow(h1,"HIST");
      DrawUnderOverflow(h2, "HIST SAME");
      leg->Draw();

      C->cd();

      // Create bottom part (difference)
      TPad *pad2 = new TPad("pad2","pad2",0,0,1,0.2);
      pad2->SetTopMargin(0.1);
      pad2->Draw();
      pad2->cd();

      // BASgBASF - BASgBASF
      h1->Add(h2,-1);

      // Set bottom histogram properties
      h1->SetTitle("");
      h1->SetXTitle("");
      h1->GetXaxis()->SetLabelSize(0);
      h1->GetYaxis()->SetLabelFont(63);
      h1->GetYaxis()->SetLabelSize(10);
      h1->SetLabelSize(10);
      h1->GetXaxis()->SetLabelOffset(999);
      h1->GetYaxis()->SetNdivisions(205);

      // Set bottom histogram range
      double maxy = h1->GetMaximum() > h1->GetBinContent(h1->GetNbinsX()+1) ? h1->GetMaximum() : h1->GetBinContent(h1->GetNbinsX()+1);
      double miny = h1->GetMinimum() < h1->GetBinContent(0) ? h1->GetMinimum() : h1->GetBinContent(0);
      double delta = TMath::Max(maxy - miny,1.0);

      h1->SetAxisRange(miny - 0.1*delta,maxy + 0.1*delta ,"Y");
      DrawUnderOverflow(h1,"HIST");

      // Set PDF file name
      string fName;
      string pdf = ".pdf";
      
      fName = fileName + pdf;

      if(opt == 0)
	fName = fName + "(";
      else if(opt == 2)
	fName = fName + ")";
      
      // Save histogram as PDF
      C->Print(fName.c_str(), title.c_str());

      // Delete instances of new
      delete C;
      delete leg;
      delete h1;
      delete h2;
    }
}

void DrawUnderOverflow(TH1F *h, string opt)
{
  // This function paint the histogram h with an extra bin for overflows

  //const char* title = h->GetTitle();
  Int_t nxold = h->GetNbinsX();
  Int_t nx    = h->GetNbinsX()+2;
  Double_t bw = h->GetBinWidth(0);
  Double_t x1 = h->GetBinLowEdge(1)-bw;
  Double_t x2 = h->GetBinLowEdge(nxold)+2*bw;

  // Book a temporary histogram having an extra bin for overflows
  TH1F *htmp1 = (TH1F*) h->Clone("htmp1");
  TH1F *htmp2 = (TH1F*) h->Clone("htmp2");

  htmp1->Reset();
  htmp2->Reset();
  htmp1->SetBins(nx, x1, x2);
  htmp2->SetBins(nx, x1, x2);

  // Fill the new histogram
  for (Int_t i=1; i<=nxold; i++) 
    htmp1->Fill(htmp1->GetBinCenter(i+1), h->GetBinContent(i));
   
  // Fill under/overflow bins
  htmp2->Fill(htmp2->GetBinCenter(1), h->GetBinContent(0));
  htmp2->Fill(htmp2->GetBinCenter(nx), h->GetBinContent(nxold+1));

  // Draw the temporary histogram
  htmp1->DrawCopy(opt.c_str());
  htmp2->DrawCopy("HIST SAME");
}
