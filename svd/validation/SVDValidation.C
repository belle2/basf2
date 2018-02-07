/*
<header>
<input>SVDValidationTTree.root</input>
<input>SVDValidationTTreeStrip.root</input>
<input>SVDValidationTTreeSpacePoint.root</input>
<input>SVDValidationTTreeSimhit.root</input>
<description>
    This ROOT macro is used for the SVD validation. It creates several
    histograms, divided by variable,layer number, strip direction (U, V), 
    type of sensor (barrel or slanted, Layer3 type is called barrel too for simplicity's sake)
    and saves them to a ROOT file.
</description>
<contact> G.Caria, gcaria@student.unimelb.edu.au
Modifed by Renu Garg, renu92garg@gmail.com </contact>
</header>
*/

#include <TStyle.h>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"

// ======================================================================
int layerToIndex(int layer){
  int layermin = 3;
  int layerIndex;
  layerIndex = layer - layermin;	      
  return layerIndex;
}

// ======================================================================
TCanvas* makeNewCanvas(const char* cname, const char* ctitle)
{
      TCanvas* c = new TCanvas(cname, ctitle, 1028, 864);
      c->Divide(2,2);
      c->Draw();
      return c;
}
  
// ======================================================================
TH1F* create1DHisto(const char* name, const char* title,
                     Int_t n_bins, Double_t x_min, Double_t x_max,
                     const char* x_label)
{ 
  TH1F* h = new TH1F(name, title, n_bins, x_min, x_max);
  h->GetXaxis()->SetTitle(x_label);
  h->GetYaxis()->SetTitle("counts");
  return h;
}

// ======================================================================
TH2F* create2DHisto(const char* name, const char* title,
		    Int_t n_bins, Double_t x_min, Double_t x_max,
		    const char* x_label,Int_t y_bins,Double_t y_min,Double_t y_max,const char* y_label)
{ 
  TH2F* h1 = new TH2F(name, title, n_bins, x_min, x_max, y_bins, y_min, y_max);
  h1->GetXaxis()->SetTitle(x_label);
  h1->GetYaxis()->SetTitle(y_label);
  return h1;
}

// ======================================================================
void addDetails(TH1F* h, const char* descr, const char* check, const char* contact_str)
{
      h->GetListOfFunctions()->Add(new TNamed("Description", descr));
      h->GetListOfFunctions()->Add(new TNamed("Check", check));
      h->GetListOfFunctions()->Add(new TNamed("Contact", contact_str));

}

void add2Details(TH2F* h, const char* descr, const char* check, const char* contact_str)
{
  h->GetListOfFunctions()->Add(new TNamed("Description", descr));
  h->GetListOfFunctions()->Add(new TNamed("Check", check));
  h->GetListOfFunctions()->Add(new TNamed("Contact", contact_str));

}

