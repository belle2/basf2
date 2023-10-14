void compare(){

        gROOT->SetStyle("Pub");
        gStyle->SetOptStat(0);
        gStyle->SetOptTitle(0);
        gStyle->SetLabelSize(0.04,"xy");
        gStyle->SetTitleSize(0.04,"xy");
        gStyle->SetTitleOffset(1.,"xy");
        gStyle->SetTitleOffset(1.,"xy");
        gStyle->SetPadTopMargin(0.04);
        gStyle->SetPadRightMargin(0.06);
        gStyle->SetPadBottomMargin(0.02);
        gStyle->SetPadLeftMargin(0.08);

	TFile *file1;
	TFile *file2;
	TH1F *h1;
	TH1F *h2;
	TCanvas *c1;

	const float Ngen = 1000;

	vector<TString> PhysBits;
	PhysBits.clear();
	TString cdcBits[10] = {"fff","ffy","ffz","fzo","fso","fyo","ffb","fsb","ssb","stt"};
	TString eclBits[4] = {"hie","c4","bha3d","lml1"};
	TString klmBits[9] = {"mu_b2b","mu_eb2b","eklm2","beklm","cdcklm1","cdcklm2","seklm1","seklm2","ecleklm1"};

	for(int i = 0 ; i < 10; i++){
		PhysBits.push_back(cdcBits[i]);
	}
	for(int i = 0 ; i < 4; i++){
		PhysBits.push_back(eclBits[i]);
	}
	for(int i = 0 ; i < 9; i++){
		PhysBits.push_back(klmBits[i]);
	}


	TString types[3]={"bhabha","charged","dimuon"};
	for(int i=0;i<3;i++){
		file1 = new TFile("Hist_"+types[i]+".root","read");
		h1 = (TH1F*)file1->Get("hout");
		file2 = new TFile("../release-07/Hist_"+types[i]+".root","read");
		h2 = (TH1F*)file2->Get("hout");

		TH1F *h_rel7 = new TH1F("h_rel7","",23,0,23);
		TH1F *h_rel8 = new TH1F("h_rel8","",23,0,23);
		TH1F *h_resd = new TH1F("h_resd","",23,0,23);
		h_rel7->SetLineColor(2);
		h_rel8->SetLineColor(4);
		h_rel8->SetLineStyle(kDashed);

		h_resd->SetLineColor(1);
		h_rel7->GetYaxis()->SetTitle("efficiency");
		h_rel7->GetYaxis()->CenterTitle();
		h_rel8->GetYaxis()->SetTitle("efficiency");
		h_rel8->GetYaxis()->CenterTitle();

		h_resd->GetXaxis()->SetTitle("trg bits");
		h_resd->GetXaxis()->CenterTitle();
		h_resd->GetYaxis()->SetTitle("residual");
		h_resd->GetYaxis()->CenterTitle();
		for(int j = 0; j < 23 ; j ++){
			int ibin = j+1;
			h_rel7->GetXaxis()->SetBinLabel(ibin,PhysBits[j]);
			h_rel8->GetXaxis()->SetBinLabel(ibin,PhysBits[j]);
			h_resd->GetXaxis()->SetBinLabel(ibin,PhysBits[j]);
			float BinC1 = 0;
			float BinC2 = 0;
			bool read1 = false;
			bool read2 = false;
			for(int k = 1; k <= 160; k++){
				TString binT1 = h1->GetXaxis()->GetBinLabel(k);
				TString binT2 = h2->GetXaxis()->GetBinLabel(k);
				if(binT1 == PhysBits[j]){
					BinC1 = h1->GetBinContent(k);
					read1 = true;
				}
				if(binT2 == PhysBits[j]){
					BinC2 = h2->GetBinContent(k);
					read2 = true;
				}
				if(read1 == true && read2 == true)break;
			}
			float BinCr = BinC1 - BinC2;
			float EC1 = sqrt( (BinC1 / Ngen) * ( 1 - BinC1 / Ngen) ) / Ngen;
			float EC2 = sqrt( (BinC2 / Ngen) * ( 1 - BinC2 / Ngen) ) / Ngen;
			cout<<"res: "<<BinCr<<endl;
			h_rel8->SetBinContent(ibin, BinC1 / Ngen);
			h_rel8->SetBinError(ibin, EC1 );
			h_rel7->SetBinContent(ibin, BinC2 / Ngen);
			h_rel7->SetBinError(ibin, EC2 );
			h_resd->SetBinContent(ibin, BinCr / Ngen);
			h_resd->SetBinError(ibin, sqrt(EC1*EC1 + EC2*EC2) );
		}


		c1=new TCanvas("c1"+types[i],"",800,640);
		TPad *p1 = new TPad("p1","p1",0.0,0.325,1,1);
		p1->SetFillStyle(0);
		p1->SetFrameFillColor(0);
		p1->SetBottomMargin(0.005);
		p1->SetBorderMode(0);
		p1->Draw();
		TPad *p2 = new TPad("p2","p1",0.0,0.0,1,0.325);
		p2->SetTopMargin(0);
		p2->SetBottomMargin(0.25);
		p2->SetBorderMode(0);
		p2->SetFillStyle(0);
		p2->SetFrameFillColor(0);
		p2->Draw();

		h_rel7->GetYaxis()->SetRangeUser(0,std::max(h_rel7->GetMaximum(),h_rel8->GetMaximum())+0.1);
		p1->cd();
		h_rel7->Draw("HIST");
		h_rel8->Draw("HISTsame");

		TLegend *leg=new TLegend(0.60,0.65,0.85,0.85);
		leg->AddEntry("h_rel7","release-07","l");
		leg->AddEntry("h_rel8","release-08","l");
		leg->SetTextSize(0.05);
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetLineColor(0);
		leg->SetShadowColor(0);
		leg->Draw();

		p2->cd();
		h_resd->Draw("HIST");
		h_resd->GetXaxis()->SetLabelSize(0.10);
		h_resd->GetXaxis()->SetLabelOffset(0.02);
		h_resd->GetXaxis()->SetTitleSize(0.10);
		h_resd->GetXaxis()->SetTitleOffset(1.00);

		h_resd->GetYaxis()->SetLabelSize(0.09);
		h_resd->GetYaxis()->SetRangeUser(h_resd->GetMinimum()-0.11, h_resd->GetMaximum()+0.11);
		h_resd->GetYaxis()->SetTitleSize(0.10);
		h_resd->GetYaxis()->SetTitleOffset(0.40);
		h_resd->GetYaxis()->SetNdivisions(505);
		
		c1->SaveAs(types[i]+"_compare.png");
	}
}
