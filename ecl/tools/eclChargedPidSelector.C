//
// Contributor: Caitlin MacQueen
// Contact: cmq.centaurus@gmail.com
// Last Modified: June 2018
//

#define eclChargedPidSelector_cxx
// The class definition in eclChargedPidSelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("eclChargedPidSelector.C")
// Root > T->Process("eclChargedPidSelector.C","some options")
// Root > T->Process("eclChargedPidSelector.C+")
//

#include "eclChargedPidSelector.h"
#include <TH2.h>
#include <TStyle.h>


void eclChargedPidSelector::Begin(TTree * /*tree*/)
{
    // The Begin() function is called at the start of the query.
    // When running with PROOF Begin() is only called on the client.
    // The tree argument is deprecated (on PROOF 0 is passed).

    TString option = GetOption();

    TH1F* hflow = new TH1F( "hflow", " Flow ", 6, 0., 6. );  fOutput->Add( hflow );

    TH1F* h_Eop_0_0 = new TH1F( "h_Eop_0_0", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_0 );
    TH1F* h_Eop_0_1 = new TH1F( "h_Eop_0_1", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_1 );
    TH1F* h_Eop_0_2 = new TH1F( "h_Eop_0_2", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_2 );
    TH1F* h_Eop_0_3 = new TH1F( "h_Eop_0_3", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_3 );
    TH1F* h_Eop_0_4 = new TH1F( "h_Eop_0_4", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_4 );
    TH1F* h_Eop_0_5 = new TH1F( "h_Eop_0_5", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_5 );
    TH1F* h_Eop_0_6 = new TH1F( "h_Eop_0_6", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_6 );
    TH1F* h_Eop_0_7 = new TH1F( "h_Eop_0_7", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_7 );
    TH1F* h_Eop_0_8 = new TH1F( "h_Eop_0_8", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_0_8 );
    TH1F* h_Eop_1_0 = new TH1F( "h_Eop_1_0", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_0 );
    TH1F* h_Eop_1_1 = new TH1F( "h_Eop_1_1", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_1 );
    TH1F* h_Eop_1_2 = new TH1F( "h_Eop_1_2", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_2 );
    TH1F* h_Eop_1_3 = new TH1F( "h_Eop_1_3", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_3 );
    TH1F* h_Eop_1_4 = new TH1F( "h_Eop_1_4", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_4 );
    TH1F* h_Eop_1_5 = new TH1F( "h_Eop_1_5", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_5 );
    TH1F* h_Eop_1_6 = new TH1F( "h_Eop_1_6", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_6 );
    TH1F* h_Eop_1_7 = new TH1F( "h_Eop_1_7", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_7 );
    TH1F* h_Eop_1_8 = new TH1F( "h_Eop_1_8", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_1_8 );
    TH1F* h_Eop_2_0 = new TH1F( "h_Eop_2_0", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_0 );
    TH1F* h_Eop_2_1 = new TH1F( "h_Eop_2_1", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_1 );
    TH1F* h_Eop_2_2 = new TH1F( "h_Eop_2_2", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_2 );
    TH1F* h_Eop_2_3 = new TH1F( "h_Eop_2_3", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_3 );
    TH1F* h_Eop_2_4 = new TH1F( "h_Eop_2_4", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_4 );
    TH1F* h_Eop_2_5 = new TH1F( "h_Eop_2_5", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_5 );
    TH1F* h_Eop_2_6 = new TH1F( "h_Eop_2_6", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_6 );
    TH1F* h_Eop_2_7 = new TH1F( "h_Eop_2_7", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_7 );
    TH1F* h_Eop_2_8 = new TH1F( "h_Eop_2_8", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_2_8 );
    TH1F* h_Eop_3_0 = new TH1F( "h_Eop_3_0", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_0 );
    TH1F* h_Eop_3_1 = new TH1F( "h_Eop_3_1", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_1 );
    TH1F* h_Eop_3_2 = new TH1F( "h_Eop_3_2", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_2 );
    TH1F* h_Eop_3_3 = new TH1F( "h_Eop_3_3", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_3 );
    TH1F* h_Eop_3_4 = new TH1F( "h_Eop_3_4", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_4 );
    TH1F* h_Eop_3_5 = new TH1F( "h_Eop_3_5", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_5 );
    TH1F* h_Eop_3_6 = new TH1F( "h_Eop_3_6", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_6 );
    TH1F* h_Eop_3_7 = new TH1F( "h_Eop_3_7", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_7 );
    TH1F* h_Eop_3_8 = new TH1F( "h_Eop_3_8", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_3_8 );
    TH1F* h_Eop_4_0 = new TH1F( "h_Eop_4_0", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_0 );
    TH1F* h_Eop_4_1 = new TH1F( "h_Eop_4_1", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_1 );
    TH1F* h_Eop_4_2 = new TH1F( "h_Eop_4_2", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_2 );
    TH1F* h_Eop_4_3 = new TH1F( "h_Eop_4_3", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_3 );
    TH1F* h_Eop_4_4 = new TH1F( "h_Eop_4_4", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_4 );
    TH1F* h_Eop_4_5 = new TH1F( "h_Eop_4_5", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_5 );
    TH1F* h_Eop_4_6 = new TH1F( "h_Eop_4_6", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_6 );
    TH1F* h_Eop_4_7 = new TH1F( "h_Eop_4_7", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_7 );
    TH1F* h_Eop_4_8 = new TH1F( "h_Eop_4_8", "ECL E/p", 130, 0., 2.6 );    fOutput->Add( h_Eop_4_8 );
    TH1F* h_Eop_5_0 = new TH1F( "h_Eop_5_0", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_0 );
    TH1F* h_Eop_5_1 = new TH1F( "h_Eop_5_1", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_1 );
    TH1F* h_Eop_5_2 = new TH1F( "h_Eop_5_2", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_2 );
    TH1F* h_Eop_5_3 = new TH1F( "h_Eop_5_3", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_3 );
    TH1F* h_Eop_5_4 = new TH1F( "h_Eop_5_4", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_4 );
    TH1F* h_Eop_5_5 = new TH1F( "h_Eop_5_5", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_5 );
    TH1F* h_Eop_5_6 = new TH1F( "h_Eop_5_6", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_6 );
    TH1F* h_Eop_5_7 = new TH1F( "h_Eop_5_7", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_7 );
    TH1F* h_Eop_5_8 = new TH1F( "h_Eop_5_8", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_5_8 );
    TH1F* h_Eop_6_0 = new TH1F( "h_Eop_6_0", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_0 );
    TH1F* h_Eop_6_1 = new TH1F( "h_Eop_6_1", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_1 );
    TH1F* h_Eop_6_2 = new TH1F( "h_Eop_6_2", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_2 );
    TH1F* h_Eop_6_3 = new TH1F( "h_Eop_6_3", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_3 );
    TH1F* h_Eop_6_4 = new TH1F( "h_Eop_6_4", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_4 );
    TH1F* h_Eop_6_5 = new TH1F( "h_Eop_6_5", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_5 );
    TH1F* h_Eop_6_6 = new TH1F( "h_Eop_6_6", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_6 );
    TH1F* h_Eop_6_7 = new TH1F( "h_Eop_6_7", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_7 );
    TH1F* h_Eop_6_8 = new TH1F( "h_Eop_6_8", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_6_8 );
    TH1F* h_Eop_7_0 = new TH1F( "h_Eop_7_0", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_0 );
    TH1F* h_Eop_7_1 = new TH1F( "h_Eop_7_1", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_1 );
    TH1F* h_Eop_7_2 = new TH1F( "h_Eop_7_2", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_2 );
    TH1F* h_Eop_7_3 = new TH1F( "h_Eop_7_3", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_3 );
    TH1F* h_Eop_7_4 = new TH1F( "h_Eop_7_4", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_4 );
    TH1F* h_Eop_7_5 = new TH1F( "h_Eop_7_5", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_5 );
    TH1F* h_Eop_7_6 = new TH1F( "h_Eop_7_6", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_6 );
    TH1F* h_Eop_7_7 = new TH1F( "h_Eop_7_7", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_7 );
    TH1F* h_Eop_7_8 = new TH1F( "h_Eop_7_8", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_7_8 );
    TH1F* h_Eop_8_0 = new TH1F( "h_Eop_8_0", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_0 );
    TH1F* h_Eop_8_1 = new TH1F( "h_Eop_8_1", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_1 );
    TH1F* h_Eop_8_2 = new TH1F( "h_Eop_8_2", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_2 );
    TH1F* h_Eop_8_3 = new TH1F( "h_Eop_8_3", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_3 );
    TH1F* h_Eop_8_4 = new TH1F( "h_Eop_8_4", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_4 );
    TH1F* h_Eop_8_5 = new TH1F( "h_Eop_8_5", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_5 );
    TH1F* h_Eop_8_6 = new TH1F( "h_Eop_8_6", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_6 );
    TH1F* h_Eop_8_7 = new TH1F( "h_Eop_8_7", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_7 );
    TH1F* h_Eop_8_8 = new TH1F( "h_Eop_8_8", "ECL E/p", 260, 0., 2.6 );    fOutput->Add( h_Eop_8_8 );
    TH1F* h_Eop_9_0 = new TH1F( "h_Eop_9_0", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_0 );
    TH1F* h_Eop_9_1 = new TH1F( "h_Eop_9_1", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_1 );
    TH1F* h_Eop_9_2 = new TH1F( "h_Eop_9_2", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_2 );
    TH1F* h_Eop_9_3 = new TH1F( "h_Eop_9_3", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_3 );
    TH1F* h_Eop_9_4 = new TH1F( "h_Eop_9_4", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_4 );
    TH1F* h_Eop_9_5 = new TH1F( "h_Eop_9_5", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_5 );
    TH1F* h_Eop_9_6 = new TH1F( "h_Eop_9_6", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_6 );
    TH1F* h_Eop_9_7 = new TH1F( "h_Eop_9_7", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_7 );
    TH1F* h_Eop_9_8 = new TH1F( "h_Eop_9_8", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_9_8 );
    TH1F* h_Eop_10_0 = new TH1F( "h_Eop_10_0", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_0 );
    TH1F* h_Eop_10_1 = new TH1F( "h_Eop_10_1", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_1 );
    TH1F* h_Eop_10_2 = new TH1F( "h_Eop_10_2", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_2 );
    TH1F* h_Eop_10_3 = new TH1F( "h_Eop_10_3", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_3 );
    TH1F* h_Eop_10_4 = new TH1F( "h_Eop_10_4", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_4 );
    TH1F* h_Eop_10_5 = new TH1F( "h_Eop_10_5", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_5 );
    TH1F* h_Eop_10_6 = new TH1F( "h_Eop_10_6", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_6 );
    TH1F* h_Eop_10_7 = new TH1F( "h_Eop_10_7", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_7 );
    TH1F* h_Eop_10_8 = new TH1F( "h_Eop_10_8", "ECL E/p", 390, 0., 2.6 );    fOutput->Add( h_Eop_10_8 );

    TH1F* h_ShowerE_0_0 = new TH1F( "h_ShowerE_0_0", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_0 );
    TH1F* h_ShowerE_0_1 = new TH1F( "h_ShowerE_0_1", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_1 );
    TH1F* h_ShowerE_0_2 = new TH1F( "h_ShowerE_0_2", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_2 );
    TH1F* h_ShowerE_0_3 = new TH1F( "h_ShowerE_0_3", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_3 );
    TH1F* h_ShowerE_0_4 = new TH1F( "h_ShowerE_0_4", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_4 );
    TH1F* h_ShowerE_0_5 = new TH1F( "h_ShowerE_0_5", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_5 );
    TH1F* h_ShowerE_0_6 = new TH1F( "h_ShowerE_0_6", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_6 );
    TH1F* h_ShowerE_0_7 = new TH1F( "h_ShowerE_0_7", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_7 );
    TH1F* h_ShowerE_0_8 = new TH1F( "h_ShowerE_0_8", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_0_8 );
    TH1F* h_ShowerE_1_0 = new TH1F( "h_ShowerE_1_0", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_0 );
    TH1F* h_ShowerE_1_1 = new TH1F( "h_ShowerE_1_1", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_1 );
    TH1F* h_ShowerE_1_2 = new TH1F( "h_ShowerE_1_2", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_2 );
    TH1F* h_ShowerE_1_3 = new TH1F( "h_ShowerE_1_3", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_3 );
    TH1F* h_ShowerE_1_4 = new TH1F( "h_ShowerE_1_4", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_4 );
    TH1F* h_ShowerE_1_5 = new TH1F( "h_ShowerE_1_5", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_5 );
    TH1F* h_ShowerE_1_6 = new TH1F( "h_ShowerE_1_6", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_6 );
    TH1F* h_ShowerE_1_7 = new TH1F( "h_ShowerE_1_7", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_7 );
    TH1F* h_ShowerE_1_8 = new TH1F( "h_ShowerE_1_8", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_1_8 );
    TH1F* h_ShowerE_2_0 = new TH1F( "h_ShowerE_2_0", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_0 );
    TH1F* h_ShowerE_2_1 = new TH1F( "h_ShowerE_2_1", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_1 );
    TH1F* h_ShowerE_2_2 = new TH1F( "h_ShowerE_2_2", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_2 );
    TH1F* h_ShowerE_2_3 = new TH1F( "h_ShowerE_2_3", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_3 );
    TH1F* h_ShowerE_2_4 = new TH1F( "h_ShowerE_2_4", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_4 );
    TH1F* h_ShowerE_2_5 = new TH1F( "h_ShowerE_2_5", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_5 );
    TH1F* h_ShowerE_2_6 = new TH1F( "h_ShowerE_2_6", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_6 );
    TH1F* h_ShowerE_2_7 = new TH1F( "h_ShowerE_2_7", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_7 );
    TH1F* h_ShowerE_2_8 = new TH1F( "h_ShowerE_2_8", "ECL Shower Energy", 100, 0., 1.0 );    fOutput->Add( h_ShowerE_2_8 );
    TH1F* h_ShowerE_3_0 = new TH1F( "h_ShowerE_3_0", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_0 );
    TH1F* h_ShowerE_3_1 = new TH1F( "h_ShowerE_3_1", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_1 );
    TH1F* h_ShowerE_3_2 = new TH1F( "h_ShowerE_3_2", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_2 );
    TH1F* h_ShowerE_3_3 = new TH1F( "h_ShowerE_3_3", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_3 );
    TH1F* h_ShowerE_3_4 = new TH1F( "h_ShowerE_3_4", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_4 );
    TH1F* h_ShowerE_3_5 = new TH1F( "h_ShowerE_3_5", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_5 );
    TH1F* h_ShowerE_3_6 = new TH1F( "h_ShowerE_3_6", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_6 );
    TH1F* h_ShowerE_3_7 = new TH1F( "h_ShowerE_3_7", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_7 );
    TH1F* h_ShowerE_3_8 = new TH1F( "h_ShowerE_3_8", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_3_8 );
    TH1F* h_ShowerE_4_0 = new TH1F( "h_ShowerE_4_0", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_0 );
    TH1F* h_ShowerE_4_1 = new TH1F( "h_ShowerE_4_1", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_1 );
    TH1F* h_ShowerE_4_2 = new TH1F( "h_ShowerE_4_2", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_2 );
    TH1F* h_ShowerE_4_3 = new TH1F( "h_ShowerE_4_3", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_3 );
    TH1F* h_ShowerE_4_4 = new TH1F( "h_ShowerE_4_4", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_4 );
    TH1F* h_ShowerE_4_5 = new TH1F( "h_ShowerE_4_5", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_5 );
    TH1F* h_ShowerE_4_6 = new TH1F( "h_ShowerE_4_6", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_6 );
    TH1F* h_ShowerE_4_7 = new TH1F( "h_ShowerE_4_7", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_7 );
    TH1F* h_ShowerE_4_8 = new TH1F( "h_ShowerE_4_8", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_4_8 );
    TH1F* h_ShowerE_5_0 = new TH1F( "h_ShowerE_5_0", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_0 );
    TH1F* h_ShowerE_5_1 = new TH1F( "h_ShowerE_5_1", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_1 );
    TH1F* h_ShowerE_5_2 = new TH1F( "h_ShowerE_5_2", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_2 );
    TH1F* h_ShowerE_5_3 = new TH1F( "h_ShowerE_5_3", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_3 );
    TH1F* h_ShowerE_5_4 = new TH1F( "h_ShowerE_5_4", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_4 );
    TH1F* h_ShowerE_5_5 = new TH1F( "h_ShowerE_5_5", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_5 );
    TH1F* h_ShowerE_5_6 = new TH1F( "h_ShowerE_5_6", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_6 );
    TH1F* h_ShowerE_5_7 = new TH1F( "h_ShowerE_5_7", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_7 );
    TH1F* h_ShowerE_5_8 = new TH1F( "h_ShowerE_5_8", "ECL Shower Energy", 200, 0., 2.0 );    fOutput->Add( h_ShowerE_5_8 );
    TH1F* h_ShowerE_6_0 = new TH1F( "h_ShowerE_6_0", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_0 );
    TH1F* h_ShowerE_6_1 = new TH1F( "h_ShowerE_6_1", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_1 );
    TH1F* h_ShowerE_6_2 = new TH1F( "h_ShowerE_6_2", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_2 );
    TH1F* h_ShowerE_6_3 = new TH1F( "h_ShowerE_6_3", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_3 );
    TH1F* h_ShowerE_6_4 = new TH1F( "h_ShowerE_6_4", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_4 );
    TH1F* h_ShowerE_6_5 = new TH1F( "h_ShowerE_6_5", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_5 );
    TH1F* h_ShowerE_6_6 = new TH1F( "h_ShowerE_6_6", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_6 );
    TH1F* h_ShowerE_6_7 = new TH1F( "h_ShowerE_6_7", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_7 );
    TH1F* h_ShowerE_6_8 = new TH1F( "h_ShowerE_6_8", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_6_8 );
    TH1F* h_ShowerE_7_0 = new TH1F( "h_ShowerE_7_0", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_0 );
    TH1F* h_ShowerE_7_1 = new TH1F( "h_ShowerE_7_1", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_1 );
    TH1F* h_ShowerE_7_2 = new TH1F( "h_ShowerE_7_2", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_2 );
    TH1F* h_ShowerE_7_3 = new TH1F( "h_ShowerE_7_3", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_3 );
    TH1F* h_ShowerE_7_4 = new TH1F( "h_ShowerE_7_4", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_4 );
    TH1F* h_ShowerE_7_5 = new TH1F( "h_ShowerE_7_5", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_5 );
    TH1F* h_ShowerE_7_6 = new TH1F( "h_ShowerE_7_6", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_6 );
    TH1F* h_ShowerE_7_7 = new TH1F( "h_ShowerE_7_7", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_7 );
    TH1F* h_ShowerE_7_8 = new TH1F( "h_ShowerE_7_8", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_7_8 );
    TH1F* h_ShowerE_8_0 = new TH1F( "h_ShowerE_8_0", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_0 );
    TH1F* h_ShowerE_8_1 = new TH1F( "h_ShowerE_8_1", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_1 );
    TH1F* h_ShowerE_8_2 = new TH1F( "h_ShowerE_8_2", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_2 );
    TH1F* h_ShowerE_8_3 = new TH1F( "h_ShowerE_8_3", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_3 );
    TH1F* h_ShowerE_8_4 = new TH1F( "h_ShowerE_8_4", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_4 );
    TH1F* h_ShowerE_8_5 = new TH1F( "h_ShowerE_8_5", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_5 );
    TH1F* h_ShowerE_8_6 = new TH1F( "h_ShowerE_8_6", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_6 );
    TH1F* h_ShowerE_8_7 = new TH1F( "h_ShowerE_8_7", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_7 );
    TH1F* h_ShowerE_8_8 = new TH1F( "h_ShowerE_8_8", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_8_8 );
    TH1F* h_ShowerE_9_0 = new TH1F( "h_ShowerE_9_0", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_0 );
    TH1F* h_ShowerE_9_1 = new TH1F( "h_ShowerE_9_1", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_1 );
    TH1F* h_ShowerE_9_2 = new TH1F( "h_ShowerE_9_2", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_2 );
    TH1F* h_ShowerE_9_3 = new TH1F( "h_ShowerE_9_3", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_3 );
    TH1F* h_ShowerE_9_4 = new TH1F( "h_ShowerE_9_4", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_4 );
    TH1F* h_ShowerE_9_5 = new TH1F( "h_ShowerE_9_5", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_5 );
    TH1F* h_ShowerE_9_6 = new TH1F( "h_ShowerE_9_6", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_6 );
    TH1F* h_ShowerE_9_7 = new TH1F( "h_ShowerE_9_7", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_7 );
    TH1F* h_ShowerE_9_8 = new TH1F( "h_ShowerE_9_8", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_9_8 );
    TH1F* h_ShowerE_10_0 = new TH1F( "h_ShowerE_10_0", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_0 );
    TH1F* h_ShowerE_10_1 = new TH1F( "h_ShowerE_10_1", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_1 );
    TH1F* h_ShowerE_10_2 = new TH1F( "h_ShowerE_10_2", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_2 );
    TH1F* h_ShowerE_10_3 = new TH1F( "h_ShowerE_10_3", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_3 );
    TH1F* h_ShowerE_10_4 = new TH1F( "h_ShowerE_10_4", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_4 );
    TH1F* h_ShowerE_10_5 = new TH1F( "h_ShowerE_10_5", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_5 );
    TH1F* h_ShowerE_10_6 = new TH1F( "h_ShowerE_10_6", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_6 );
    TH1F* h_ShowerE_10_7 = new TH1F( "h_ShowerE_10_7", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_7 );
    TH1F* h_ShowerE_10_8 = new TH1F( "h_ShowerE_10_8", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerE_10_8 );

    TH1F* h_ShowerEnergy = new TH1F( "h_ShowerEnergy", "ECL Shower Energy", 300, 0., 6.0 );    fOutput->Add( h_ShowerEnergy );
    TH1F* h_trkP = new TH1F( "h_trkP", "Reconstructed Track Momentum", 300, 0., 6.0 );    fOutput->Add( h_trkP );

    TH1F* h_trkTheta = new TH1F( "h_trkTheta", "Reconstructed Track Polar Angle", 100, 0., 180. );    fOutput->Add( h_trkTheta );
    TH1F* h_ShowerTheta = new TH1F( "h_ShowerTheta", "ECL Shower Polar Angle", 100, 0., 180. );    fOutput->Add( h_ShowerTheta );
    TH1F* h_ShowerPhi = new TH1F( "h_ShowerPhi", "ECL Shower Azimuthal Angle", 100, -180., 180. );    fOutput->Add( h_ShowerPhi );

    TH1F* h_Eop = new TH1F( "h_Eop", "ECL E/p", 200, 0., 2.6 );    fOutput->Add( h_Eop );
    TH1F* h_EoEt = new TH1F( "h_EoEt", "Shower Energy on True MC Energy", 200, 0., 2.0 );    fOutput->Add( h_EoEt );
    TH1F* h_PoPt = new TH1F( "h_PoPt", "Track Momentum on True MC Momentum", 200, 0., 2.0 );    fOutput->Add( h_PoPt );

    TH2F* h_recon_momVtheta = new TH2F("h_recon_momVtheta", "Reconstructed Track Momentum against Reconstructed Track Theta", 300, 0., 6, 180, 0., 180);    fOutput->Add( h_recon_momVtheta );

    TH2F* h_theta_truevrecon = new TH2F("h_theta_truevrecon", "True MC Particle Theta against Reconstructed Track Theta", 100, 0., 180, 100, 0., 180);    fOutput->Add( h_theta_truevrecon );
    TH2F* h_EoPvtheta = new TH2F("h_EoPvtheta", "E/P against Reconstructed Track Theta", 150, 0., 1.5, 150, 0., 180);    fOutput->Add( h_EoPvtheta );
    TH2F* h_EonEtrue = new TH2F("h_EonEtrue", "Shower Energy against True MC Energy", 150, 0., 6.0, 150, 0., 6.0);    fOutput->Add( h_EonEtrue );
    TH2F* h_PonPtrue = new TH2F("h_PonPtrue", "Track Momentum against True MC Momentum", 150, 0., 6.0, 150, 0., 6.0);    fOutput->Add( h_PonPtrue );

    // TH1F* h_ShowerZernike11 = new TH1F( "h_ShowerZernike11", "ECL Shower Zernike Moment 11", 150, 0., 0.24);    fOutput->Add( h_ShowerZernike11 );
    // TH1F* h_ShowerZernike20 = new TH1F( "h_ShowerZernike20", "ECL Shower Zernike Moment 20", 150, 0., 1.  );    fOutput->Add( h_ShowerZernike20 );
    // TH1F* h_ShowerZernike22 = new TH1F( "h_ShowerZernike22", "ECL Shower Zernike Moment 22", 150, 0., 0.8 );    fOutput->Add( h_ShowerZernike22 );
    // TH1F* h_ShowerZernike31 = new TH1F( "h_ShowerZernike31", "ECL Shower Zernike Moment 31", 150, 0., 0.9 );    fOutput->Add( h_ShowerZernike31 );
    // TH1F* h_ShowerZernike33 = new TH1F( "h_ShowerZernike33", "ECL Shower Zernike Moment 33", 150, 0., 0.2 );    fOutput->Add( h_ShowerZernike33 );
    // TH1F* h_ShowerZernike40 = new TH1F( "h_ShowerZernike40", "ECL Shower Zernike Moment 40", 150, 0., 1.6 );    fOutput->Add( h_ShowerZernike40 );
    // TH1F* h_ShowerZernike42 = new TH1F( "h_ShowerZernike42", "ECL Shower Zernike Moment 42", 150, 0., 0.8 );    fOutput->Add( h_ShowerZernike42 );
    // TH1F* h_ShowerZernike44 = new TH1F( "h_ShowerZernike44", "ECL Shower Zernike Moment 44", 150, 0., 0.06 );    fOutput->Add( h_ShowerZernike44 );
    // TH1F* h_ShowerZernike51 = new TH1F( "h_ShowerZernike51", "ECL Shower Zernike Moment 51", 150, 0., 1.2 );    fOutput->Add( h_ShowerZernike51 );
    // TH1F* h_ShowerZernike53 = new TH1F( "h_ShowerZernike53", "ECL Shower Zernike Moment 53", 150, 0., 0.4 );    fOutput->Add( h_ShowerZernike53 );
    // TH1F* h_ShowerZernike55 = new TH1F( "h_ShowerZernike55", "ECL Shower Zernike Moment 55", 150, 0., 0.06 );    fOutput->Add( h_ShowerZernike55 );

    // TH2F* h_ShowerEnergy_Z11 = new TH2F("h_ShowerEnergy_Z11", "Correlation of Shower Energy and Zernike Moment 11", 50, 0., 5.5, 50, 0., 0.24);    fOutput->Add( h_ShowerEnergy_Z11 );
    // TH2F* h_ShowerEnergy_Z20 = new TH2F("h_ShowerEnergy_Z20", "Correlation of Shower Energy and Zernike Moment 20", 50, 0., 5.5, 50, 0., 1.  );    fOutput->Add( h_ShowerEnergy_Z20 );
    // TH2F* h_ShowerEnergy_Z22 = new TH2F("h_ShowerEnergy_Z22", "Correlation of Shower Energy and Zernike Moment 22", 50, 0., 5.5, 50, 0., 0.8 );    fOutput->Add( h_ShowerEnergy_Z22 );
    // TH2F* h_ShowerEnergy_Z31 = new TH2F("h_ShowerEnergy_Z31", "Correlation of Shower Energy and Zernike Moment 31", 50, 0., 5.5, 50, 0., 0.9 );    fOutput->Add( h_ShowerEnergy_Z31 );
    // TH2F* h_ShowerEnergy_Z33 = new TH2F("h_ShowerEnergy_Z33", "Correlation of Shower Energy and Zernike Moment 33", 50, 0., 5.5, 50, 0., 0.2 );    fOutput->Add( h_ShowerEnergy_Z33 );
    // TH2F* h_ShowerEnergy_Z40 = new TH2F("h_ShowerEnergy_Z40", "Correlation of Shower Energy and Zernike Moment 40", 50, 0., 5.5, 50, 0., 1.6 );    fOutput->Add( h_ShowerEnergy_Z40 );
    // TH2F* h_ShowerEnergy_Z42 = new TH2F("h_ShowerEnergy_Z42", "Correlation of Shower Energy and Zernike Moment 42", 50, 0., 5.5, 50, 0., 0.8 );    fOutput->Add( h_ShowerEnergy_Z42 );
    // TH2F* h_ShowerEnergy_Z44 = new TH2F("h_ShowerEnergy_Z44", "Correlation of Shower Energy and Zernike Moment 44", 50, 0., 5.5, 50, 0., 0.1);    fOutput->Add( h_ShowerEnergy_Z44 );
    // TH2F* h_ShowerEnergy_Z51 = new TH2F("h_ShowerEnergy_Z51", "Correlation of Shower Energy and Zernike Moment 51", 50, 0., 5.5, 50, 0., 1.2 );    fOutput->Add( h_ShowerEnergy_Z51 );
    // TH2F* h_ShowerEnergy_Z53 = new TH2F("h_ShowerEnergy_Z53", "Correlation of Shower Energy and Zernike Moment 53", 50, 0., 5.5, 50, 0., 0.4 );    fOutput->Add( h_ShowerEnergy_Z53 );
    // TH2F* h_ShowerEnergy_Z55 = new TH2F("h_ShowerEnergy_Z55", "Correlation of Shower Energy and Zernike Moment 55", 50, 0., 5.5, 50, 0., 0.1);    fOutput->Add( h_ShowerEnergy_Z55 );

    // TH2F* h_ShowerMom_Z11 = new TH2F("h_ShowerMom_Z11", "Correlation of Track Momentum and Zernike Moment 11", 50, 0., 5., 50, 0., 0.24);    fOutput->Add( h_ShowerMom_Z11 );
    // TH2F* h_ShowerMom_Z20 = new TH2F("h_ShowerMom_Z20", "Correlation of Track Momentum and Zernike Moment 20", 50, 0., 5., 50, 0., 1.  );    fOutput->Add( h_ShowerMom_Z20 );
    // TH2F* h_ShowerMom_Z22 = new TH2F("h_ShowerMom_Z22", "Correlation of Track Momentum and Zernike Moment 22", 50, 0., 5., 50, 0., 0.8 );    fOutput->Add( h_ShowerMom_Z22 );
    // TH2F* h_ShowerMom_Z31 = new TH2F("h_ShowerMom_Z31", "Correlation of Track Momentum and Zernike Moment 31", 50, 0., 5., 50, 0., 0.9 );    fOutput->Add( h_ShowerMom_Z31 );
    // TH2F* h_ShowerMom_Z33 = new TH2F("h_ShowerMom_Z33", "Correlation of Track Momentum and Zernike Moment 33", 50, 0., 5., 50, 0., 0.2 );    fOutput->Add( h_ShowerMom_Z33 );
    // TH2F* h_ShowerMom_Z40 = new TH2F("h_ShowerMom_Z40", "Correlation of Track Momentum and Zernike Moment 40", 50, 0., 5., 50, 0., 1.6 );    fOutput->Add( h_ShowerMom_Z40 );
    // TH2F* h_ShowerMom_Z42 = new TH2F("h_ShowerMom_Z42", "Correlation of Track Momentum and Zernike Moment 42", 50, 0., 5., 50, 0., 0.8 );    fOutput->Add( h_ShowerMom_Z42 );
    // TH2F* h_ShowerMom_Z44 = new TH2F("h_ShowerMom_Z44", "Correlation of Track Momentum and Zernike Moment 44", 50, 0., 5., 50, 0., 0.1);    fOutput->Add( h_ShowerMom_Z44 );
    // TH2F* h_ShowerMom_Z51 = new TH2F("h_ShowerMom_Z51", "Correlation of Track Momentum and Zernike Moment 51", 50, 0., 5., 50, 0., 1.2 );    fOutput->Add( h_ShowerMom_Z51 );
    // TH2F* h_ShowerMom_Z53 = new TH2F("h_ShowerMom_Z53", "Correlation of Track Momentum and Zernike Moment 53", 50, 0., 5., 50, 0., 0.4 );    fOutput->Add( h_ShowerMom_Z53 );
    // TH2F* h_ShowerMom_Z55 = new TH2F("h_ShowerMom_Z55", "Correlation of Track Momentum and Zernike Moment 55", 50, 0., 5., 50, 0., 0.1);    fOutput->Add( h_ShowerMom_Z55 );

    // TH2F* h_ShowerTheta_Z11 = new TH2F("h_ShowerTheta_Z11", "Correlation of Shower #theta and Zernike Moment 11", 50, 0., 180., 50, 0., 0.24);    fOutput->Add( h_ShowerTheta_Z11 );
    // TH2F* h_ShowerTheta_Z20 = new TH2F("h_ShowerTheta_Z20", "Correlation of Shower #theta and Zernike Moment 20", 50, 0., 180., 50, 0., 1.  );    fOutput->Add( h_ShowerTheta_Z20 );
    // TH2F* h_ShowerTheta_Z22 = new TH2F("h_ShowerTheta_Z22", "Correlation of Shower #theta and Zernike Moment 22", 50, 0., 180., 50, 0., 0.8 );    fOutput->Add( h_ShowerTheta_Z22 );
    // TH2F* h_ShowerTheta_Z31 = new TH2F("h_ShowerTheta_Z31", "Correlation of Shower #theta and Zernike Moment 31", 50, 0., 180., 50, 0., 0.9 );    fOutput->Add( h_ShowerTheta_Z31 );
    // TH2F* h_ShowerTheta_Z33 = new TH2F("h_ShowerTheta_Z33", "Correlation of Shower #theta and Zernike Moment 33", 50, 0., 180., 50, 0., 0.2 );    fOutput->Add( h_ShowerTheta_Z33 );
    // TH2F* h_ShowerTheta_Z40 = new TH2F("h_ShowerTheta_Z40", "Correlation of Shower #theta and Zernike Moment 40", 50, 0., 180., 50, 0., 1.6 );    fOutput->Add( h_ShowerTheta_Z40 );
    // TH2F* h_ShowerTheta_Z42 = new TH2F("h_ShowerTheta_Z42", "Correlation of Shower #theta and Zernike Moment 42", 50, 0., 180., 50, 0., 0.8 );    fOutput->Add( h_ShowerTheta_Z42 );
    // TH2F* h_ShowerTheta_Z44 = new TH2F("h_ShowerTheta_Z44", "Correlation of Shower #theta and Zernike Moment 44", 50, 0., 180., 50, 0., 0.1 );    fOutput->Add( h_ShowerTheta_Z44 );
    // TH2F* h_ShowerTheta_Z51 = new TH2F("h_ShowerTheta_Z51", "Correlation of Shower #theta and Zernike Moment 51", 50, 0., 180., 50, 0., 1.2 );    fOutput->Add( h_ShowerTheta_Z51 );
    // TH2F* h_ShowerTheta_Z53 = new TH2F("h_ShowerTheta_Z53", "Correlation of Shower #theta and Zernike Moment 53", 50, 0., 180., 50, 0., 0.4 );    fOutput->Add( h_ShowerTheta_Z53 );
    // TH2F* h_ShowerTheta_Z55 = new TH2F("h_ShowerTheta_Z55", "Correlation of Shower #theta and Zernike Moment 55", 50, 0., 180., 50, 0., 0.1 );    fOutput->Add( h_ShowerTheta_Z55 );

    // TH2F* h_ShowerPhi_Z11 = new TH2F("h_ShowerPhi_Z11", "Correlation of Shower #phi and Zernike Moment 11", 50, -180., 180., 50, 0., 0.24);    fOutput->Add( h_ShowerPhi_Z11 );
    // TH2F* h_ShowerPhi_Z20 = new TH2F("h_ShowerPhi_Z20", "Correlation of Shower #phi and Zernike Moment 20", 50, -180., 180., 50, 0., 1.  );    fOutput->Add( h_ShowerPhi_Z20 );
    // TH2F* h_ShowerPhi_Z22 = new TH2F("h_ShowerPhi_Z22", "Correlation of Shower #phi and Zernike Moment 22", 50, -180., 180., 50, 0., 0.8 );    fOutput->Add( h_ShowerPhi_Z22 );
    // TH2F* h_ShowerPhi_Z31 = new TH2F("h_ShowerPhi_Z31", "Correlation of Shower #phi and Zernike Moment 31", 50, -180., 180., 50, 0., 0.9 );    fOutput->Add( h_ShowerPhi_Z31 );
    // TH2F* h_ShowerPhi_Z33 = new TH2F("h_ShowerPhi_Z33", "Correlation of Shower #phi and Zernike Moment 33", 50, -180., 180., 50, 0., 0.2 );    fOutput->Add( h_ShowerPhi_Z33 );
    // TH2F* h_ShowerPhi_Z40 = new TH2F("h_ShowerPhi_Z40", "Correlation of Shower #phi and Zernike Moment 40", 50, -180., 180., 50, 0., 1.6 );    fOutput->Add( h_ShowerPhi_Z40 );
    // TH2F* h_ShowerPhi_Z42 = new TH2F("h_ShowerPhi_Z42", "Correlation of Shower #phi and Zernike Moment 42", 50, -180., 180., 50, 0., 0.8 );    fOutput->Add( h_ShowerPhi_Z42 );
    // TH2F* h_ShowerPhi_Z44 = new TH2F("h_ShowerPhi_Z44", "Correlation of Shower #phi and Zernike Moment 44", 50, -180., 180., 50, 0., 0.1 );    fOutput->Add( h_ShowerPhi_Z44 );
    // TH2F* h_ShowerPhi_Z51 = new TH2F("h_ShowerPhi_Z51", "Correlation of Shower #phi and Zernike Moment 51", 50, -180., 180., 50, 0., 1.2 );    fOutput->Add( h_ShowerPhi_Z51 );
    // TH2F* h_ShowerPhi_Z53 = new TH2F("h_ShowerPhi_Z53", "Correlation of Shower #phi and Zernike Moment 53", 50, -180., 180., 50, 0., 0.4 );    fOutput->Add( h_ShowerPhi_Z53 );
    // TH2F* h_ShowerPhi_Z55 = new TH2F("h_ShowerPhi_Z55", "Correlation of Shower #phi and Zernike Moment 55", 50, -180., 180., 50, 0., 0.1 );    fOutput->Add( h_ShowerPhi_Z55 );

}

