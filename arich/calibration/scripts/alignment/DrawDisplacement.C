/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Scale factor for lengths of arrows
Double_t scalefactor = 7000;
TCanvas* c1;
void DrawDisplacement(const char* AmplitudeFile, const char* PhaseFile){
  Double_t amp[36];
  Double_t phase[36];
  ifstream fa(AmplitudeFile);
  ifstream fp(PhaseFile);
  string line;
  // Read values from data files
  for (Int_t i = 0; i < 36; i++){
    getline(fa, line);
    replace(line.begin(), line.end(), ',', ' ');
    istringstream sa(line);
    sa >> amp[i];
    getline(fp, line);
    replace(line.begin(), line.end(), ',', ' ');
    istringstream sp(line);
    sp >> phase[i];
    //cout << i << " " << amp[i] << " " << phase[i] << endl;
  }
  c1 = new TCanvas("arich", "arich", 800, 800);
  c1->Range(-120,-120,120,120);
  double r[4] = {111.5, 89., 66.5, 44.1};
  // Write ARICH and regions for alignment
  for (Int_t i = 0; i < 4; i++){
    TEllipse *e = new TEllipse(0,0,r[i]);
    if (i == 1 || i == 2) e->SetLineStyle(2);
    e->Draw();
  }
  for (Int_t i = 0; i < 12; i++){
    Double_t phi = (Double_t)i/6.*M_PI;
    TLine * l = new TLine(r[3]*cos(phi), r[3]*sin(phi),
			  r[0]*cos(phi), r[0]*sin(phi));
    l->SetLineStyle(2);
    l->Draw();
  }
  // Write Arrows that represent direction and magnitude of the displacement
  for (Int_t i = 0; i < 36; i++){
    Double_t r_c = 0.5*(r[3-i%3]+r[2-i%3]);
    Double_t phi_c = (Double_t)((i/3)-6)/6.*M_PI + M_PI/12.;
    Double_t x_c = r_c*cos(phi_c);
    Double_t y_c = r_c*sin(phi_c);
    TArrow *a = new TArrow(x_c, y_c, 
			   x_c + scalefactor*amp[i]*cos(-phase[i]),
			   y_c + scalefactor*amp[i]*sin(-phase[i]),
			   0.015, "|>");
    a->SetLineColor(4);
    a->SetFillColor(4);
    a->SetLineWidth(3);
    a->Draw();
    char str[256];
    sprintf(str, "%i", i+1);
    TText *t = new TText((r_c+5)*(cos(phi_c + M_PI/20.))-5,
			 (r_c+5)*(sin(phi_c + M_PI/20.))-5, str);
    t->SetTextSize(0.04);
    t->Draw();
  }
}
