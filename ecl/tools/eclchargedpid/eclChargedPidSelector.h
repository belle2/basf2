//
// Contributors: Caitlin MacQueen, Marco Milesi
// Contact: cmq.centaurus@gmail.com
// Last Modified: June 2018
//

#ifndef eclChargedPidSelector_h
#define eclChargedPidSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TLorentzVector.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <TCut.h>

// Header file for the classes stored in the TTree if any.
#include <vector>

// Fixed size dimensions of array or collections stored in the TTree if any.

/** eclChargedPidSelector
 */
class eclChargedPidSelector : public TSelector {
public :
  /** pointer to the analyzed TTree or TChain */
  TTree*          fChain;   //!

  // Declaration of leaf types

  /** expNo */
  Int_t           expNo;
  /** runNo */
  Int_t           runNo;
  /** evtNo */
  Int_t           evtNo;
  /** eclShowerMultip */
  Int_t           eclShowerMultip;
  /** eclShowerEnergy */
  vector<double>*  eclShowerEnergy;
  /** eclShowerTheta */
  vector<double>*  eclShowerTheta;
  /** eclShowerPhi */
  vector<double>*  eclShowerPhi;
  /** eclShowerR */
  vector<double>*  eclShowerR;
  /** eclShowerHypothesisId */
  vector<int>*     eclShowerHypothesisId;
  /** eclShowerAbsZernike40 */
  vector<double>*  eclShowerAbsZernike40;
  /** eclShowerAbsZernike51 */
  vector<double>*  eclShowerAbsZernike51;
  /** mcMultip */
  Int_t           mcMultip;
  /** mcPdg */
  vector<int>*     mcPdg;
  /** mcMothPdg */
  vector<int>*     mcMothPdg;
  /** mcEnergy */
  vector<double>*  mcEnergy;
  /** mcP */
  vector<double>*  mcP;
  /** mcTheta */
  vector<double>*  mcTheta;
  /** mcPhi */
  vector<double>*  mcPhi;
  /** trkMultip */
  Int_t           trkMultip;
  /** trkPdg */
  vector<int>*     trkPdg;
  /** trkCharge */
  vector<int>*     trkCharge;
  /** trkP */
  vector<double>*  trkP;
  /** trkTheta */
  vector<double>*  trkTheta;
  /** trkPhi */
  vector<double>*  trkPhi;
  /** eclEoP */
  vector<double>*  eclEoP;

  // List of branches
  /** b_expNo */
  TBranch*        b_expNo;   //!
  /** b_runNo */
  TBranch*        b_runNo;   //!
  /** b_evtNo */
  TBranch*        b_evtNo;   //!
  /** b_eclShowerMultip */
  TBranch*        b_eclShowerMultip;   //!
  /** b_eclShowerEnergy */
  TBranch*        b_eclShowerEnergy;   //!
  /** b_eclShowerTheta */
  TBranch*        b_eclShowerTheta;   //!
  /** b_eclShowerPhi */
  TBranch*        b_eclShowerPhi;   //!
  /** b_eclShowerR */
  TBranch*        b_eclShowerR;   //!
  /** b_eclShowerHypothesisId */
  TBranch*        b_eclShowerHypothesisId;   //!
  /** b_eclShowerAbsZernike40 */
  TBranch*        b_eclShowerAbsZernike40;   //!
  /** b_eclShowerAbsZernike51 */
  TBranch*        b_eclShowerAbsZernike51;   //!
  /** b_mcMultip */
  TBranch*        b_mcMultip;   //!
  /** b_mcPdg */
  TBranch*        b_mcPdg;   //!
  /** b_mcMothPdg */
  TBranch*        b_mcMothPdg;   //!
  /** b_mcEnergy */
  TBranch*        b_mcEnergy;   //!
  /** b_mcP */
  TBranch*        b_mcP;   //!
  /** b_mcTheta */
  TBranch*        b_mcTheta;   //!
  /** b_mcPhi */
  TBranch*        b_mcPhi;   //!
  /** b_trkMulti */
  TBranch*        b_trkMulti;   //!
  /** b_trkPdg */
  TBranch*        b_trkPdg;   //!
  /** b_trkCharge */
  TBranch*        b_trkCharge;   //!
  /** b_trkP */
  TBranch*        b_trkP;   //!
  /** b_trkTheta */
  TBranch*        b_trkTheta;   //!
  /** b_trkPhi */
  TBranch*        b_trkPhi;   //!
  /** b_eclEoP */
  TBranch*        b_eclEoP;   //!

  /** fOutfile */
  string fOutfile;

  void SetOutputDir(const char* outpath); /**< Set output directory */

