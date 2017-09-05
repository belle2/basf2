#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TSystem.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <cdc/dbobjects/CDCSpaceResols.h>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {
  namespace CDC {

    /**
     * Class for Space resolution calibration.
     */
    class SpaceResolutionCalibration {
      //      typedef std::array<float, 3> array3; /**< angle bin info. */
    public:
      /// Constructor
      SpaceResolutionCalibration();
      /// Destructor
      virtual ~SpaceResolutionCalibration() {}
      /// Debug or not
      virtual void setDebug(bool debug = false) {m_debug = debug; }
      /// Use database or text mode
      virtual void setUseDB(bool useDB = false) {m_useDB = useDB; }
      /// minimum NDF required for track
      virtual void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// Minimum Pval required
      virtual void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// Bin width of each slide
      virtual void setBinWidth(double bw) {m_binWidth = bw;}
      /// Work with B field or not;
      virtual void BField(bool bfield) {m_BField = bfield;}
      /// Input root file names, results of collector module
      virtual void inputFileNames(std::string inputname)
      {
        m_inputRootFileNames.assign(inputname);
      }
      /// Store histograms durring the calibration or not
      virtual void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      /// File name describe theta/alpha bin, if don't want to use default from input sigma
      virtual void ProfileFileNames(std::string profileFileName)
      {
        m_ProfileFileName.assign(profileFileName);
      }
      /// use sigma bin profile form input sigma or new one from input file
      virtual void useProfileFromInputSigma(bool useProfileFromInputSigma)
      {
        m_useProfileFromInputSigma = useProfileFromInputSigma;
      }
      /// Output sigma file name, for text mode
      virtual void setSigmaFileName(std::string name) {m_sigmafile.assign(name);}
      //      virtual void setMode(unsigned short mode=1){m_SigmaParamMode = mode;}
      /// execute all, make the interface the same as CAF
      void execute()
      {
        calibrate();
      }

    protected:
      /// Run algo on data
      virtual bool calibrate();
      /// create histogram
      virtual void createHisto();
      /// read sigma bining (alpha, theta bining)
      virtual void readProfile();
      /// read sigma from previous calibration, (input sigma)
      virtual void readSigma();
      /// read sigma from DB
      virtual void readSigmaFromDB();
      /// read sigma from text file
      virtual void readSigmaFromText();
      /// store histogram
      virtual void storeHisto();
      /// save calibration, in text file or db
      virtual void write();

    private:
      static const int Max_nalpha = 18; /**< Maximum alpha bin.*/
      static const int Max_ntheta = 7; /**< maximum theta bin  */
      static const unsigned short Max_np = 40; /**< Maximum number of point =1/binwidth */

      double m_ndfmin = 5; /**< Minimum NDF  */
      double m_Pvalmin = 0.; /**<  Minimum Prob(chi2) of track*/
      double m_binWidth = 0.05; /**<width of each bin, unit cm*/
      bool m_debug = false;   /**< Debug or not */
      bool m_draw = false;    /**< print out histogram in pdf file or not*/
      bool m_storeHisto = false; /**<  Store histogram or not*/
      bool m_useDB = false;     /**<  use db or text mode*/
      bool m_useProfileFromInputSigma = true; /**<  Use binning from old sigma or new one form input*/
      bool m_BField = true;                   /**< Work with BField, fit range and initial parameters is different incase B and noB */
      //      bool m_LRseparate = true;
      double sigma_old[56][2][18][7][8]; /**<old sigma prameters.*/
      double sigma_new[56][2][18][7][8]; /**<new sigma prameters.*/
      TF1* ffit[56][2][18][7];           /**< fitting function*/
      TGraphErrors* gfit[56][2][18][7];  /**< sigma*sigma graph for fit*/
      TGraphErrors* gr[56][2][18][7];    /**< sigma graph.*/
      TH2F* hist_b[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of biased residual */
      TH2F* hist_u[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of unbiased residual */
      TH1F* hu_m[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram biased residual*/
      TH1F* hu_s[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of biased residual*/
      TH1F* hb_m[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram of unbiased residual*/
      TH1F* hb_s[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of ubiased residual*/
      int m_fitflag[56][2][Max_nalpha][Max_ntheta] = {{{{0}}}} ; /**< Fit flag; 1:OK ; 0:error*/

      std::string m_outputSigmaFileName = "sigma_new.dat"; /**< Output sigma file name */
      std::string m_inputRootFileNames = "rootfile/output*"; /**<  Input root file names*/
      std::string m_ProfileFileName = "sigma_profile"; /**<  Profile file name*/
      DBObjPtr<CDCSpaceResols>* m_sResolFromDB ;         /**<  Database for sigma*/
      std::string m_sigmafile = "cdc/data/sigma.dat";    /**<  Sigma file name, for text mode*/
      int m_firstExperiment; /**< First experiment. */
      int m_firstRun; /**< First run. */
      int m_lastExperiment; /**< Last experiment */
      int m_lastRun; /**< Last run. */

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

      unsigned short m_sigmaParamMode_old; /**< sigma mode from input. */
      unsigned short m_sigmaParamMode = 1; /**< sigma mode for this calibration.*/

      ClassDef(SpaceResolutionCalibration, 0); /**< Class for calibraion CDC space resolution */
    };
  }
}
