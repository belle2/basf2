/*
<header>
<input>SVDValidationTTree.root</input>
<input>SVDValidationTTreeStrip.root</input>
<input>SVDValidationTTreeSimhit.root</input>
<description>
    This ROOT macro is used for the SVD validation. It creates several
    histograms, divided by variable,layer number, strip direction (U, V), 
    type of sensor (barrel or slanted, Layer3 type is called barrel too for simplicity's sake)
    and saves them to a ROOT file.
</description>
<contact>G.Caria, gcaria@student.unimelb.edu.au</contact>
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
void addDetails(TH1F* h, const char* descr, const char* check, const char* contact_str)
{
      h->GetListOfFunctions()->Add(new TNamed("Description", descr));
      h->GetListOfFunctions()->Add(new TNamed("Check", check));
      h->GetListOfFunctions()->Add(new TNamed("Contact", contact_str));
}

// ======================================================================
void plotThis(const char *Type[], const char *Side[], const char* name, const char * title, int nbins, double xmin, double xmax, const char* x_label,
                TTree* tree, const char* expr,  
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

          const char* hName = Form("%s-%d-%s-%s",name, layer, Type[m], Side[i]);
          TH1F* h  = create1DHisto(hName, hName,
                                nbins, xmin, xmax, x_label);

          
          const char* cond = (Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 
          tree->Draw(Form("%s>>%s", expr, hName), cond);

          addDetails(h, descr, check, contact_str);

          h->Write(hName);

          delete h;
          }} 

      c->Write(Form("%s, Layer %d",title, layer));
      delete c;
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

          delete h;
          } 

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
  const char* contact_str = "G.Caria, gcaria@student.unimelb.edu.au";  

  //-------------------------------------------------------------
  // open the files with simulated and reconstructed events data
  TFile* input = TFile::Open("../SVDValidationTTree.root");
  TFile* inputStrip = TFile::Open("../SVDValidationTTreeStrip.root");
  TFile* inputSimhit = TFile::Open("../SVDValidationTTreeSimhit.root");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDValidationPlots.root", "recreate");

  //------------------------------------------------------------------  
  // loads ttrees
  TTree* tree = (TTree*) input->Get("tree");
  TTree* treeStrip = (TTree*) inputStrip->Get("tree");
  TTree* treeSimhit = (TTree*) inputSimhit->Get("tree");

  // =========================== Plotting starts here ===========================
  
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
           "Should be less", contact_str);

  // Pull
  name = "Pull";
  plotThis(Type, Side, "cPull", name, 100, -5, 5, name,
           tree, "cluster_pull", "Pull (clusterPos - truehitPos/clusterPosSignma) distributions",
           "Should be centered at 0 with RMS less than 2.0", contact_str);

// Cluster charge     
  name = "Cluster Charge";
  plotThis(Type, Side, "cClsCharge", name, 100, 0, 350, Form("%s (# electrons)",name),
           tree, "cluster_charge", Form("%s distributions", name),
           "Should peak at around 6000 electrons", contact_str);

// Strip charge   
  name = "Strip Charge";
  plotThis(Type, Side, "cStripCharge", name, 100, 0, 200, Form("%s (# electrons)",name),
           treeStrip, "strip_charge", Form("%s distributions", name),
           "Should peak at around 3000 electrons", contact_str);

// Strip SN 
  name = "Strip SN ratio";
  plotThis(Type, Side, "cStripSN", name, 100, 0, 0.3, Form("%s",name),
           treeStrip, "strip_charge/strip_noise", Form("%s distributions", name),
           "Should peak at around 20", contact_str);

// Simhits: dE/dx 
  name = "dE/dx Simhits";
  plotThisNoSideLoop(Type, "cdEdx", name, 100, 1, 10, Form("%s (MeV/cm)",name),
           treeSimhit, "simhit_dEdx*1000", Form("%s distributions", name),
           "Should peak at around 2,8 MeV/cm", contact_str);

}

