//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Nov 30 21:28:38 2017 by ROOT version 5.34/10
// from TTree m_tree/ECL Charged PID tree
// found on file: pdg11_range3000to5500MeV.root
//////////////////////////////////////////////////////////

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

class eclChargedPidSelector : public TSelector {
public :
  TTree*          fChain;   //!pointer to the analyzed TTree or TChain

  // Declaration of leaf types
  Int_t           expNo;
  Int_t           runNo;
  Int_t           evtNo;
  Int_t           eclShowerMultip;
  vector<double>*  eclShowerEnergy;
  vector<double>*  eclShowerTheta;
  vector<double>*  eclShowerPhi;
  vector<double>*  eclShowerR;
  vector<int>*     eclShowerHypothesisId;
//   vector<double>  *eclShowerAbsZernike11;
//   vector<double>  *eclShowerAbsZernike20;
//   vector<double>  *eclShowerAbsZernike22;
//   vector<double>  *eclShowerAbsZernike31;
//   vector<double>  *eclShowerAbsZernike33;
  vector<double>*  eclShowerAbsZernike40;
//   vector<double>  *eclShowerAbsZernike42;
//   vector<double>  *eclShowerAbsZernike44;
  vector<double>*  eclShowerAbsZernike51;
//   vector<double>  *eclShowerAbsZernike53;
//   vector<double>  *eclShowerAbsZernike55;
  Int_t           mcMultip;
  vector<int>*     mcPdg;
  vector<int>*     mcMothPdg;
  vector<double>*  mcEnergy;
  vector<double>*  mcP;
  vector<double>*  mcTheta;
  vector<double>*  mcPhi;
  Int_t           trkMultip;
  vector<int>*     trkPdg;
  vector<int>*     trkCharge;
  vector<double>*  trkP;
  vector<double>*  trkTheta;
  vector<double>*  trkPhi;
  vector<double>*  eclEoP;

  // List of branches
  TBranch*        b_expNo;   //!
  TBranch*        b_runNo;   //!
  TBranch*        b_evtNo;   //!
  TBranch*        b_eclShowerMultip;   //!
  TBranch*        b_eclShowerEnergy;   //!
  TBranch*        b_eclShowerTheta;   //!
  TBranch*        b_eclShowerPhi;   //!
  TBranch*        b_eclShowerR;   //!
  TBranch*        b_eclShowerHypothesisId;   //!
//   TBranch        *b_eclShowerAbsZernike11;   //!
//   TBranch        *b_eclShowerAbsZernike20;   //!
//   TBranch        *b_eclShowerAbsZernike22;   //!
//   TBranch        *b_eclShowerAbsZernike31;   //!
//   TBranch        *b_eclShowerAbsZernike33;   //!
  TBranch*        b_eclShowerAbsZernike40;   //!
//   TBranch        *b_eclShowerAbsZernike42;   //!
//   TBranch        *b_eclShowerAbsZernike44;   //!
  TBranch*        b_eclShowerAbsZernike51;   //!
//   TBranch        *b_eclShowerAbsZernike53;   //!
//   TBranch        *b_eclShowerAbsZernike55;   //!
  TBranch*        b_mcMultip;   //!
  TBranch*        b_mcPdg;   //!
  TBranch*        b_mcMothPdg;   //!
  TBranch*        b_mcEnergy;   //!
  TBranch*        b_mcP;   //!
  TBranch*        b_mcTheta;   //!
  TBranch*        b_mcPhi;   //!
  TBranch*        b_trkMulti;   //!
  TBranch*        b_trkPdg;   //!
  TBranch*        b_trkCharge;   //!
  TBranch*        b_trkP;   //!
  TBranch*        b_trkTheta;   //!
  TBranch*        b_trkPhi;   //!
  TBranch*        b_eclEoP;   //!

  eclChargedPidSelector(TTree* /*tree*/ = 0) : fChain(0) { }
  virtual ~eclChargedPidSelector() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree* tree);
  virtual void    SlaveBegin(TTree* tree);
  virtual void    Init(TTree* tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char* option) { fOption = option; }
  virtual void    SetObject(TObject* obj) { fObject = obj; }
  virtual void    SetInputList(TList* input) { fInput = input; }
  virtual TList*  GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  ClassDef(eclChargedPidSelector, 0);
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
//   eclShowerAbsZernike11 = 0;
//   eclShowerAbsZernike20 = 0;
//   eclShowerAbsZernike22 = 0;
//   eclShowerAbsZernike31 = 0;
//   eclShowerAbsZernike33 = 0;
  eclShowerAbsZernike40 = 0;
//   eclShowerAbsZernike42 = 0;
//   eclShowerAbsZernike44 = 0;
  eclShowerAbsZernike51 = 0;
//   eclShowerAbsZernike53 = 0;
//   eclShowerAbsZernike55 = 0;
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
//   fChain->SetBranchAddress("eclShowerAbsZernike11", &eclShowerAbsZernike11, &b_eclShowerAbsZernike11);
//   fChain->SetBranchAddress("eclShowerAbsZernike20", &eclShowerAbsZernike20, &b_eclShowerAbsZernike20);
//   fChain->SetBranchAddress("eclShowerAbsZernike22", &eclShowerAbsZernike22, &b_eclShowerAbsZernike22);
//   fChain->SetBranchAddress("eclShowerAbsZernike31", &eclShowerAbsZernike31, &b_eclShowerAbsZernike31);
//   fChain->SetBranchAddress("eclShowerAbsZernike33", &eclShowerAbsZernike33, &b_eclShowerAbsZernike33);
  fChain->SetBranchAddress("eclShowerAbsZernike40", &eclShowerAbsZernike40, &b_eclShowerAbsZernike40);
//   fChain->SetBranchAddress("eclShowerAbsZernike42", &eclShowerAbsZernike42, &b_eclShowerAbsZernike42);
//   fChain->SetBranchAddress("eclShowerAbsZernike44", &eclShowerAbsZernike44, &b_eclShowerAbsZernike44);
  fChain->SetBranchAddress("eclShowerAbsZernike51", &eclShowerAbsZernike51, &b_eclShowerAbsZernike51);
//   fChain->SetBranchAddress("eclShowerAbsZernike53", &eclShowerAbsZernike53, &b_eclShowerAbsZernike53);
//   fChain->SetBranchAddress("eclShowerAbsZernike55", &eclShowerAbsZernike55, &b_eclShowerAbsZernike55);
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
