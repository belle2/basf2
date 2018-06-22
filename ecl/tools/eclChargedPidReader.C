#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include <stdio.h>

void NewReader(){
    printf("start\n");

    int Part_Vec[] = {11, 13, 211, 321, 2212};

    for(UInt_t beambkg=1; beambkg < 2; beambkg++){
	for(UInt_t k=0; k<5; k++){

	    printf("Particle: %d\n", Part_Vec[k]);
	    TFile* input_p  = TFile::Open(Form("./InputRoot/BGx%d/pdg%d.root", beambkg, Part_Vec[k]), "read" );
	    TTree* tree_p = (TTree*)input_p->Get("n1_tree");
	    tree_p->Process( "eclChargedPidSelector.C+" );
	    input_p->Close();

	    printf("AntiParticle: %d\n", Part_Vec[k]);
	    TFile* input_ap  = TFile::Open(Form("./InputRoot/BGx%d/pdganti%d.root", beambkg, Part_Vec[k]), "read" );
	    TTree* tree_ap = (TTree*)input_ap->Get("n1_tree");
	    tree_ap->Process( "eclChargedPidSelector.C+" );
	    input_ap->Close();

	}
    }
    printf("end\n");
}