// ======================================================================
void plotThis(const char *Type[], const char *Side[], const char* name, const char * title, int nbins, double xmin, double xmax, const char* x_label,        TTree* tree, const char* expr,  
                const char* descr, const char* check, const char* contact_str)
{
   for (int layer=3; layer<=6; layer++) // loop on layers  
     {
       int layerIndex = layerToIndex(layer);
       TCanvas* c = makeNewCanvas(Form("%s-%d",name,layer),Form("%s, Layer %d",title, layer));
       int k = 0;
       for (int  m=1; m>=0; m--)  // loop over types
	 for (int i=0; i<=1; i++)  // loop over sides
          {{
	      k+=1;
	      c->cd(k);
	      
	      if ((layer == 3) && (m == 0)) continue; // skip slanted histos for layer 3 
	      
	      const char* hName = Form("%s_%d_%s_%s",name, layer, Type[m], Side[i]);
	      TH1F* h  = create1DHisto(hName, hName,
				       nbins, xmin, xmax, x_label);
	      h->SetStats(kFALSE);
	      const char* cond = (Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 
	      tree->Draw(Form("%s>>%s", expr, hName), cond);
	      
	      addDetails(h, descr, check, contact_str);
	      h->Write(hName);
	      //delete h;
	    }} 
       c->Modified();
       c->Update();
       
       c->Write(Form("%s, Layer %d",title, layer));
       delete c;
     }
}


//=================================================================================

void plotMean(const char *Type[], const char *Side[], const char* name, const char * title, int nbins, double xmin, double xmax, const char* x_label,        TTree* tree,const char* descr, const char* check, const char* contact_str)
{
  float lay[]={0,33.624,85.053,136.481,187.910,239.338,290.767,342.195,0, 8.00,44.00,80.00,116.0,152.0,188.0,224.0,260.0,296.00,332.00,0,22,52,82,112.0,142.0,172.0,202.0,232.0,262.0,292,322,352,0, 18.5,41.0,63.5,86.0,108.5,131.0,153.5,176.0,198.5,221.0,243.5,266.00,288.5,311.0,333.5,356};

  Float_t cluster_size,cluster_phi;
  Int_t layer,strip_dir,sensor_type,ladder;

  tree->SetBranchAddress("cluster_size",&cluster_size);
  tree->SetBranchAddress("cluster_phi",&cluster_phi);
  tree->SetBranchAddress("layer",&layer);
  tree->SetBranchAddress("ladder",&ladder);
  tree->SetBranchAddress("strip_dir",&strip_dir);
  tree->SetBranchAddress("sensor_type",&sensor_type);
  
  for (int j=3; j<=6; j++) // loop on layers  
    {
      int la[]={7,10,12,16};
      for (int  m=1; m>=0; m--)  // loop over types
        for (int i=0; i<=1; i++)  // loop over sides
          {{
	      if ((j == 3) && (m == 0)) continue; // skip slanted histos for layer 3 
	      const char* hName = Form("%s_%d_%s_%s",name,j, Type[m], Side[i]);

	      TH1F* h1  = create1DHisto(hName, hName,
				       la[j-3], xmin, xmax, x_label);

	      for(int k=0;k<la[j-3];k++) //loop over phi bin
		{
		  float initial,final;		  
		  const char* name1=Form("hist_%d",k);
		  TH1F *h=new TH1F(Form("%s",name1),Form("%s",name1),10,0,10);
		  if(j==3){
		    initial=lay[k];
		    final=lay[k+1];}
		  else if(j==4){
		    initial=lay[k+8];
		     final=lay[k+9];
		  }
		  else if(j==5){
		     initial=lay[k+19];
		     final=lay[k+20];
		  }
		  else{
		     initial=lay[k+32];
		     final=lay[k+33];
		  }
		  Int_t N=0;
		  for (int l=0;l<tree->GetEntries();l++)
		    {
		      tree->GetEntry(l);
		      
		      if(cluster_phi>initial&&cluster_phi<final&&layer==j&&strip_dir==i&&sensor_type==m){
			h->Fill(cluster_size);
			N=N+1;
		      }
		    }

		  float mean=h->GetMean();
		  float err=h->GetRMS()/sqrt(N);
		  gStyle->SetErrorX(0);		 
		  h1->SetBinContent(k+1,mean);
		  h1->SetBinError(k+1,err);
		  h1->GetXaxis()->SetTitle("#phi (degree)");
		  h1->GetYaxis()->SetTitle("Mean Value");
		  h1->SetMarkerStyle(20);
		  h1->SetMarkerColor(4);
		  h1->SetStats(kFALSE);

		  delete h;
		}

	      addDetails(h1, descr, check, contact_str);
	      h1->Write();
	      
	    }} 
    }
}




//===================================================================================

void plotThis2d(const char *Type[], const char *Side[], const char* name, const char * title, int nbins, double xmin, double xmax, const char* x_label,int ybins, double ymin, double ymax, const char* y_label,
                TTree* tree, const char* expr,  const char* expy,
                const char* descr, const char* check, const char* contact_str)
{
   for (int layer=3; layer<=6; layer++) // loop on layers  
      {
      int layerIndex = layerToIndex(layer);
      for (int  m=1; m>=0; m--)  // loop over types
        for (int i=0; i<=1; i++)  // loop over sides
          {{

          if ((layer == 3) && (m == 0)) continue; // skip slanted histos for layer 3 

          const char* hName = Form("%s_%d_%s_%s",name, layer, Type[m], Side[i]);

	  TH2F* h1  = create2DHisto(hName, hName,          
				   nbins, xmin, xmax, x_label,ybins,ymin,ymax,y_label);
	  h1->SetStats(kFALSE);
          const char* cond = (Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 

	  tree->Draw(Form("%s:%s>>%s",expy,expr,hName),cond);
          add2Details(h1, descr, check, contact_str);
	  h1->Write(hName);

          delete h1;
          }} 
      }
}

// ======================================================================
void plotThisNoSideLoop(const char *Type[], const char* name, const char * title, int nbins, double xmin, double xmax, const char* x_label,
                TTree* tree, const char* expr,  
                const char* descr, const char* check, const char* contact_str)
{
   for (int layer=3; layer<=6; layer++) // loop on layers  
      {
      int layerIndex = layerToIndex(layer);
      TCanvas* c = makeNewCanvas(Form("%s-%d",name,layer),Form("%s, Layer %d",title, layer));
      int k = 0;
      for (int  m=1; m>=0; m--)  // loop over types
          {
          k+=1;
          c->cd(k);

          if ((layer == 3) && (m == 0)) continue; // skip slanted histos for layer 3 

          const char* hName = Form("%s-%d-%s",name, layer, Type[m]);
          TH1F* h  = create1DHisto(hName, hName,
				   nbins, xmin, xmax, x_label);

          
          const char* cond = (Form("layer==%d&&sensor_type==%d", layer, m)); 
          tree->Draw(Form("%s>>%s", expr, hName), cond);
	  
          addDetails(h, descr, check, contact_str);
	  
          h->Write(hName);
	  
	  //delete h;
          } 
      c->Modified();
      c->Update();
      c->Write(Form("%s, Layer %d",title, layer));
      
      delete c;
      }
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

void SVDValidation()
{
  //-------------------------------------------------------------
  //Parameters for all plots
  const char *Side[] = {"U","V"};
  const char *Type[] = {"Slanted","Barrel"};
  // const char* contact_str = "G.Caria, gcaria@student.unimelb.edu.au";  
  const char* contact_str="R. Garg, renu92garg@gmail.com";
  //-------------------------------------------------------------
  // open the files with simulated and reconstructed events data
  TFile* input = TFile::Open("../SVDValidationTTree.root");
  TFile* inputStrip = TFile::Open("../SVDValidationTTreeStrip.root");
  TFile* inputSpacePoint = TFile::Open("../SVDValidationTTreeSpacePoint.root");
  TFile* inputSimhit = TFile::Open("../SVDValidationTTreeSimhit.root");
  
  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDValidation.root", "recreate");
  
  //------------------------------------------------------------------  
  // loads ttrees
  TTree* tree = (TTree*) input->Get("tree");
  TTree* treeStrip = (TTree*) inputStrip->Get("tree");
  TTree* treeSpacePoint = (TTree*) inputSpacePoint->Get("tree");
  TTree* treeSimhit = (TTree*) inputSimhit->Get("tree");
  
  // =========================== Plotting starts here ===========================
  
  // SpacePoint TimeU
  plotThisNoSideLoop(Type,  "spTimeU", "SpacePoint Time U", 200, -100, 100, "SP time u (ns)",
           treeSpacePoint, "time_u", "time of the U cluster",
           "between 20 and 40 ns", contact_str);

  // SpacePoint TimeV
  plotThisNoSideLoop(Type,  "spTimeV", "SpacePoint Time V", 200, -100, 100, "SP time v (ns)",
           treeSpacePoint, "time_v", "time of the V cluster",
           "between 20 and 40 ns", contact_str);

  // Cls size
  plotThis(Type, Side, "cSize", "Cluster size", 9, 0.5, 9.5, "Cls size",
           tree, "cluster_size", "# of strips in clusters",
           "Peak in range 2-3", contact_str);
 
  // Interstrip position
  plotThis(Type, Side, "cInterstripPos", "Interstrip position", 50, 0., 1, "Interstrip Position",
           tree, "cluster_interstripPosition", "InterstripPos = (cluster_pos %% pitch / pitch) distributions",
           "", contact_str);
  
  // Residual
  const char* name = "Residual";
  plotThis(Type, Side, "cResidual", name, 100, -0.010, 0.010, Form("%s (cm)",name),
           tree, "cluster_residual", "Residual (clusterPos - truehitPos) distributions",
           "Should be peak around zero", contact_str);
  
  // Pull
  name = "Pull";
  plotThis(Type, Side, "cPull", name, 100, -5, 5, name,
           tree, "cluster_pull", "Pull (clusterPos - truehitPos/clusterPosSignma) distributions",
           "Should be centered at 0 with RMS less than 2.0", contact_str);
  
  // Cluster charge     
  name = "Cluster Charge";
  plotThis(Type, Side, "cClsCharge", name, 50, 0, 120000, Form("%s (# electrons)",name),
           tree, "cluster_charge", Form("%s distributions", name),
           "Should peak around 20-40 ke", contact_str);
  
  // Cluster SN 
  name = "Cluster SN ratio";
  plotThis(Type, Side, "cClusterSN", name, 121, -0.5, 120.5, Form("%s",name),
         tree, "cluster_snr", Form("%s distributions", name),
         "Should peak at around 20", contact_str);

  // Strip charge   
  name = "Strip Charge";
  plotThis(Type, Side, "cStripCharge", name, 50, 0, 120000, Form("%s (# electrons)",name),
	   treeStrip, "strip_charge", Form("%s distributions", name),
	   "Should peak around 15-30 ke", contact_str);

  
  // Simhits: dE/dx 
  name = "dE/dx Simhits";
  plotThisNoSideLoop(Type, "cdEdx", name, 100, 1, 10, Form("%s (MeV/cm)",name),
  		     treeSimhit, "simhit_dEdx*1000", Form("%s distributions", name),
  		     "Should peak around 2.8 MeV/cm", contact_str);

  //mean of cluster size v/s phi
  name = "cluster size v/s phi";
  plotMean(Type, Side, "cSizeMean",name,  100, 0, 360, Form("%s",name),
           tree,"Mean of cluster size w.r.t to each ladder while error bar represent the RMS/#sqrt{N}, here N is number of entries in particular cluster",
           " ", contact_str);

  
}

