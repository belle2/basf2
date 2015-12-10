#include <iostream>
using namespace std;

// Constructor functions to plot histograms and their difference
void drawHist(string fileName, TFile* file1, TFile* file2, string name1, string name2, string axisName, int opt);

void plotComparisonBASF1vs2()
{
  // Remove StatBox
  gStyle->SetOptStat(0);
  
  // ADD BASF1 FILES
  TFile *f1 = new TFile("ntuple-Neutrals.root");
  TFile *f2 = new TFile("Belle1-Tracks.root");
  TFile *gBASF = new TFile("BASFMonitors-testSample.root"); 


  // ADD BASF2 FILES
  TFile *g1 = new TFile("b2biiGammaConversionMonitors.root");
  TFile *g1p1 = new TFile("b2biiPi0ConversionMonitors.root");
  TFile *g1p2 = new TFile("b2biiMCPi0ConversionMonitors.root");

  TFile *gBeamParams = new TFile("b2biiBeamParamsConversionMonitors.root");
  TFile *gKSMonitors = new TFile("b2biiKshortConversionMonitors.root");
  TFile *gKSAVFMonitors = new TFile("b2biiKshortConversionMonitors-AVF.root");
  
  TFile *gTracks = new TFile("b2biiTrackConversionMonitors.root");

  // ADD COMPARISON PLOT LINES
  // This will put all plots in the same pdf file
  // Template:
  // drawHist(name of output pdf, BASF1 file, BASF2 file, BASF1 histogram, BASF2 histogram, x-axis title, 0/1/2 -> first/intermediate/last plot in file)
  
  
  // ---------------------------------------------------------------------------------
  // Gamma Monitor
  // ---------------------------------------------------------------------------------

  string fileName;

  if(gBASF->IsZombie() || gBeamParams->IsZombie())
    cout << "Error opening files!" << endl;
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

  if(gBASF->IsZombie() || gKSMonitors->IsZombie())
    cout << "Error opening files!" << endl;
  else 
    {
      fileName = "KS-MonitorPlots";
      drawHist(fileName, gBASF, gKSMonitors, "h201",   "M",               "M (BVF)",               0);
      drawHist(fileName, gBASF, gKSMonitors, "h202",   "px",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h203",   "py",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h204",   "pz",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h205",   "E",              "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h206",   "x",               "x (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h207",   "y",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h208",   "z",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h212",   "daughter__bo0__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h213",   "daughter__bo0__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h214",   "daughter__bo0__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h215",   "daughter__bo0__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h216",   "daughter__bo0__cmx__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h217",   "daughter__bo0__cmy__bc",               "z (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h218",   "daughter__bo0__cmy__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h222",   "daughter__bo1__cmpx__bc",              "px (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h223",   "daughter__bo1__cmpy__bc",              "py (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h224",   "daughter__bo1__cmpz__bc",              "pz (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h225",   "daughter__bo1__cmE__bc",               "E (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h226",   "daughter__bo1__cmx__bc",               "y (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h227",   "daughter__bo1__cmy__bc",               "z (BVF)",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h228",   "daughter__bo1__cmy__bc",               "z (BVF)",              1);

      drawHist(fileName, gBASF, gKSMonitors, "h291",   "extraInfo__bogoodKs__bc",              "goodKs flag",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h292",   "extraInfo__boksnbVLike__bc",           "nisKS.nbVlike",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h293",   "extraInfo__boksnbNoLam__bc",           "nisKs.nbNoLam",              1);
      drawHist(fileName, gBASF, gKSMonitors, "h294",   "extraInfo__boksnbStandard__bc",        "nisKs.standard",              2);
    }

  if(f1->IsZombie() or g1->IsZombie())
    cout << "Error opening files!" << endl;
  else 
    {
      fileName = "gammaMonitorPlots";
      drawHist(fileName, f1, g1, "h301",   "x",               "X [cm]",               0);
      drawHist(fileName, f1, g1, "h302",   "y",               "Y [cm]",               1);
      drawHist(fileName, f1, g1, "h303",   "z",               "Z [cm]",               1);
      drawHist(fileName, f1, g1, "h304",   "px",              "Px [GeV/c]",           1);
      drawHist(fileName, f1, g1, "h305",   "py",              "Py [GeV/c]",           1);
      drawHist(fileName, f1, g1, "h306",   "pz",              "Pz [GeV/c]",           1);
      
      drawHist(fileName, f1, g1, "h307",   "mcPX",            "True_Px [GeV/c]",      1);
      drawHist(fileName, f1, g1, "h308",   "mcPY",            "True_Py [GeV/c]",      1);
      drawHist(fileName, f1, g1, "h309",   "mcPZ",            "True_Pz [GeV/c]",      1);
      drawHist(fileName, f1, g1, "h310",   "mcE",             "True_E [GeV]",         1);
      drawHist(fileName, f1, g1, "h311",   "E",               "E [GeV]",              1);
      drawHist(fileName, f1, g1, "h312",   "clusterTheta",    "#theta",               1);
      drawHist(fileName, f1, g1, "h313",   "clusterPhi",      "#phi",                 1);
      drawHist(fileName, f1, g1, "h314",   "clusterR",        "R [cm]",               1);
      drawHist(fileName, f1, g1, "h315",   "clusterUncorrE",  "E_{depSum} [GeV]",     1);
      drawHist(fileName, f1, g1, "h316",   "clusterHighestE", "Highest E [GeV]",      1);
      drawHist(fileName, f1, g1, "h317",   "clusterE9E25",    "E_{9}/E_{25}",         1);
      drawHist(fileName, f1, g1, "h318",   "clusterNHits",    "N_{hits}",             1);

      drawHist(fileName, f1, g1, "h319",   "momVertCovM__bo0__cm0__bc",    "Err00",   1);
      drawHist(fileName, f1, g1, "h320",   "momVertCovM__bo1__cm0__bc",    "Err10",   1);
      drawHist(fileName, f1, g1, "h321",   "momVertCovM__bo1__cm1__bc",    "Err11",   1);
      drawHist(fileName, f1, g1, "h322",   "momVertCovM__bo2__cm0__bc",    "Err20",   1);
      drawHist(fileName, f1, g1, "h323",   "momVertCovM__bo2__cm1__bc",    "Err21",   1);
      drawHist(fileName, f1, g1, "h324",   "momVertCovM__bo2__cm2__bc",    "Err22",   1);
      drawHist(fileName, f1, g1, "h325",   "momVertCovM__bo3__cm0__bc",    "Err30",   1);
      drawHist(fileName, f1, g1, "h326",   "momVertCovM__bo3__cm1__bc",    "Err31",   1);
      drawHist(fileName, f1, g1, "h327",   "momVertCovM__bo3__cm2__bc",    "Err32",   1);
      drawHist(fileName, f1, g1, "h328",   "momVertCovM__bo3__cm3__bc",    "Err33",   1);
      drawHist(fileName, f1, g1, "h329",   "momVertCovM__bo4__cm0__bc",    "Err40",   1);
      drawHist(fileName, f1, g1, "h330",   "momVertCovM__bo4__cm1__bc",    "Err41",   1);
      drawHist(fileName, f1, g1, "h331",   "momVertCovM__bo4__cm2__bc",    "Err42",   1);
      drawHist(fileName, f1, g1, "h332",   "momVertCovM__bo4__cm3__bc",    "Err43",   1);
      drawHist(fileName, f1, g1, "h333",   "momVertCovM__bo4__cm4__bc",    "Err44",   1);
      drawHist(fileName, f1, g1, "h334",   "momVertCovM__bo5__cm0__bc",    "Err50",   1);
      drawHist(fileName, f1, g1, "h335",   "momVertCovM__bo5__cm1__bc",    "Err51",   1);
      drawHist(fileName, f1, g1, "h336",   "momVertCovM__bo5__cm2__bc",    "Err52",   1);
      drawHist(fileName, f1, g1, "h337",   "momVertCovM__bo5__cm3__bc",    "Err53",   1);
      drawHist(fileName, f1, g1, "h338",   "momVertCovM__bo5__cm4__bc",    "Err54",   1);
      drawHist(fileName, f1, g1, "h339",   "momVertCovM__bo5__cm5__bc",    "Err55",   1);
      drawHist(fileName, f1, g1, "h340",   "momVertCovM__bo6__cm0__bc",    "Err60",   1);
      drawHist(fileName, f1, g1, "h341",   "momVertCovM__bo6__cm1__bc",    "Err61",   1);
      drawHist(fileName, f1, g1, "h342",   "momVertCovM__bo6__cm2__bc",    "Err62",   1);
      drawHist(fileName, f1, g1, "h343",   "momVertCovM__bo6__cm3__bc",    "Err63",   1);
      drawHist(fileName, f1, g1, "h344",   "momVertCovM__bo6__cm4__bc",    "Err64",   1);
      drawHist(fileName, f1, g1, "h345",   "momVertCovM__bo6__cm5__bc",    "Err65",   1);
      drawHist(fileName, f1, g1, "h346",   "momVertCovM__bo6__cm6__bc",    "Err66",   2);
    }

  // ---------------------------------------------------------------------------------
  // Pi0 Monitor
  // ---------------------------------------------------------------------------------
  
  if(f1->IsZombie() or g1p1->IsZombie() or g1p2->IsZombie())
    cout << "Error opening files!" << endl;
  else 
    {
      fileName = "pi0MonitorPlots";
      drawHist(fileName, f1, g1p1, "h401",   "x",               "X [cm]",               0);
      drawHist(fileName, f1, g1p1, "h402",   "y",               "Y [cm]",               1);
      drawHist(fileName, f1, g1p1, "h403",   "z",               "Z [cm]",               1);
      drawHist(fileName, f1, g1p1, "h404",   "px",              "Px [GeV/c]",           1);
      drawHist(fileName, f1, g1p1, "h405",   "py",              "Py [GeV/c]",           1);
      drawHist(fileName, f1, g1p1, "h406",   "pz",              "Pz [GeV/c]",           1);
      drawHist(fileName, f1, g1p1, "h407",   "E",               "E [GeV]",              1);
      drawHist(fileName, f1, g1p1, "h408",   "InvM",            "InvM [GeV/c^{2}]",     1);
      drawHist(fileName, f1, g1p1, "h409",   "M",               "M [GeV/c^{2}]",        1);

      drawHist(fileName, f1, g1p2, "h410",   "mcPX",            "True_Px [GeV/c]",      1);
      drawHist(fileName, f1, g1p2, "h411",   "mcPY",            "True_Py [GeV/c]",      1);
      drawHist(fileName, f1, g1p2, "h412",   "mcPZ",            "True_Pz [GeV/c]",      1);
      drawHist(fileName, f1, g1p2, "h413",   "mcE",             "True_E [GeV]",         1);

      drawHist(fileName, f1, g1p1, "h414",   "daughter__bo0__cmx__bc",  "d_{1}: X [cm]",     1);
      drawHist(fileName, f1, g1p1, "h415",   "daughter__bo0__cmy__bc",  "d_{1}: Y [cm]",     1);
      drawHist(fileName, f1, g1p1, "h416",   "daughter__bo0__cmz__bc",  "d_{1}: Z [cm]",     1);
      drawHist(fileName, f1, g1p1, "h417",   "daughter__bo0__cmpx__bc", "d_{1}: Px [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h418",   "daughter__bo0__cmpy__bc", "d_{1}: Py [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h419",   "daughter__bo0__cmpz__bc", "d_{1}: Pz [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h420",   "daughter__bo0__cmE__bc",  "d_{1}: E [GeV]",    1);

      drawHist(fileName, f1, g1p1, "h421",   "daughter__bo1__cmx__bc",  "d_{2}: X [cm]",     1);
      drawHist(fileName, f1, g1p1, "h422",   "daughter__bo1__cmy__bc",  "d_{2}: Y [cm]",     1);
      drawHist(fileName, f1, g1p1, "h423",   "daughter__bo1__cmz__bc",  "d_{2}: Z [cm]",     1);
      drawHist(fileName, f1, g1p1, "h424",   "daughter__bo1__cmpx__bc", "d_{2}: Px [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h425",   "daughter__bo1__cmpy__bc", "d_{2}: Py [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h426",   "daughter__bo1__cmpz__bc", "d_{2}: Pz [GeV/c]", 1);
      drawHist(fileName, f1, g1p1, "h427",   "daughter__bo1__cmE__bc",  "d_{2}: E [GeV]",    1);

      drawHist(fileName, f1, g1p1, "h428",   "momVertCovM__bo0__cm0__bc",    "Err00",   1);
      drawHist(fileName, f1, g1p1, "h429",   "momVertCovM__bo1__cm0__bc",    "Err10",   1);
      drawHist(fileName, f1, g1p1, "h430",   "momVertCovM__bo1__cm1__bc",    "Err11",   1);
      drawHist(fileName, f1, g1p1, "h431",   "momVertCovM__bo2__cm0__bc",    "Err20",   1);
      drawHist(fileName, f1, g1p1, "h432",   "momVertCovM__bo2__cm1__bc",    "Err21",   1);
      drawHist(fileName, f1, g1p1, "h433",   "momVertCovM__bo2__cm2__bc",    "Err22",   1);
      drawHist(fileName, f1, g1p1, "h434",   "momVertCovM__bo3__cm0__bc",    "Err30",   1);
      drawHist(fileName, f1, g1p1, "h435",   "momVertCovM__bo3__cm1__bc",    "Err31",   1);
      drawHist(fileName, f1, g1p1, "h436",   "momVertCovM__bo3__cm2__bc",    "Err32",   1);
      drawHist(fileName, f1, g1p1, "h437",   "momVertCovM__bo3__cm3__bc",    "Err33",   1);
      drawHist(fileName, f1, g1p1, "h438",   "momVertCovM__bo4__cm0__bc",    "Err40",   1);
      drawHist(fileName, f1, g1p1, "h439",   "momVertCovM__bo4__cm1__bc",    "Err41",   1);
      drawHist(fileName, f1, g1p1, "h440",   "momVertCovM__bo4__cm2__bc",    "Err42",   1);
      drawHist(fileName, f1, g1p1, "h441",   "momVertCovM__bo4__cm3__bc",    "Err43",   1);
      drawHist(fileName, f1, g1p1, "h442",   "momVertCovM__bo4__cm4__bc",    "Err44",   1);
      drawHist(fileName, f1, g1p1, "h443",   "momVertCovM__bo5__cm0__bc",    "Err50",   1);
      drawHist(fileName, f1, g1p1, "h444",   "momVertCovM__bo5__cm1__bc",    "Err51",   1);
      drawHist(fileName, f1, g1p1, "h445",   "momVertCovM__bo5__cm2__bc",    "Err52",   1);
      drawHist(fileName, f1, g1p1, "h446",   "momVertCovM__bo5__cm3__bc",    "Err53",   1);
      drawHist(fileName, f1, g1p1, "h447",   "momVertCovM__bo5__cm4__bc",    "Err54",   1);
      drawHist(fileName, f1, g1p1, "h448",   "momVertCovM__bo5__cm5__bc",    "Err55",   1);
      drawHist(fileName, f1, g1p1, "h449",   "momVertCovM__bo6__cm0__bc",    "Err60",   1);
      drawHist(fileName, f1, g1p1, "h450",   "momVertCovM__bo6__cm1__bc",    "Err61",   1);
      drawHist(fileName, f1, g1p1, "h451",   "momVertCovM__bo6__cm2__bc",    "Err62",   1);
      drawHist(fileName, f1, g1p1, "h452",   "momVertCovM__bo6__cm3__bc",    "Err63",   1);
      drawHist(fileName, f1, g1p1, "h453",   "momVertCovM__bo6__cm4__bc",    "Err64",   1);
      drawHist(fileName, f1, g1p1, "h454",   "momVertCovM__bo6__cm5__bc",    "Err65",   1);
      drawHist(fileName, f1, g1p1, "h455",   "momVertCovM__bo6__cm6__bc",    "Err66",   2);
    }

  // ---------------------------------------------------------------------------------
  // Track Monitor
  // ---------------------------------------------------------------------------------

  if(f2->IsZombie() or gTracks->IsZombie())
    cout << "Error opening files!" << endl;
  else 
    {
      fileName = "TrackMonitorPlots";
      drawHist(fileName, f2, gTracks, "h501", "atcPIDBelle__bo3__cm2__bc", "PID: kapi",    0);
      drawHist(fileName, f2, gTracks, "h502", "atcPIDBelle__bo4__cm3__bc", "PID: prKa",    1);
      drawHist(fileName, f2, gTracks, "h503", "atcPIDBelle__bo4__cm2__bc", "PID: prpi",    1);
      drawHist(fileName, f2, gTracks, "h504", "eIDBelle",                  "PID: e",       1);
      drawHist(fileName, f2, gTracks, "h505", "muIDBelle",                 "PID: mu",      1);
      drawHist(fileName, f2, gTracks, "h506", "muIDBelleQuality",          "muID Quality", 1);

      drawHist(fileName, f2, gTracks, "h511", "px",                        "Px [GeV/c]",   1);
      drawHist(fileName, f2, gTracks, "h512", "py",                        "Py [GeV/c]",   1);
      drawHist(fileName, f2, gTracks, "h513", "pz",                        "Pz [GeV/c]",   1);
      drawHist(fileName, f2, gTracks, "h514", "E",                         "E [GeV]",      1);
      drawHist(fileName, f2, gTracks, "h515", "x",                         "X [cm]",       1);
      drawHist(fileName, f2, gTracks, "h516", "y",                         "Y [cm]",       1);
      drawHist(fileName, f2, gTracks, "h517", "z",                         "Z [cm]",       1);

      drawHist(fileName, f2, gTracks, "h521", "momVertCovM__bo0__cm0__bc", "Err00",        1);
      drawHist(fileName, f2, gTracks, "h522", "momVertCovM__bo0__cm1__bc", "Err10",        1);
      drawHist(fileName, f2, gTracks, "h523", "momVertCovM__bo0__cm2__bc", "Err11",        1);
      drawHist(fileName, f2, gTracks, "h524", "momVertCovM__bo0__cm3__bc", "Err20",        1);
      drawHist(fileName, f2, gTracks, "h525", "momVertCovM__bo0__cm4__bc", "Err21",        1);
      drawHist(fileName, f2, gTracks, "h526", "momVertCovM__bo0__cm5__bc", "Err22",        1);
      drawHist(fileName, f2, gTracks, "h527", "momVertCovM__bo0__cm6__bc", "Err30",        1);
      drawHist(fileName, f2, gTracks, "h528", "momVertCovM__bo1__cm1__bc", "Err31",        1);
      drawHist(fileName, f2, gTracks, "h529", "momVertCovM__bo1__cm2__bc", "Err32",        1);
      drawHist(fileName, f2, gTracks, "h530", "momVertCovM__bo1__cm3__bc", "Err33",        1);
      drawHist(fileName, f2, gTracks, "h531", "momVertCovM__bo1__cm4__bc", "Err40",        1);
      drawHist(fileName, f2, gTracks, "h532", "momVertCovM__bo1__cm5__bc", "Err41",        1);
      drawHist(fileName, f2, gTracks, "h533", "momVertCovM__bo1__cm6__bc", "Err42",        1);
      drawHist(fileName, f2, gTracks, "h534", "momVertCovM__bo2__cm2__bc", "Err43",        1);
      drawHist(fileName, f2, gTracks, "h535", "momVertCovM__bo2__cm3__bc", "Err44",        1);
      drawHist(fileName, f2, gTracks, "h536", "momVertCovM__bo2__cm4__bc", "Err50",        1);
      drawHist(fileName, f2, gTracks, "h537", "momVertCovM__bo2__cm5__bc", "Err51",        1);
      drawHist(fileName, f2, gTracks, "h538", "momVertCovM__bo2__cm6__bc", "Err52",        1);
      drawHist(fileName, f2, gTracks, "h539", "momVertCovM__bo3__cm3__bc", "Err53",        1);
      drawHist(fileName, f2, gTracks, "h540", "momVertCovM__bo3__cm4__bc", "Err54",        1);
      drawHist(fileName, f2, gTracks, "h541", "momVertCovM__bo3__cm5__bc", "Err55",        1);
      drawHist(fileName, f2, gTracks, "h542", "momVertCovM__bo3__cm6__bc", "Err60",        1);
      drawHist(fileName, f2, gTracks, "h543", "momVertCovM__bo4__cm4__bc", "Err61",        1);
      drawHist(fileName, f2, gTracks, "h544", "momVertCovM__bo4__cm5__bc", "Err62",        1);
      drawHist(fileName, f2, gTracks, "h545", "momVertCovM__bo4__cm6__bc", "Err63",        1);
      drawHist(fileName, f2, gTracks, "h546", "momVertCovM__bo5__cm5__bc", "Err64",        1);
      drawHist(fileName, f2, gTracks, "h547", "momVertCovM__bo5__cm6__bc", "Err65",        1);
      drawHist(fileName, f2, gTracks, "h548", "momVertCovM__bo6__cm6__bc", "Err66",        2);
    }


  gApplication->Terminate();

}

void drawHist(string fileName, TFile* file1, TFile* file2, string name1, string name2, string axisName, int opt)
{
  // Create canvas
  TCanvas *C = new TCanvas("c1","c1",1000,600);

  // Load histograms
  h1 = (TH1D*) file1->Get(name1.c_str());
  h2 = (TH1D*) file2->Get(name2.c_str());

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
  ostringstream tempTitle;
  tempTitle << "Title:" << h1->GetTitle();
  title = tempTitle.str();
  
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

  h1->DrawCopy("");
  h2->DrawCopy("same");
  leg->Draw();

  C->cd();

  // Create bottom part (difference)
  TPad *pad2 = new TPad("pad2","pad2",0,0,1,0.2);
  pad2->SetTopMargin(0.1);
  pad2->Draw();
  pad2->cd();

  // BASF1 - BASF2
  h1->Add(h2,-1);

  // Set bottom histogram properties
  h1->SetTitle("");
  h1->SetXTitle("");
  h1->GetXaxis()->SetLabelFont(63);
  h1->GetXaxis()->SetLabelSize(10);
  h1->GetYaxis()->SetLabelFont(63);
  h1->GetYaxis()->SetLabelSize(10);
  h1->SetLabelSize(10);
  h1->GetXaxis()->SetLabelOffset(999);
  h1->GetXaxis()->SetLabelSize(0);

  // Set bottom histogram range
  double maxy = h1->GetMaximum();
  double miny = h1->GetMinimum();
  double delta = TMath::Max(maxy - miny,1.0);

  h1->SetAxisRange(miny - 0.1*delta,maxy + 0.1*delta ,"Y");
  h1->DrawCopy("");

  // Set PDF file name
  string fName;
  ostringstream tempfName;
  tempfName << fileName << ".pdf";
 
  if(opt == 0)
    tempfName << "(";
  else if(opt == 2)
    tempfName << ")";

 fName = tempfName.str();

 // Save histogram as PDF
 C->Print(fName.c_str(), title.c_str());

 // Delete instances of new
 delete C;
 delete leg;
}
