/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TH1F.h>
#include <TF1.h>
#include <string>

class LaserCalibratorFit {
public:
  LaserCalibratorFit(int moduleID);
  ~LaserCalibratorFit();


  void setHist(TH1F* hist[512]);
  void setFitMethod(std::string method = "gauss") {m_fitMethod = method;} /** < gauss: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball */
  void setFitRange(double xmin = -200, double xmax = 200) {m_xmin = xmin; m_xmax = xmax;}
  int  fitPixel(int pixelID); /** fit a specific pixelID (0 - 511) */
  int  fitAllPixels(); /** fit all pixels in on slot */
  void writeFile(std::string outfile);

  int getModuleID() {return m_moduleID;}
  double getPixelChisq(int pixelID);
  std::string getFitMethod() {return m_fitMethod;}

private:
  TH1F* m_hist[512];
  int m_moduleID;
  TF1* m_func[512];
  std::string m_fitMethod;

  double m_xmin;
  double m_xmax;

  TF1* makeGFit(TH1F* h);
  TF1* makeCBFit(TH1F* h);
  TF1* makeCB2Fit(TH1F* h, int pixelID, bool sall);
};
