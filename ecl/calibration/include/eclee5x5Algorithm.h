/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclee5x5Algorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <ecl/geometry/ECLNeighbours.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using Bhabha events */
    class eclee5x5Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclee5x5Algorithm();

      /**..Destructor */
      virtual ~eclee5x5Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_minEntries */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /** Getter for m_minEntries */
      int getMinEntries() {return m_minEntries;}

      /** Setter for m_payloadName */
      void setPayloadName(const std::string& payloadname) {m_payloadName = payloadname;}

      /** Getter for m_payloadname */
      std::string getPayloadName() {return m_payloadName;}

      /** Setter for m_storeConst */
      void setStoreConst(bool storeConst) {m_storeConst = storeConst;}

      /** Getter for m_storeConst */
      bool getStoreConst() {return m_storeConst;}

      /** Setter for m_fracLo */
      void setfracLo(double fracLo) {m_fracLo = fracLo;}

      /** Getter for m_fracLo */
      double getfracLo() {return m_fracLo;}

      /** Setter for m_fracHiSym */
      void setfracHiSym(double fracHiSym) {m_fracHiSym = fracHiSym;}

      /** Getter for m_fracHiSym */
      double getfracHiSym() {return m_fracHiSym;}

      /** Setter for m_fracHiASym */
      void setfracHiASym(double fracHiASym) {m_fracHiASym = fracHiASym;}

      /** Getter for m_fracHiASym */
      double getfracHiASym() {return m_fracHiASym;}

      /** Setter for m_nsigLo */
      void setnsigLo(double nsigLo) {m_nsigLo = nsigLo;}

      /** Getter for m_nsigLo */
      double getnsigLo() {return m_nsigLo;}

      /** Setter for m_nsigHiSym */
      void setnsigHiSym(double nsigHiSym) {m_nsigHiSym = nsigHiSym;}

      /** Getter for m_nsigHiSym */
      double getnsigHiSym() {return m_nsigHiSym;}

      /** Setter for m_nsigHiASym */
      void setnsigHiASym(double nsigHiASym) {m_nsigHiASym = nsigHiASym;}

      /** Getter for m_nsigHiASym */
      double getnsigHiASym() {return m_nsigHiASym;}

      /** Setter for m_lastLoThetaID */
      void setlastLoThetaID(int lastLoThetaID) {m_lastLoThetaID = lastLoThetaID;}

      /** Getter for m_lastLoThetaID */
      int getlastLoThetaID() {return m_lastLoThetaID;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      /**..Parameters to control job to find energy calibration using Bhabhas */
      std::string m_outputName = "eclee5x5Algorithm.root"; /**< file name for histogram output */
      int m_minEntries = 150;  /**< all crystals to be calibrated must have this many entries */
      /** Name of the payload to be stored. options: ECLCrystalEnergy5x5, ECLExpee5x5E, ECLeedPhiData, ECLeedPhiMC, or None */
      std::string m_payloadName = "ECLCrystalEnergy5x5";
      bool m_storeConst = true; /**< write payload to localdb if true */
      ECL::ECLNeighbours* m_eclNeighbours5x5{nullptr}; /**< Neighbours, used to get nCrys per ring*/
      double m_fracLo = 0.2; /**< start dPhi fit where data is > fraclo*peak */
      double m_fracHiSym = 0.2; /**< end dPhi fit where data is > fracHiSym*peak */
      double m_fracHiASym = 0.4; /**< or fracHiASym*peak, at low values of thetaID */
      double m_nsigLo = 2.5; /**< dPhi region is mean - nsigLo*sigma */
      double m_nsigHiSym = 2.5; /**< to mean + nsigHiSym*sigma */
      double m_nsigHiASym = 2.0; /**< or mean+nsigHiASym*sigma at low thetaID */
      int m_lastLoThetaID = 4; /**< use asymmetric dPhi range for thetaID<= this value */
    };
  }
} // namespace Belle2


