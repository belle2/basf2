void drawTRGHist(){

        gROOT->SetStyle("Plain");
        gStyle->SetOptStat(0);
        gStyle->SetOptTitle(0);
        gStyle->SetLabelSize(0.04,"xy");
        gStyle->SetTitleSize(0.04,"xy");
        gStyle->SetTitleOffset(1.2,"xy");
        gStyle->SetTitleOffset(1.2,"xy");
        gStyle->SetPadTopMargin(0.04);
        gStyle->SetPadRightMargin(0.04);
        gStyle->SetPadBottomMargin(0.25);
        gStyle->SetPadLeftMargin(0.04);

	TFile *file1;
	TFile *file2;
	TH1F *h1;
	TH1F *h2;
	TCanvas *c1;

	file1 = new TFile("TRGValidation.root","read");
	file2 = new TFile("newTRGValidation.root","read");
	h1 = (TH1F*)file1->Get("hin_expert");
	h2 = (TH1F*)file2->Get("hin_expert");
	h1->SetLineColor(2);
	h2->SetLineColor(4);

	c1=new TCanvas("c1_hin","",1600,600);
	h1->Draw("HIST");
	h2->Draw("HIST");

	int bin1 = h1->GetXaxis()->GetNbins();
 	for(int ibin = 1; ibin<=bin1; ibin++){
		double binCon1 = h1->GetBinContent(ibin);
		double binErr1 = h1->GetBinError(ibin);
		double binCon2 = h2->GetBinContent(ibin);
		double binErr2 = h2->GetBinError(ibin);
		if( abs(binCon1-binCon2) > 3*sqrt(binErr1*binErr1+binErr2*binErr2) ){
			if( abs(binCon1-binCon2) > 5*sqrt(binErr1*binErr1+binErr2*binErr2) ){
				TString binT1 = h1->GetXaxis()->GetBinLabel(ibin);
				cout<<"ERROR! Trigger bit "<<binT1<<" deviate larger than 5sigma!!"<<endl;
			}
			else{
				TString binT1 = h1->GetXaxis()->GetBinLabel(ibin);
				cout<<"Warning! Trigger bit "<<binT1<<" deviate larger than 3sigma!!"<<endl;
			}
		}
	}

	c1->SaveAs("inputBits_Compare.eps");

}
