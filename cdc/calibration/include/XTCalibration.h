#include "string"
#include "TH2D.h"
#include "TH1D.h"
#include "TF1.h"
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
//#include <framework/database/DBImportObjPtr.h>
#include <cdc/dbobjects/CDCXtRelations.h>
namespace Belle2 {
  namespace CDC {
    class XTCalibration {
      //      typedef std::array<float, 3> array3; /**< angle bin info. */
    public:
      XTCalibration();      /// Constructor set the prefix to TestCalibration
      virtual ~XTCalibration() {}
      virtual void setDebug(bool debug = false) {m_debug = debug; }
      virtual void setUseDB(bool useDB = false) {m_useDB = useDB; }
      virtual void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      virtual void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      virtual void inputFileNames(std::string inputname) {m_InputRootFileNames.assign(inputname);}
      virtual void profileFileNames(std::string profileFileName) {m_ProfileFileName.assign(profileFileName);}
      virtual void useProfileFromInputXT(bool useProfileXTFromInputXT) {m_useProfileXTFromInputXT = useProfileXTFromInputXT;}
      virtual void setXTFileName(std::string name) {m_xtfile.assign(name);}
      virtual void setMode(unsigned short mode = 1) {m_xtmode = mode;}
      virtual void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      virtual void CreateHisto();
      virtual void readProfile();
      virtual void readXT();
      virtual void Write();
      virtual void storeHisto();

      void execute()
      {
        calibrate();
      }

    protected:
      /// Run algo on data
      virtual bool calibrate();
      virtual void readXTFromDB();
      virtual void readXTFromText();
    private:
      double m_ndfmin = 5;
      double m_Pvalmin = 0.;

      bool m_debug = false;
      bool m_storeHisto = false;
      bool m_useDB = false;
      bool m_useProfileXTFromInputXT = true;
      bool m_LRseparate = true;
      bool m_useSliceFit = false;

      double m_XT_fit[56][2][18][7][8];
      double xtold[56][2][18][7][8];
      int fitflag[56][2][20][10];
      TF1* xtf5r[56][2][20][10];
      //TF1* xtf5rLeft_draw[56][20][10];
      //TF1* xtf5rLeft_sim[56][20][10];
      //TF1* xtf5r_sim[56][2][20][10];

      TProfile* hprof[56][2][20][10];
      TH2D* hist2d[56][2][20][10];
      TH2D* hist2d_draw[56][20][10];
      TH1D* hist2d_1[56][2][20][10];
      /*********************************
      Fit Flag
      =-1: low statitic
       =1: good
      =0: Fit failure
      =2: Error Outer
      =3: Error Inner part;
      **********************************/

      std::string m_OutputXTFileName = "xt.dat";
      std::string m_InputRootFileNames = "rootfile/output*";
      std::string m_ProfileFileName = "xt_profile";
      DBObjPtr<CDCXtRelations>* dbXT_old;
      std::string m_xtfile = "cdc/data/xt.dat";
      int m_firstExperiment; /**< First experiment. */
      int m_firstRun; /**< First run. */
      int m_lastExperiment; /**< Last experiment */
      int m_lastRun; /**< Last run. */

      //      double m_alpha[18][3];//alpha bin, 18bin, 0=low, 1 = up, 2 = alpha
      //double m_theta[10][3];//alpha bin, 18bin, 0=low, 1 = up, 2 = alpha
      //      unsigned short m_xtParamMode;    /*!< Mode for xt parameterization */
      //unsigned short m_nXtParams;      /*!< no. of xt parameters per bin */
      double l_alpha[18], u_alpha[18], ialpha[18];
      double l_theta[10], u_theta[10], itheta[10];
      double l_alpha_old[18], u_alpha_old[18], ialpha_old[18];
      double l_theta_old[10], u_theta_old[10], itheta_old[10];
      unsigned short xtmode_old;
      int m_nalpha, m_ntheta; /**<number of alpha, theta bins*/
      int nalpha_old, ntheta_old; /**<number of alpha, theta bins*/
      int m_MAXalpha = 18;
      int m_MAXtheta = 10;
      unsigned short m_xtmode = 1; /**< Mode of xt; 0 is polynomial;1 is Chebyshev.*/
      int m_smallestEntryRequire = 1000; /**< minimum number of hit per hitosgram. */
      /// boundary parameter for fitting
      double m_par6[56] = {89,   91,  94,  99,  104, 107, 110, 117,
                           126, 144, 150, 157, 170, 180,
                           160, 167, 183, 205, 200, 194,
                           177, 189, 192, 206, 224, 234,
                           193, 206, 209, 215, 222, 239,
                           204, 212, 217, 227, 235, 240,
                           215, 222, 230, 239, 246, 253,
                           227, 232, 239, 243, 253, 258,
                           231, 243, 246, 256, 263, 300
                          };

      ClassDef(XTCalibration, 0); /**< Test class implementing calibration algorithm */
    };
  }
}
