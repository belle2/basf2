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
    /**
     * Class to perform xt calibration for drift chamber.
     */
    class XTCalibration {
    public:
      /// Constructor
      XTCalibration();
      /// Destructor
      virtual ~XTCalibration() {}
      /// set to use BField
      virtual void BField(bool bfield) {m_BField = bfield;}
      /// Run in debug or silent
      virtual void setDebug(bool debug = false) {m_debug = debug; }
      /// Set to run with database mode or text mode
      virtual void setUseDB(bool useDB = false) {m_useDB = useDB; }
      /// set minimum number of degree of freedom requirement
      virtual void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// set minimum Prob(Chi2) requirement
      virtual void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// Input root file name, output of collector
      virtual void inputFileNames(std::string inputname) {m_inputRootFileNames.assign(inputname);}
      /// Profile file name incase you want to change the xt binning.
      virtual void profileFileNames(std::string profileFileName) {m_profileFileName.assign(profileFileName);}
      /// if you want to change xt bining, you have to set this to true
      virtual void useProfileFromInputXT(bool useProfileXTFromInputXT) {m_useProfileXTFromInputXT = useProfileXTFromInputXT;}
      /// input xt file name incase text mode is used.
      virtual void setXTFileName(std::string name) {m_xtfile.assign(name);}
      /// set xt mode, 0 is polynimial, 1 is Chebshev polynomial
      virtual void setMode(unsigned short mode = 1) {m_xtmode = mode;}
      /// set to store histogram or not.
      virtual void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}

      /// Run calibration
      void execute()
      {
        calibrate();
      }

    protected:
      /// Run algo on data
      virtual bool calibrate();
      /// Read old xt parameter from database
      virtual void readXTFromDB();
      /// Read old xt parameter from text file, incase text mode is used
      virtual void readXTFromText();
      /// Create histogram for calibration
      virtual void CreateHisto();
      /// Read profile xt file
      virtual void readProfile();
      /// read xt paramter (wrap text mode and database mode)
      virtual void readXT();
      /// Store calibrated constand
      virtual void Write();
      /// Store histogram to file
      virtual void storeHisto();

    private:
      double m_ndfmin = 5;    /**< minimum ndf required */
      double m_Pvalmin = 0.;  /**< minimum pvalue required */
      bool m_debug = false;   /**< run in debug or silent*/
      bool m_storeHisto = false;  /**< Store histogram or not*/
      bool m_useDB = false;       /**< Use Database or text mode*/
      bool m_useProfileXTFromInputXT = true; /**< use profile from text file or default in input xt*/
      bool m_LRseparate = true; /**< Separate LR in calibration or mix*/
      bool m_useSliceFit = false; /**< Use slice fit or profile */
      bool m_BField = true;  /**< with b field or none*/

      double m_XT_fit[56][2][18][7][8];  /**< Fitted parameter*/
      double xtold[56][2][18][7][8];     /**< Old paremeter */
      int fitflag[56][2][20][10];         /**< Fit flag */
      TF1* xtf5r[56][2][20][10];          /**< XTFunction */

      TProfile* hprof[56][2][20][10];     /**< Profile xt histo*/
      TH2D* hist2d[56][2][20][10];        /**< 2D histo of xt*/
      TH2D* hist2d_draw[56][20][10];       /**< 2d histo for draw*/
      TH1D* hist2d_1[56][2][20][10];       /**< 1D xt histo, results of slice fit*/
      /*********************************
      Fit Flag
      =-1: low statitic
       =1: good
      =0: Fit failure
      =2: Error Outer
      =3: Error Inner part;
      **********************************/

      std::string m_OutputXTFileName = "xt_new.dat"; /**< Out put xt filename*/
      std::string m_inputRootFileNames = "rootfile/output*"; /**< input root filename*/
      std::string m_profileFileName = "xt_profile";      /**<profile file name*/
      std::string m_xtfile = "cdc/data/xt.dat";          /**< Input xt file name, incase text mode*/

      int m_firstExperiment; /**< First experiment. */
      int m_firstRun; /**< First run. */
      int m_lastExperiment; /**< Last experiment */
      int m_lastRun; /**< Last run. */

      //      double m_alpha[18][3];//alpha bin, 18bin, 0=low, 1 = up, 2 = alpha
      //double m_theta[10][3];//alpha bin, 18bin, 0=low, 1 = up, 2 = alpha
      //      unsigned short m_xtParamMode;    /*!< Mode for xt parameterization */
      //unsigned short m_nXtParams;      /*!< no. of xt parameters per bin */
      int m_nalpha; /**<number of alpha bins*/
      int m_ntheta;/**<number of  theta bins*/
      double l_alpha[18];/**< Lower boundays of alpha bins. */
      double u_alpha[18];/**< Upper boundays of alpha bins. */
      double ialpha[18]; /**< represented alphas of alpha bins. */
      double l_theta[7]; /**< Lower boundays of theta bins. */
      double u_theta[7];/**< Upper boundays of theta bins. */
      double itheta[7]; /**< represented alphas of theta bins. */

      int nalpha_old; /**<number of alpha bins from input*/
      int ntheta_old;/**<number of  theta bins from input*/
      double l_alpha_old[18];/**< Lower boundays of alpha bins from input */
      double u_alpha_old[18];/**< Upper boundays of alpha bins from input */
      double ialpha_old[18]; /**< represented alphas of alpha bins from input. */
      double l_theta_old[7]; /**< Lower boundays of theta bins from input. */
      double u_theta_old[7];/**< Upper boundays of theta bins from input. */
      double itheta_old[7]; /**< represented alphas of theta bins from input. */

      unsigned short xtmode_old; /**< XT mode old, 0-polynomial, 1 Cheb*/
      int m_MAXalpha = 18;        /**< max alpha bin*/
      int m_MAXtheta = 7;         /**< max theta bin*/
      unsigned short m_xtmode = 1; /**< Mode of xt; 0 is polynomial;1 is Chebyshev.*/
      int m_smallestEntryRequire = 1000; /**< minimum number of hit per hitosgram. */
      /// boundary parameter for fitting, semi-experiment number
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

      ClassDef(XTCalibration, 0); /**< class implementing XT correction algorithm */
    };
  }
}
