/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "histio.C"
#include "TCanvas.h"
#include "TChain.h"
#include "TH1.h"
#include "TCut.h"
#include "TStyle.h"

   TCanvas* c2 ;
   TChain *belle_root, *belle2_root ;

   void bookset(const char* hbasename, const char* basftype, const char* htitle, Int_t bins, Float_t xl, Float_t xh);

   void fillset(const char* hbasename, const char* basftype, const char* treevar, const char* cuts);

//==============================================================================

   void fill01( Int_t setIndex=-1 ) {

      gStyle->SetOptStat(0) ;

      gDirectory->Delete("h*") ;
      c2 = (TCanvas*) gDirectory->FindObject("c2") ;
      if (c2==0) {
         cout << "Creating canvas." << endl ;
         c2 = new TCanvas("c2","c2",700,500) ;
      }

      belle_root = (TChain*) gDirectory->FindObject("belle_root") ;
      if (belle_root==0) {
         cout << "Setting up chains." << endl ;
         belle_root = new TChain("tree") ;
         belle2_root = new TChain("tree") ;
//         belle_root->Add("/Users/hulya/research/kek/belle2/b2bii/root-files/cc/cc_exp55_run795.root") ;
//         belle2_root->Add("/Users/hulya/research/kek/belle2/b2bii/root-files/cc/cc-basf2.root") ;
      belle_root->Add("/gpfs/home/belle/hulya/public/b2bii-validation/input-root-files/cc_exp55_run795.root") ;
      belle2_root->Add("/gpfs/home/belle/hulya/public/b2bii-validation/input-root-files/cc-basf2.root") ;
      } else {
         cout << "Chains already set up." << endl ;
      }

      Int_t index(0) ;

      //BASF

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg01", "basf", "idhep_mpdg", 100, -22500, 22500) ;
         fillset( "idhep_mpdg01", "basf", "hepevtIdhep", "hepevtIdhep<1000000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg02", "basf", "idhep_mpdg", 102, 8998675, 9031825) ;
         fillset( "idhep_mpdg02", "basf", "hepevtIdhep", "hepevtIdhep>8998675&&hepevtIdhep<9031825") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg03", "basf", "idhep_mpdg", 102, -70, 950) ;
         fillset( "idhep_mpdg03", "basf", "hepevtIdhep", "hepevtIdhep>-70&&hepevtIdhep<950") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg04", "basf", "idhep_mpdg", 4, 909, 913) ;
         fillset( "idhep_mpdg04", "basf", "hepevtIdhep", "hepevtIdhep>900&&hepevtIdhep<1000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg05", "basf", "idhep_mpdg", 10, -5, 5) ;
         fillset( "idhep_mpdg05", "basf", "hepevtIdhep", "hepevtIdhep>-5&&hepevtIdhep<5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg06", "basf", "idhep_mpdg", 50, 20, 24) ;
         fillset( "idhep_mpdg06", "basf", "hepevtIdhep", "hepevtIdhep>20&&hepevtIdhep<24") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Px", "basf", "Px", 100, -5.6, 6) ;
         fillset( "Px", "basf", "hepevtPx", "hepevtPx>-5.6&&hepevtPx<6") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Py", "basf", "Py", 100, -6, 5.8) ;
         fillset( "Py", "basf", "hepevtPy", "hepevtPy>-6&&hepevtPy<5.8") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Pz", "basf", "Pz", 100, -5, 16.5) ;
         fillset( "Pz", "basf", "hepevtPz", "hepevtPz>-5&&hepevtPz<16.5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Energy", "basf", "Energy", 100, 0, 35) ;
         fillset( "Energy", "basf", "hepevtEnergy", "hepevtEnergy>0&&hepevtEnergy<35") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Mass", "basf", "Mass", 100, 0, 31.5) ;
         fillset( "Mass", "basf", "hepevtMass", "hepevtMass>0&&hepevtMass<31.5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "ProdVertexX", "basf", "Production Vertex (X)", 100, -5000, 5000) ;
         fillset( "ProdVertexX", "basf", "hepevtVx", "hepevtVx>-5000&&hepevtVx<5000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "ProdVertexY", "basf", "Production Vertex (Y)", 100, -4800, 5000) ;
         fillset( "ProdVertexY", "basf", "hepevtVy", "hepevtVy>-4800&&hepevtVy<5000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "ProdVertexZ", "basf", "Production Vertex (Z)", 100, -10000, 10000) ;
         fillset( "ProdVertexZ", "basf", "hepevtVz", "hepevtVz>-10000&&hepevtVz<10000") ;
      }


      //BASF2

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg01", "basf2", "idhep_mpdg", 100, -22500, 22500) ;
         fillset( "idhep_mpdg01", "basf2",  "m_pdg", "m_pdg<1000000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg02", "basf2", "idhep_mpdg", 102, 8998675, 9031825) ;
         fillset( "idhep_mpdg02", "basf2",  "m_pdg", "m_pdg>8998675&&m_pdg<9031825") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg03", "basf2", "idhep_mpdg", 102, -70, 950) ;
         fillset( "idhep_mpdg03", "basf2",  "m_pdg", "m_pdg>-70&&m_pdg<950") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg04", "basf2", "idhep_mpdg", 102, 774, 3426) ;
         fillset( "idhep_mpdg04", "basf2",  "m_pdg", "m_pdg>774&&m_pdg<3426") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg05", "basf2", "idhep_mpdg", 10, -5, 5) ;
         fillset( "idhep_mpdg05", "basf2",  "m_pdg", "m_pdg>-5&&m_pdg<5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "idhep_mpdg06", "basf2", "idhep_mpdg", 50, 20, 24) ;
         fillset( "idhep_mpdg06", "basf2",  "m_pdg", "m_pdg>20&&m_pdg<24") ;
      }


      if (setIndex<0||setIndex==++index) {
         bookset( "Px", "basf2", "Px", 100, -5.6, 6) ;
         fillset( "Px", "basf2", "MCParticles.m_momentum_x", "MCParticles.m_momentum_x>-5.6&&MCParticles.m_momentum_x<6") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Py", "basf2", "Py", 100, -6, 5.8) ;
         fillset( "Py", "basf2", "MCParticles.m_momentum_y", "MCParticles.m_momentum_y>-6&&MCParticles.m_momentum_y<5.8") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Pz", "basf2", "Pz", 100, -5, 16.5) ;
         fillset( "Pz", "basf2", "MCParticles.m_momentum_z", "MCParticles.m_momentum_z>-5&&MCParticles.m_momentum_z<16.5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Energy", "basf2", "Energy", 100, 0, 35) ;
         fillset( "Energy", "basf2", "MCParticles.m_energy", "MCParticles.m_energy>0&&MCParticles.m_energy<35") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "Mass", "basf2", "Mass", 100, 0, 31.5) ;
         fillset( "Mass", "basf2", "MCParticles.m_mass", "MCParticles.m_mass>0&&MCParticles.m_mass<31.5") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "ProdVertexX", "basf2", "Production Vertex (X)", 100, -5000, 5000) ;
         fillset( "ProdVertexX", "basf2", "MCParticles.m_productionVertex_x*10", "MCParticles.m_productionVertex_x*10>-5000&&MCParticles.m_productionVertex_x*10<5000") ;
      }

      if (setIndex<0||setIndex==++index) {
         bookset( "ProdVertexY", "basf2", "Production Vertex (Y)", 100, -4800, 5000) ;
         fillset( "ProdVertexY", "basf2", "MCParticles.m_productionVertex_y*10", "MCParticles.m_productionVertex_y*10>-4800&&MCParticles.m_productionVertex_y*10<5000") ;
      }

      if (setIndex<0||setIndex==index) {
         bookset( "ProdVertexZ", "basf2", "Production Vertex (Z)", 100, -10000, 10000) ;
         fillset( "ProdVertexZ", "basf2", "MCParticles.m_productionVertex_z*10", "MCParticles.m_productionVertex_z*10>-10000&&MCParticles.m_productionVertex_z*10<10000") ;
      }

      cout << "Saving plots." << endl ;
      saveHist("scratch/cc-test.root","h*") ;
    }

