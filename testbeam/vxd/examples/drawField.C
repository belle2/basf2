// Draw vector field from file
// OR ... overlay vector field over a histogram ... read bellow
// Author: Tadeas Bilka
void drawField(){
  bool displayAngles = false;
  // set here your file name
  // The file format is as follows:
  // 1st line:
  // xmin ymin xmax ymax
  // each data line:
  // z y px py pz
  //
  // where it is supposed that z matches histogram horizontal (x) coordinate (y is y, verticall coordinate)
  // and main beam direction is in x (but px < 0, read bellow)
  ifstream file("field.txt");
  double y, z;
  double px, py, pz;

  // Size of arrow (0.01 seems to be reasonable)
  double arrow = 0.01;
  // Legend arrow size (1mrad = 0.001)
  double exampleArrowLenAngle = 0.001;
  double exampleArrowLenMomentum = 0.005;
  // Length of arrow. For pz(py)/px around 0.001, factor of 300 seems to be reasonable
  // 30 is OK for off-axis momentum about 5MeV
  double scale = 30.;
  if (displayAngles)
    scale = 300.;

  double xmin = -0.6;
  double ymin = -1.0;
  double xmax = 0.6;
  double ymax = 1.0;
  
  // Read first settings line
  file >> xmin >> ymin >> xmax >> ymax;

  //Comment following 6 lines to use this script to overlay the vector field over a previously drawn histogram
  TCanvas * c1 = new TCanvas("c1", "canvas", 600, 1000);
  TH1 *frame = gPad->DrawFrame(xmin,ymin,xmax,ymax);
  frame->GetXaxis()->SetTitle("Z (U) [cm]");
  frame->GetYaxis()->SetTitle("Y (V) [cm]"); 
  frame->SetTitle("Field of average beam direction");
  gPad->Update();  

  if (displayAngles) {
    // Legend arrow
    TArrow * ar = new TArrow(xmin, ymax + 0.03, xmin + exampleArrowLenAngle * scale, ymax + 0.03, arrow, "|>");
    ar->SetLineWidth(2);
    ar->SetLineColor(kRed);
    ar->SetFillColor(kRed);
    ar->Draw();
    // Legend arrow label  
    TText * label = new TText(xmin, ymax + 0.05, "1 mrad");
    label->Draw();
  } else {
    // Legend arrow
    TArrow * ar = new TArrow(xmin, ymax + 0.03, xmin + exampleArrowLenMomentum * scale, ymax + 0.03, arrow, "|>");
    ar->SetLineWidth(2);
    ar->SetLineColor(kRed);
    ar->SetFillColor(kRed);
    ar->Draw();
    // Legend arrow label  
    TText * label = new TText(xmin, ymax + 0.05, "5 MeV");
    label->Draw();
  }

  while (file) {
    // read one data line
    file >> z >> y >> px >> py >> pz;    
    // skip very low average pt values (possible if there is just a few of particles in a bin)
    if (abs(px) < 1.0e-9) continue;
    
    double y1 = y;
    double x1 = z;
    double x2, y2;
    if (displayAngles) {
      // (-px) because we fire beam from + to - values of x => px < 0
      y2 = py/(-px) * scale + y1;
      x2 = pz/(-px) * scale + x1;
    } else {
      y2 = py * scale + y1;
      x2 = pz * scale + x1;      
    }
    TArrow * arr = new TArrow(x1, y1, x2, y2, arrow, "|>");
    arr->SetLineWidth(2);
    arr->Draw();
  }  
}