void eclChargedPidSelector::SlaveBegin(TTree * /*tree*/)
{
    // The SlaveBegin() function is called after the Begin() function.
    // When running with PROOF SlaveBegin() is called on each slave server.
    // The tree argument is deprecated (on PROOF 0 is passed).

    TString option = GetOption();

}

Bool_t eclChargedPidSelector::Process(Long64_t entry)
{
    // The Process() function is called for each entry in the tree (or possibly
    // keyed object in the case of PROOF) to be processed. The entry argument
    // specifies which entry in the currently loaded tree is to be processed.
    // It can be passed to either eclChargedPidSelector::GetEntry() or TBranch::GetEntry()
    // to read either all or the required parts of the data. When processing
    // keyed objects with PROOF, the object is already loaded and is available
    // via the fObject pointer.
    //
    // This function should contain the "body" of the analysis. It can contain
    // simple or elaborate selection criteria, run algorithms on the data
    // of the event and typically fill histograms.
    //
    // The processing can be stopped by calling Abort().
    //
    // Use fStatus to set the return value of TTree::Process().
    //
    // The return value is currently not used.

    fChain->GetEntry(entry);

    ( (TH1F*)fOutput->FindObject( "hflow" ) )->Fill( 0 );

    if(trkMultip!=1) return kTRUE;
    ( (TH1F*)fOutput->FindObject( "hflow" ) )->Fill( 1 );

    if(eclShowerMultip!=1) return kTRUE;
    ( (TH1F*)fOutput->FindObject( "hflow" ) )->Fill( 2 );

    UInt_t i = (eclShowerEnergy->size()) - 1;
    if( (eclShowerEnergy->at(i)) < 0.03 ) return kTRUE;
    ( (TH1F*)fOutput->FindObject( "hflow" ) )->Fill( 3 );


    if( (trkP->at(i)>=0.300) && (trkP->at(i)<0.400) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_0" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_0_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_0_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=0.400) && (trkP->at(i)<0.500) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_0" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_1_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_1_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=0.500) && (trkP->at(i)<0.750) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_0" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_2_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_2_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=0.750) && (trkP->at(i)<1.000) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_0" ) )->Fill( eclShowerEnergy->at(i) );
 	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_3_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_3_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=1.000) && (trkP->at(i)<1.500) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_0" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_4_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_4_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=1.500) && (trkP->at(i)<2.000) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_0" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_0" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_1" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_1" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_2" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_2" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_3" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_3" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_4" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_4" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_5" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_5" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_6" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_6" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_7" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_7" ) )->Fill( eclShowerEnergy->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_5_8" ) )->Fill( eclEoP->at(i) );
	    ( (TH1F*)fOutput->FindObject( "h_ShowerE_5_8" ) )->Fill( eclShowerEnergy->at(i) );
	}
    }

    else if( (trkP->at(i)>=2.000) && (trkP->at(i)<3.000) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_0" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_1" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_2" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_3" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_4" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_5" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_6" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_7" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_6_8" ) )->Fill( eclEoP->at(i) );
	}
    }

    else if( (trkP->at(i)>=3.000) && (trkP->at(i)<4.000) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_0" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_1" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_2" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_3" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_4" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_5" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_6" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_7" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_7_8" ) )->Fill( eclEoP->at(i) );
	}
    }

    else if( (trkP->at(i)>=4.000) && (trkP->at(i)<4.500) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_0" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_1" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_2" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_3" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_4" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_5" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_6" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_7" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_8_8" ) )->Fill( eclEoP->at(i) );
	}
    }

    else if( (trkP->at(i)>=4.500) && (trkP->at(i)<5.000) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_0" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_1" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_2" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_3" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_4" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_5" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_6" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_7" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_9_8" ) )->Fill( eclEoP->at(i) );
	}
    }

    else if( (trkP->at(i)>=5.000) && (trkP->at(i)<5.500) ){
	if( (TMath::RadToDeg()*(trkTheta->at(i))>=0) && (TMath::RadToDeg()*(trkTheta->at(i))<17) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_0" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=17) && (TMath::RadToDeg()*(trkTheta->at(i))<31.4) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_1" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=31.4) && (TMath::RadToDeg()*(trkTheta->at(i))<32.2) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_2" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=32.2) && (TMath::RadToDeg()*(trkTheta->at(i))<44) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_3" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=44) && (TMath::RadToDeg()*(trkTheta->at(i))<117) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_4" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=117) && (TMath::RadToDeg()*(trkTheta->at(i))<128.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_5" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=128.7) && (TMath::RadToDeg()*(trkTheta->at(i))<130.7) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_6" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=130.7) && (TMath::RadToDeg()*(trkTheta->at(i))<150) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_7" ) )->Fill( eclEoP->at(i) );
	}
	else if( (TMath::RadToDeg()*(trkTheta->at(i))>=150) && (TMath::RadToDeg()*(trkTheta->at(i))<180) ){
	    ( (TH1F*)fOutput->FindObject( "h_Eop_10_8" ) )->Fill( eclEoP->at(i) );
	}
    }


    ( (TH1F*)fOutput->FindObject( "h_ShowerEnergy" ) )->Fill( eclShowerEnergy->at(i) );

    ( (TH1F*)fOutput->FindObject( "h_trkP" ) )->Fill( trkP->at(i) );
    ( (TH1F*)fOutput->FindObject( "h_Eop" ) )->Fill( eclEoP->at(i) );

    ( (TH1F*)fOutput->FindObject( "h_EoEt" ) )->Fill( (eclShowerEnergy->at(i))/(mcEnergy->at(i)) );
    ( (TH1F*)fOutput->FindObject( "h_PoPt" ) )->Fill( (trkP->at(i))/(mcP->at(i)) );

    ( (TH1F*)fOutput->FindObject( "h_trkTheta" ) )->Fill( 57.296*(trkTheta->at(i)) );
    ( (TH1F*)fOutput->FindObject( "h_ShowerTheta" ) )->Fill( 57.296*(eclShowerTheta->at(i)) );
    ( (TH1F*)fOutput->FindObject( "h_ShowerPhi" ) )->Fill( 57.296*(eclShowerPhi->at(i)) );

    ( (TH2F*)fOutput->FindObject( "h_recon_momVtheta" ) )->Fill( trkP->at(i), 57.296*(trkTheta->at(i)) );

    ( (TH2F*)fOutput->FindObject( "h_theta_truevrecon" ) )->Fill( 57.296*(mcTheta->at(i)), 57.296*(trkTheta->at(i)) );
    ( (TH2F*)fOutput->FindObject( "h_EoPvtheta" ) )->Fill( eclEoP->at(i), 57.296*(trkTheta->at(i)) );
    ( (TH2F*)fOutput->FindObject( "h_PonPtrue" ) )->Fill( trkP->at(i), mcP->at(i) );
    ( (TH2F*)fOutput->FindObject( "h_EonEtrue" ) )->Fill( eclShowerEnergy->at(i), mcEnergy->at(i) );

    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike11" ) )->Fill( eclShowerAbsZernike11->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike20" ) )->Fill( eclShowerAbsZernike20->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike22" ) )->Fill( eclShowerAbsZernike22->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike31" ) )->Fill( eclShowerAbsZernike31->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike33" ) )->Fill( eclShowerAbsZernike33->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike40" ) )->Fill( eclShowerAbsZernike40->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike42" ) )->Fill( eclShowerAbsZernike42->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike44" ) )->Fill( eclShowerAbsZernike44->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike51" ) )->Fill( eclShowerAbsZernike51->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike53" ) )->Fill( eclShowerAbsZernike53->at(i) );
    // ( (TH1F*)fOutput->FindObject( "h_ShowerZernike55" ) )->Fill( eclShowerAbsZernike55->at(i) );

    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z11" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike11->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z20" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike20->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z22" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike22->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z31" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike31->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z33" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike33->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z40" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike40->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z42" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike42->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z44" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike44->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z51" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike51->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z53" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike53->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerEnergy_Z55" ) )->Fill( eclShowerEnergy->at(i), eclShowerAbsZernike55->at(i) );

    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z11" ) )->Fill( trkP->at(i), eclShowerAbsZernike11->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z20" ) )->Fill( trkP->at(i), eclShowerAbsZernike20->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z22" ) )->Fill( trkP->at(i), eclShowerAbsZernike22->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z31" ) )->Fill( trkP->at(i), eclShowerAbsZernike31->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z33" ) )->Fill( trkP->at(i), eclShowerAbsZernike33->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z40" ) )->Fill( trkP->at(i), eclShowerAbsZernike40->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z42" ) )->Fill( trkP->at(i), eclShowerAbsZernike42->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z44" ) )->Fill( trkP->at(i), eclShowerAbsZernike44->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z51" ) )->Fill( trkP->at(i), eclShowerAbsZernike51->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z53" ) )->Fill( trkP->at(i), eclShowerAbsZernike53->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerMom_Z55" ) )->Fill( trkP->at(i), eclShowerAbsZernike55->at(i) );


    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z11" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike11->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z20" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike20->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z22" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike22->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z31" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike31->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z33" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike33->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z40" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike40->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z42" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike42->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z44" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike44->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z51" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike51->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z53" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike53->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerTheta_Z55" ) )->Fill( 57.296*(eclShowerTheta->at(i)), eclShowerAbsZernike55->at(i) );

    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z11" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike11->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z20" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike20->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z22" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike22->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z31" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike31->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z33" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike33->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z40" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike40->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z42" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike42->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z44" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike44->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z51" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike51->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z53" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike53->at(i) );
    // ( (TH2F*)fOutput->FindObject( "h_ShowerPhi_Z55" ) )->Fill( 57.296*(eclShowerPhi->at(i)), eclShowerAbsZernike55->at(i) );

    return kTRUE;
}

void eclChargedPidSelector::SlaveTerminate()
{
    // The SlaveTerminate() function is called after all entries or objects
    // have been processed. When running with PROOF SlaveTerminate() is called
    // on each slave server.

}

void eclChargedPidSelector::SetOutputDir(const char* outpath)
{
    string outpath_str = string(outpath);

    string tmp = gFile->GetPath();
    tmp = tmp.replace(tmp.length()-2,tmp.length(),"");
    size_t pdgpos = tmp.find("pdg");
    fOutfile = outpath_str + "/" + tmp.substr(pdgpos);

    printf("Output file: %s\n", fOutfile.c_str());
}

void eclChargedPidSelector::Terminate()
{
    // The Terminate() function is the last function to be called during
    // a query. It always runs on the client, it can be used to present
    // the results graphically or save the results to file.

    TFile* histfile = new TFile( fOutfile.c_str(), "RECREATE" );
    TIter next_object( fOutput );
    TObject* obj;
    histfile->cd();
    while ( ( obj = next_object() ) ){
	obj->Write();
    }
    histfile->Close();

    // Clear the TSelectorList
    fOutput->Clear();
}