  /** Constructor */
  eclChargedPidSelector(TTree* = 0) :
    fChain(0),
    eclShowerEnergy(0),
    eclShowerTheta(0),
    eclShowerPhi(0),
    eclShowerR(0),
    eclShowerHypothesisId(0),
    eclShowerAbsZernike40(0),
    eclShowerAbsZernike51(0),
    mcPdg(0),
    mcMothPdg(0),
    mcEnergy(0),
    mcP(0),
    mcTheta(0),
    mcPhi(0),
    trkPdg(0),
    trkCharge(0),
    trkP(0),
    trkTheta(0),
    trkPhi(0),
    eclEoP(0),
    b_expNo(0),
    b_runNo(0),
    b_evtNo(0),
    b_eclShowerMultip(0),
    b_eclShowerEnergy(0),
    b_eclShowerTheta(0),
    b_eclShowerPhi(0),
    b_eclShowerR(0),
    b_eclShowerHypothesisId(0),
    b_eclShowerAbsZernike40(0),
    b_eclShowerAbsZernike51(0),
    b_mcMultip(0),
    b_mcPdg(0),
    b_mcMothPdg(0),
    b_mcEnergy(0),
    b_mcP(0),
    b_mcTheta(0),
    b_mcPhi(0),
    b_trkMulti(0),
    b_trkPdg(0),
    b_trkCharge(0),
    b_trkP(0),
    b_trkTheta(0),
    b_trkPhi(0),
    b_eclEoP(0),
    fOutfile(string())
  { };

  virtual ~eclChargedPidSelector() { }; /**< Destructor */

  virtual Int_t   Version() const { return 2; }; /**< Version */
  virtual void    Begin(TTree* tree); /**< Begin */
  virtual void    SlaveBegin(TTree* tree); /**< SlaveBegin */
  virtual void    Init(TTree* tree); /**< Init */
  virtual Bool_t  Notify(); /**< Notify */
  virtual Bool_t  Process(Long64_t entry); /**< Process */
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }; /**< GetEntry */
  virtual void    SetOption(const char* option) { fOption = option; }; /**< SetOption */
  virtual void    SetObject(TObject* obj) { fObject = obj; }; /**< SetObject */
  virtual void    SetInputList(TList* input) { fInput = input; }; /**< SetInputList */
  virtual TList*  GetOutputList() const { return fOutput; }; /**< GetOutputList */
  virtual void    SlaveTerminate(); /**< SlaveTerminate */
  virtual void    Terminate(); /**< Terminate */

  ClassDef(eclChargedPidSelector, 0); /**< ClassDef */
};

#endif

#ifdef eclChargedPidSelector_cxx
void eclChargedPidSelector::Init(TTree* tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set object pointer
  eclShowerEnergy = 0;
  eclShowerTheta = 0;
  eclShowerPhi = 0;
  eclShowerR = 0;
  eclShowerHypothesisId = 0;
  eclShowerAbsZernike40 = 0;
  eclShowerAbsZernike51 = 0;
  mcPdg = 0;
  mcMothPdg = 0;
  mcEnergy = 0;
  mcP = 0;
  mcTheta = 0;
  mcPhi = 0;
  trkPdg = 0;
  trkCharge = 0;
  trkP = 0;
  trkTheta = 0;
  trkPhi = 0;
  eclEoP = 0;
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("expNo", &expNo, &b_expNo);
  fChain->SetBranchAddress("runNo", &runNo, &b_runNo);
  fChain->SetBranchAddress("evtNo", &evtNo, &b_evtNo);
  fChain->SetBranchAddress("eclShowerMultip", &eclShowerMultip, &b_eclShowerMultip);
  fChain->SetBranchAddress("eclShowerEnergy", &eclShowerEnergy, &b_eclShowerEnergy);
  fChain->SetBranchAddress("eclShowerTheta", &eclShowerTheta, &b_eclShowerTheta);
  fChain->SetBranchAddress("eclShowerPhi", &eclShowerPhi, &b_eclShowerPhi);
  fChain->SetBranchAddress("eclShowerR", &eclShowerR, &b_eclShowerR);
  fChain->SetBranchAddress("eclShowerHypothesisId", &eclShowerHypothesisId, &b_eclShowerHypothesisId);
  fChain->SetBranchAddress("eclShowerAbsZernike40", &eclShowerAbsZernike40, &b_eclShowerAbsZernike40);
  fChain->SetBranchAddress("eclShowerAbsZernike51", &eclShowerAbsZernike51, &b_eclShowerAbsZernike51);
  fChain->SetBranchAddress("mcMultip", &mcMultip, &b_mcMultip);
  fChain->SetBranchAddress("mcPdg", &mcPdg, &b_mcPdg);
  fChain->SetBranchAddress("mcMothPdg", &mcMothPdg, &b_mcMothPdg);
  fChain->SetBranchAddress("mcEnergy", &mcEnergy, &b_mcEnergy);
  fChain->SetBranchAddress("mcP", &mcP, &b_mcP);
  fChain->SetBranchAddress("mcTheta", &mcTheta, &b_mcTheta);
  fChain->SetBranchAddress("mcPhi", &mcPhi, &b_mcPhi);
  fChain->SetBranchAddress("trkMultip", &trkMultip, &b_trkMulti);
  fChain->SetBranchAddress("trkPdg", &trkPdg, &b_trkPdg);
  fChain->SetBranchAddress("trkCharge", &trkCharge, &b_trkCharge);
  fChain->SetBranchAddress("trkP", &trkP, &b_trkP);
  fChain->SetBranchAddress("trkTheta", &trkTheta, &b_trkTheta);
  fChain->SetBranchAddress("trkPhi", &trkPhi, &b_trkPhi);
  fChain->SetBranchAddress("eclEoP", &eclEoP, &b_eclEoP);
}

Bool_t eclChargedPidSelector::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

#endif // #ifdef eclChargedPidSelector_cxx
