/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TFile.h"
#include "TString.h"
#include "TCanvas.h"
#include "TSystem.h"
#include <string>

#include <iostream>
using namespace std;


void GeneratePlots(TFile *file, TString ListName, string sfx, Bool_t isGroupC);

//----------------------------
void ExtractCDCdEdxValidation(string filename = "fvalidatehadron_outfile.root") {

	TFile* f = new TFile(Form("%s", filename.data()), "READ");
	if (f->IsZombie()) {
		printf("File not found"); return;
	}

	string type = filename.substr(9, 6);

	const Int_t TotalList = 3;
	bool isRGBasic = kTRUE, isRGdEdx  = kTRUE, isMeanSigmavsP = kTRUE;

	if (type == "hadron") { //for hadron only basis plot
		isRGdEdx  = kFALSE, isMeanSigmavsP = kFALSE;
	}

	int iList[TotalList + 1]  = {isRGBasic, isRGdEdx, isMeanSigmavsP, 0};
	string iListName[TotalList + 1] = {"ARBasics", "PRdedx", "MeanSigmavsP", ""};

	for ( unsigned int i = 0; iListName[i].length(); i++ ) {
		if (iList[i] == 0)continue;
		gSystem->Exec(Form("rm -rf PlotdEdx/%s && mkdir -p PlotdEdx/%s", iListName[i].data(), iListName[i].data()));
		GeneratePlots(f, iListName[i].data(), type, true); //in can group
		GeneratePlots(f, iListName[i].data(), type, false); //in ind can
	}
	f->Close();
}

//--------------------------------------
void GeneratePlots(TFile *file, TString ListName, string sfx, Bool_t isGroupC) {

	TList *InputList = (TList*)file->Get(ListName.Data());
	if (!InputList) {
		Printf("No InputList list <%s> found, exiting... ", ListName.Data());
		return;
	}

	if (InputList->GetEntries() == 0)return;

	Int_t xPad = 4, yPad = 4;
	Int_t nCan = 1;
	Int_t cHeight = 400, cWidth = 400;
	const Int_t splitX = xPad, splitY = yPad;

	if (isGroupC) {
		cHeight = 400 * splitY; cWidth = 400 * splitX;
		if (InputList->GetEntries() <= (splitX * splitY)) {
			nCan = 1;
		} else {
			nCan = 1 + int((InputList->GetEntries() - 1 ) / (splitX * splitY));
		}
	} else {
		cHeight = 400; cWidth = 400;
		nCan = (int)InputList->GetEntries();
	}

	const int nCanvas = nCan;
	std::vector<TCanvas*> cOPCanvas(nCanvas, 0);
	for (int i = 0; i < nCanvas; i++) {
		cOPCanvas[i] = new TCanvas(Form("%sCan%d", ListName.Data(), i), "Validation Plots", cWidth, cHeight);
		if (isGroupC)cOPCanvas[i]->Divide(splitX, splitY);
	}

	for (int ih = 0; ih < InputList->GetEntries(); ih++) {

		string fromR = "", toR = "", hname = "";
		Int_t fRun = 1, sRun = 1;
		Int_t iC = 1, iCd = 1;

		((TH1F*)InputList->At(ih))->SetFillColor(kYellow);

		if (isGroupC) {
			iC = ih / (splitY * splitX);
			iCd = ih - splitY * splitX * iC + 1;
			cOPCanvas[iC]->cd(iCd);

			if (sfx == "hadron") {
				if (ih >= 1 && ih < 5) {
					gPad->SetLogy(); gPad->SetLogx();
				}
			}

			if (ListName == "PRdedx" || ListName == "MeanSigmavsP") {
				if (ih == iC * splitY * splitX) {
					fRun = iC * splitY * splitX;
					sRun = iC * splitY * splitX + splitY * splitX - 1;
					if (sRun > (InputList->GetEntries() - 1))sRun = InputList->GetEntries() - 1;
					fromR = InputList->At(fRun)->GetName();
					toR = InputList->At(sRun)->GetName();
					hname = fromR;
					if (ListName == "MeanSigmavsP") {
						fromR.erase(0, fromR.find("_AR") + 3);
						toR.erase(0, toR.find("_AR") + 3);
						hname.erase(hname.find("_AR"), hname.size());
					} else {
						fromR.erase(0, fromR.find("Run"));
						toR.erase(0, toR.find("Run") + 3);
						hname.erase(hname.find("Run"), hname.size());
					}
					cOPCanvas[iC]->SetName(Form("%sfrom%sto%s", hname.data(), fromR.data(), toR.data()));
				}
			} else {
				cOPCanvas[iC]->SetName("ARBasic");
			}
		}
		else {
			if (ListName == "ARBasic" && ih >= 8 && ih <= 10)cWidth = 700;
			cOPCanvas[ih]->SetName(Form("%s", InputList->At(ih)->GetName()));
			cOPCanvas[ih]->cd();
			if (sfx == "hadron") {
				if (ih >= 1 && ih < 5) {
					cOPCanvas[ih]->SetLogy();
					cOPCanvas[ih]->SetLogx();
				}
			}
		}

		InputList->At(ih)->Draw();
	}

	for (int i = 0; i < nCanvas; i++) {
		cOPCanvas[i]->SaveAs(Form("PlotdEdx/%s/fOut%s_%s.pdf", ListName.Data(), sfx.data(), cOPCanvas[i]->GetName()));
	}
}
//EOF
