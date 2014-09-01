/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBEAMALIGNMENTMODULE_H
#define TOPBEAMALIGNMENTMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <string>
#include <sstream>


#include "TFile.h"
#include "TH2.h"


namespace Belle2 {

  /**
   * Scan region, provides some functionality for the module
   */
  struct Scan {
    int nPoints;  /**< number of scan points */
    double xmin;  /**< xmin for histogram booking */
    double xmax;  /**< xmax for histogram booking */
    double step;  /**< scan step */
    double a;     /**< robustness parameter */
    std::string name; /**< name of the region */

    /**
     * Default constructor
     */
    Scan():
      nPoints(0), xmin(0.0), xmax(0.0), step(0.0), a(0.0)
    {}

    /**
     * Constructor from steering parameters
     * @param scan steering parameters
     * @param Name name (optional)
     */
    Scan(std::vector<double>& scan, std::string Name = std::string("")): name(Name) {
      while (scan.size() < 4) scan.push_back(0.0);
      int np = int(scan[2]);
      if (np < 0) B2ERROR(name << ": np must be non-negative");
      nPoints = 2 * np + 1;
      step = scan[1];
      if (step <= 0) B2ERROR(name << ": step must be positive");
      double halfsize = step * (np + 0.5);
      xmin = scan[0] - halfsize;
      xmax = scan[0] + halfsize;
      a = scan[3];
    }

    /**
     * Constructor from number of points and region limits
     * @param Nx number of points
     * @param Xmin lower limit
     * @param Xmax upper limit
     * @param np region extension to left and right
     */
    Scan(int Nx, double Xmin, double Xmax, int np = 0) {
      nPoints = Nx + 2 * np;
      step = (Xmax - Xmin) / Nx;
      xmin = Xmin - step * np;
      xmax = Xmax + step * np;
      a = 0;
    }

    /**
     * Constructor from half number of points and step size
     * @param np half number of points
     * @param Step step size
     */
    Scan(int np, double Step) {
      nPoints = 2 * np + 1;
      step = Step;
      double halfsize = step * (np + 0.5);
      xmin = - halfsize;
      xmax =   halfsize;
      a = 0;
    }

    /**
     * Return i-th point
     * @param i 0-based index
     * @return point position
     */
    double getPoint(int i) const {
      return xmin + step * (i + 0.5);
    }

    /**
     * Return center of scan region
     * @return center
     */
    double getCenter() const {
      return (xmin + xmax) / 2;
    }

  };


  /**
   * Performs beam alignment
   */
  class TOPbeamAlignmentModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPbeamAlignmentModule();

    /**
     * Destructor
     */
    virtual ~TOPbeamAlignmentModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor: histograms ring image
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action: performs the alignment
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    /**
     * Returns row-wise channel ID (e.g. lower row of channels first)
     * @param ich 0-based channel index (e.g. TOPDigit::m_channelID - 1)
     * @return row-wise channel ID
     */
    int rowWiseChannelID(int ich) {
      /*
      int Npad = m_numPADrows * m_numPADcols;
      int pmt = ich / Npad;
      int pad = ich % Npad;
      int pmtRow = pmt / m_numPMTcols;
      int pmtCol = pmt % m_numPMTcols;
      int padRow = pad / m_numPADcols;
      int padCol = pad % m_numPADcols;
      int col = padCol + pmtCol * m_numPADcols;
      int row = padRow + pmtRow * m_numPADrows;
      return col + row * m_numPMTcols * m_numPADcols;
      */
      return ich; // new numbering scheme is exactly this one!
    }


    /**
     * converts a number to printable string
     * @param number
     * @return string
     */
    std::string numberToString(int number) {
      std::stringstream ss;
      std::string str;
      ss << number;
      ss >> str;
      return str;
    }


    /**
     * converts a number to printable string
     * @param number
     * @return string
     */
    std::string numberToString(double number) {
      std::stringstream ss;
      std::string str;
      ss << number;
      ss >> str;
      return str;
    }


    /**
     * Performs beam alignment using likelihood scan. Called in terminate().
     */
    void beamAlignment();


    /**
     * fills PDF into 2D histogram
     * @param reco reconstruction object
     * @param histogram 2D histogram
     */
    void fillPDF(TOP::TOPreco& reco, TH2F* histogram);


    /**
     * performs likelihood maximization using the scan
     * @param reco reconstruction object
     */
    void likelihoodScan(TOP::TOPreco& reco);


    /**
     * finds maximum likelihood point in 2D
     * @param reco reconstruction object
     * @param scanX region to scan in x
     * @param scanY region to scan in y
     * @param X position of the maximum in x (return value)
     * @param Y position of the maximum in y (return value)
     * @param iter iteration
     */
    void findMaximumLikelihood(TOP::TOPreco& reco, const Scan& scanX, const Scan& scanY,
                               double& X, double& Y, int iter);

    /**
     * Returns maximum log likelihood after t0 scan
     * @param reco reconstruction object
     * @return log likelihood
     */
    double getLogLikelihood(TOP::TOPreco& reco);


    /**
     * Returns log likelihood
     * @param reco reconstruction object
     * @param t0 offset time
     * @return log likelihood
     */
    double getLogLikelihood(TOP::TOPreco& reco, double t0);

    /**
     * Return parabolic maximum
     * @param yLeft bin content of left-to-maximal bin
     * @param yCenter bin content of maximal bin
     * @param yRight bin content of right-to-maximal bin
     * @return a fraction of step to be added to central bin position
     */
    double getParabolicMaximum(double yLeft, double yCenter, double yRight);

    // steering parameters

    std::string m_outputFileName;    /**< output file name */
    std::string m_beamParticle;      /**< beam particle: one of e, mu, pi, K, p */
    double m_beamMomentum;           /**< beam momentum */
    std::vector<double> m_scanY;     /**< scan in y: central, step, np, a */
    std::vector<double> m_scanZ;     /**< scan in z: central, step, np, a */
    std::vector<double> m_scanTheta; /**< scan in theta: central, step, np, a */
    std::vector<double> m_scanPhi;   /**< scan in phi: central, step, np, a */
    int m_scanT0;                    /**< scan in t0: np */
    double m_tMin;                   /**< minimal TDC time */
    double m_tMax;                   /**< maximal TDC time */
    int m_numBins;                   /**< number of bins for TDC time */
    int m_numIterations;             /**< number of iterations */
    bool m_parabolicMaximum;         /**< use parabolic maximum */
    double m_minBkgPerBar;           /**< minimal assumed background photons per bar */
    double m_electronicJitter;       /**< r.m.s of electronic jitter */

    // others

    double m_mass;   /**< beam particle mass */
    double m_x0;     /**< beam position in x */
    double m_y0;     /**< beam position in y */
    double m_z0;     /**< beam position in z */
    double m_theta;  /**< beam polar angle */
    double m_phi;    /**< beam azimuthal angle */
    double m_t0;     /**< offset time to add to photon TDC times */
    int m_numChannels; /**< number of channels */
    int m_numPMTrows;  /**< number of rows of PMT's */
    int m_numPMTcols;  /**< number of columns of PMT's */
    int m_numPADrows;  /**< number of rows of PMT pads */
    int m_numPADcols;  /**< number of columns of PMT pads */
    int m_numEvents;   /**< number of events */
    std::vector<int> m_rowWiseChannelID; /**< converter to row wise channel ID */

    TOP::TOPGeometryPar* m_topgp;   /**< geometry parameters */

    // root file and histograms

    TFile* m_file;       /**< TFile */
    TH2F* m_ringImage;   /**< 2D histogram of a ring image */
    TH2F* m_pdfInitial;  /**< 2D histogram of the corresponding PDF before alignment */
    TH2F* m_pdfAligned;  /**< 2D histogram of the corresponding PDF after alignment */
    std::vector<TH2F*> m_scanHistograms;  /**< scan histograms for control */


  };

} // Belle2 namespace

#endif
