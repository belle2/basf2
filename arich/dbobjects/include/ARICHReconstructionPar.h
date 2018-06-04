/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <TF1.h>

namespace Belle2 {
  //! The Class for ARICH reconstruction parameters
  /*! This class provides ARICH parameters for reconstruction (i.e. likelihood calculation)
   */

  class ARICHReconstructionPar : public TObject {

  public:

    //! Default constructor
    ARICHReconstructionPar() {};

    //! initializes "default" values of parameters
    void initializeDefault();

    /**
     * Set background PDF function for cherenkov angle distribution (vs. theta)
     * @param bkgPDF background PDF
     */
    void setBackgroundPDF(TF1* bkgPDF)
    {
      m_bkgPDF = bkgPDF;
    }

    /**
     * Set function for cherenkov angle resolution vs. momentum
     * @param thcRes cherenkov angle resolution vs. momentum function
     */
    void setThcResolution(TF1* thcRes)
    {
      m_thcResolution = thcRes;
    }

    /**
     * Set aerogel figure of merit for cherenkov photon yield
     * @param aerogelFOM vector of aerogel layers FOMs
     */
    void setAerogelFOM(const std::vector<float>& aerogelFOM)
    {
      m_aerogelFOM =  aerogelFOM;
    }

    /**
     * Set additional cherenkov angle PDF parameters
     * @param pars vector of parameters
     */
    void setParameters(const std::vector<float>& pars)
    {
      m_pars =  pars;
    }

    /**
    * Get vector of additional cherenkov angle PDF parameters
    * @return vector of parameters
    */
    const std::vector<float>& getParameters()
    {
      return m_pars;
    }

    /**
     * Get background PDF function
     * @return background PDF function
     */
    TF1* getBackgroundPDF()
    {
      return m_bkgPDF;
    }

    /**
    * Get Cherenkov angle resolution (without smearing due to pad size!) at given track momentum
    * @param momentum track momentum
    * @return cherenkov angle resolution
    */
    double getThcResolution(double momentum)
    {
      return  m_thcResolution->Eval(momentum);
    }

    /**
     * Get aerogel cherenkov photon yield for tile of selected layer
     * @param iLayer aerogel layer
     * @return aerogel cherenkov photon yield
     */
    double getAerogelFOM(unsigned iLayer)
    {
      return m_aerogelFOM.at(iLayer);
    }

    /**
    * Get expected number of background hits for pad at given theta
    * @param th_cer pad theta angle
    * @param pars vector of parameters for PDF (beta, track hits HAPD window (1 or 0))
    * @return expected number of backgrond hits on pad
    */
    double getBackgroundPerPad(double th_cer, std::vector<double>& pars);

    /**
     * Get number of expected background hits in ring (0.1<theta<0.5rad by default)
     * @param pars vector of parameters for PDF (beta, track hits HAPD window (1 or 0))
     * @param minThc inner theta angle of ring
     * @param maxThc outter theta angle of ring
     * @return expected number of backgrond hits in ring
     */
    double getExpectedBackgroundHits(std::vector<double>& pars, double minThc = 0.1, double maxThc = 0.5);

    /**
     * Get average number of pads in ring
     * @param maxThc outter theta angle of ring
     * @param minThc inner theta angle of ring
     * @param trackTh theta angle of track (global coordinates, z axis theta = 0)
     * @return average number of pads in ring
     */
    double getNPadsInRing(double maxThc, double minThc = 0.0, double trackTh = 0.45);

    /**
     * Print parameters values
     */
    void print() const;

  private:

    TF1* m_bkgPDF = NULL; /**< background PDF function (function of theta) */
    TF1* m_thcResolution = NULL; /**< cherenkov angle resolution (function of track momentum)*/
    std::vector<float> m_pars; /**< vector of other pdf parameters */
    std::vector<float> m_aerogelFOM; /**< aerogel figure of merit (for photon yield) */
    float m_flatBkgPerPad = 0.0; /**< expected background hits per pad (treated flat over detector surface) */

    ClassDef(ARICHReconstructionPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