//==============================================================================
   void bookset(const char* hbasename, const char* basftype, const char* htitle, Int_t bins, Float_t xl, Float_t xh) {

      cout << "\n  Booking plots with title: "
           << htitle << " for " << basftype << endl ;
      cout << "  Histogram parameters: " << hbasename << "  " << bins << " [" << xl
                                         << "," << xh << "]" << endl ;

      char histname[500] ;
      char histtitle[500] ;

      sprintf(histname,"h%s_%s",hbasename,basftype) ;
      sprintf(histtitle, "%s_%s",htitle,basftype) ;
      new TH1F(histname, histtitle, bins, xl, xh ) ;

   } // bookset

//------------------------------------------------------------------------------

   void fillset(const char* hbasename, const char* basftype, const char* treevar, const char* cuts) {

      cout << " Filling " << hbasename << " plots of " << treevar << endl ;
      cout << " Cuts: " << cuts << endl ;

      char histname[500] ;
      char arg1[500] ;

      char modecuts[500] ;

      sprintf(modecuts,"%s",cuts) ;

      if (strcmp(basftype, "basf") == 0) {
         TChain *basf_root = belle_root ;

         sprintf(histname, "h%s_%s", hbasename,basftype) ;
         sprintf( arg1, "%s>>%s", treevar, histname ) ;
         cout << "arg1: " << arg1 << "  modecuts: " << modecuts << endl ;
         basf_root->Draw( arg1, modecuts ) ;
         c2->Update() ;
      }

      if (strcmp(basftype, "basf2") == 0) {
         TChain *basf2_root = belle2_root ;

         sprintf(histname, "h%s_%s", hbasename,basftype) ;
         sprintf( arg1, "%s>>%s", treevar, histname ) ;
         cout << "arg1: " << arg1 << "  modecuts: " << modecuts << endl ;
         basf2_root->Draw( arg1, modecuts ) ;
         c2->Update() ;
      }

   } //fillset
