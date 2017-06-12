/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Roberto Stroili, Wenlong Yuan                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPLASERCALIBRATORMODULE_H
#define TOPLASERCALIBRATORMODULE_H

#include <framework/core/Module.h>
#include <string>

class TH1F;
class TF1;
class TTree;

namespace Belle2 {
  /**
   * T0 Laser calibration module
   * (under development)
   */
  class TOPLaserCalibratorModule : public Module {

  public:
    TOPLaserCalibratorModule();

    virtual ~TOPLaserCalibratorModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();

    virtual void terminate();

  private:

    /**
     * number of channels per module, storage windows per channel
     */
    enum {c_NumChannels = 512,
          c_maxLaserFibers = 9
         };

    std::string m_histogramFileName; /**< output file name for histograms */
    std::string m_simFileName; /**< input sim file name */
    int m_barID;                     /**< ID of TOP module to calibrate */
    //int m_fitFunc;       /**< fitting function flag: if true fit single gaussian */
    int m_fitPixelID; /** set 0 - 511 to a specific pixelID in the fit; set 512 to fit all pixels in one slot */
    std::string m_fitMethod; /** < gauss: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball */
    std::vector<double> m_fitRange; /**<fit range [nbins, xmin, xmax] */

    TH1F* m_histo[c_NumChannels]; /**< profile histograms */
    TTree* m_fittingParmTree; /**< tree with fitting parameters */
  };

} // Belle2 namespace

#endif
