/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <TF1.h>
#include <TF2.h>

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
     * Set background PDF function with flattening phi correction
     * @param bkgPhiPDF background PDF f(phi, theta)
     */
    void setBackgroundPhiPDF(TF2* bkgPhiPDF)
    {
      m_bkgPhiPDF = bkgPhiPDF;
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
     * Set flat background per pad
     * @param flatBkgPerPad value of flat background per pad
     */
    void setFlatBkgPerPad(const float flatBkgPerPad)
    {
      m_flatBkgPerPad = flatBkgPerPad;
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
    const std::vector<float>& getParameters() const
    {
      return m_pars;
    }

    /**
     * Get background PDF function
     * @return background PDF function
     */
    const TF1* getBackgroundPDF() const
    {
      return m_bkgPDF;
    }

    /**
     * Get background PDF function (with phi correction)
     * @return background PDF function - flat with phi dependance
     */
    const TF2* getBackgroundPhiPDF() const
    {
      return m_bkgPhiPDF;
    }

    /**
    * Get Cherenkov angle resolution (without smearing due to pad size!) at given track momentum
    * @param momentum track momentum
    * @return cherenkov angle resolution
    */
    double getThcResolution(double momentum) const
    {
      return  m_thcResolution->Eval(momentum);
    }

    /**
     * Get aerogel cherenkov photon yield for tile of selected layer
     * @param iLayer aerogel layer
     * @return aerogel cherenkov photon yield
     */
    double getAerogelFOM(unsigned iLayer) const
    {
      return m_aerogelFOM.at(iLayer);
    }

    /**
     * Get flat background per pad
     * @return flat background per pad
     */
    float getFlatBkgPerPad() const
    {
      return m_flatBkgPerPad;
    }

    /**
    * Get expected number of background hits for pad at given theta
    * @param th_cer pad theta angle
    * @param pars vector of parameters for PDF (beta, track hits HAPD window (1 or 0))
    * @return expected number of backgrond hits on pad
    */
    double getBackgroundPerPad(double th_cer, const std::vector<double>& pars) const;

    /**
    * Get expected number of background hits for pad at given theta at given phi_Cer_trk
    * (flat background in Cherenkov s)pace
    * @param th_cer pad theta angle
    * @param th_cer pad theta angle
    * @param pars vector of parameters for PDF (beta, track hits HAPD window (1 or 0))
    * @return expected number of backgrond hits on pad
    */
    double getPhiCorrectedBackgroundPerPad(double fi_cer_trk, double th_cer, const std::vector<double>& pars) const;


    /**
     * Get number of expected background hits in ring (0.1<theta<0.5rad by default)
     * @param pars vector of parameters for PDF (beta, track hits HAPD window (1 or 0))
     * @param minThc inner theta angle of ring
     * @param maxThc outter theta angle of ring
     * @return expected number of backgrond hits in ring
     */
    double getExpectedBackgroundHits(const std::vector<double>& pars, double minThc = 0.1, double maxThc = 0.5) const;

    /**
     * Get average number of pads in ring
     * @param maxThc outter theta angle of ring
     * @param minThc inner theta angle of ring
     * @param trackTh theta angle of track (global coordinates, z axis theta = 0)
     * @return average number of pads in ring
     */
    double getNPadsInRing(double maxThc, double minThc = 0.0, double trackTh = 0.45) const;

    /**
     * Print parameters values
     */
    void print() const;

  private:

    mutable TF2* m_bkgPhiPDF = NULL; /**< background PDF function with phi correction */
    mutable TF1* m_bkgPDF = NULL; /**< background PDF function (function of theta) */
    TF1* m_thcResolution = NULL; /**< cherenkov angle resolution (function of track momentum)*/
    std::vector<float> m_pars; /**< vector of other pdf parameters */
    std::vector<float> m_aerogelFOM; /**< aerogel figure of merit (for photon yield) */
    float m_flatBkgPerPad = 0.0; /**< expected background hits per pad (treated flat over detector surface) */

    ClassDef(ARICHReconstructionPar, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace

