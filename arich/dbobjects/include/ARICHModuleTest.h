/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <vector>
#include <TH1F.h>
#include <TH2D.h>
#include <TGraph.h>

#include <TClass.h>

namespace Belle2 {

  /**
   * ARICH module test data.
   */
  class ARICHModuleTest: public TObject {
  public:

    /**
     * Default constructor.
     */
    ARICHModuleTest(): m_febSN(0), m_hapdSN(""), m_hvbSN(0), m_run(0), m_runposition(0), m_isOK(false), m_deadCh(), m_strangeCh(),
      m_guardBias_th(NULL), m_HV_th(NULL), m_guardBias_2Dx(NULL), m_HV_2Dx(NULL), m_guardBias_2Dy(NULL), m_HV_2Dy(NULL), m_gain(NULL),
      m_charge(NULL), m_th(NULL), m_scanX(NULL), m_scanY(NULL), m_comment("")
    {
      for (unsigned i = 0; i < 4; i++) m_chipVdiff_th[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_chipLeak_th[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_chipVdiff_2Dx[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_chipLeak_2Dx[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_chipVdiff_2Dy[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_chipLeak_2Dy[i] = NULL;
    };

    /**
     * Destructor.
     */
    ~ARICHModuleTest() {};

    /**
     * Get FEB serial number.
     * @return FEB serial number.
     */
    int getFebSN() const {return m_febSN; }

    /**
     * Set FEB serial number.
     * @param[in] serial FEB serial number.
     */
    void setFebSN(int serial) {m_febSN = serial; }

    /**
     * Get HAPD serial number.
     * @return HAPD serial number.
     */
    std::string getHapdSN() const {return m_hapdSN; }

    /**
     * Set HAPD serial number.
     * @param[in] hapd HAPD serial number.
     */
    void setHapdSN(const std::string& hapd) {m_hapdSN = hapd; }

    /**
     * Get HVB serial number.
     * @return HVB serial number.
     */
    int getHvbSN() const {return m_hvbSN; }

    /**
     * Set HVB serial number.
     * @param[in] serial HVB serial number.
     */
    void setHvbSN(int serial) {m_hvbSN = serial; }

    /**
     * Get module test run.
     * @return Nodule test run.
     */
    int getRun() const {return m_run; }

    /**
     * Set module test run.
     * @param[in] run Module test run.
     */
    void setRun(int run) {m_run = run; }

    /**
     * Get module test-run position.
     * @return Module test-run position.
     */
    int getRunPosition() const {return m_runposition; }

    /**
     * Set module test-run position.
     * @param[in] runposition Module test-run position.
     */
    void setRunPosition(int runposition) {m_runposition = runposition; }

    /**
     * Return true if module is OK.
     * @return True if module is OK.
     */
    bool getOK() const {return m_isOK; }

    /**
     * Set whether module is OK.
     * @param[in] isOK Module is OK.
     */
    void setOK(bool isOK) {m_isOK = isOK; }

    /**
     * Get a channel number from the list of dead channels.
     * @param[in] i Index of the element in the list.
     * @return HAPD channel id.
     */
    int getDeadCh(unsigned int i) const;

    /**
     * Add a channel number to the list of dead channels.
     * @param[in] channel HAPD channel id.
     */
    void appendDeadCh(int channel) {m_deadCh.push_back(channel); }

    /**
     * Set vector of dead channel numbers.
     * @param[in] deadChs HAPD channel id.
     */
    void setDeadChs(const std::vector<int>& deadChs) {m_deadCh = deadChs; }

    /**
     * Get size of the list of dead channels.
     * @return Size.
     */
    int getDeadChsSize() const {return m_deadCh.size();}

    /**
     * Get a channel number from the list of strange channels.
     * @param[in] i Index of the element in the list.
     * @return HAPD channel id.
     */
    int getStrangeCh(unsigned int i) const;

    /**
     * Add a channel number to the list of strange channels.
     * @param[in] channel HAPD channel id.
     */
    void appendStrangeCh(int channel) {m_strangeCh.push_back(channel); }

    /**
     * Set vector of strange channel numbers.
     * @param[in] strangeChs HAPD channel id.
     */
    void setStrangeChs(const std::vector<int>& strangeChs) {m_strangeCh = strangeChs; }

    /**
     * Get size of the list of strange channels.
     * @return Size.
     */
    int getStrangeChsSize() const {return m_strangeCh.size();}

    /**
     * Get guard bias during threshold scan.
     * @return TGraph guard bias during threshold scan.
     */
    TGraph* getGuardBiasTH() const {return m_guardBias_th;}

    /**
     * Set guard bias during threshold scan.
     * @param[in] guardBias_th TGraph guard bias during threshold scan.
     */
    void setGuardBiasTH(TGraph* guardBias_th) { m_guardBias_th = guardBias_th;}

    /**
     * Get Chip Vmon - Vset difference during threshold scan.
     * @return Chip Vmon - Vset difference
     */
    TGraph* getChipVdiffTH(unsigned int i) const;

    /**
     * Set Chip Vmon - Vset difference during threshold scan.
     * @param[in] i chip id
     * @param[in] chipVdiff_th Chip Vmon - Vset difference.
     */
    void setChipVdiffTH(unsigned int i, TGraph* chipVdiff_th);

    /**
     * Get leakage current during threshold scan.
     * @return Leakage current.
     */
    TGraph* getChipLeakTH(unsigned int i) const;

    /**
     * Set leakage current during threshold scan.
     * @param[in] i chip id
     * @param[in] chipLeak_th Leakage current.
     */
    void setChipLeakTH(unsigned int i, TGraph* chipLeak_th);

    /**
     * Get high voltage during threshold scan.
     * @return TGraph high voltage during threshold scan.
     */
    TGraph* getHighVoltageTH() const {return m_HV_th;}

    /**
     * Set high voltage during threshold scan.
     * @param[in] HV_th TGraph high voltage during threshold scan.
     */
    void setHighVoltageTH(TGraph* HV_th) { m_HV_th = HV_th;}

    /**
     * Get guard bias during 2Dx scan.
     * @return TGraph guard bias during 2Dx scan.
     */
    TGraph* getGuardBias2Dx() const {return m_guardBias_2Dx;}

    /**
     * Set guard bias during 2Dx scan.
     * @param[in] guardBias_2Dx TGraph guard bias during 2Dx scan.
     */
    void setGuardBias2Dx(TGraph* guardBias_2Dx) { m_guardBias_2Dx = guardBias_2Dx;}

    /**
     * Get Chip Vmon - Vset difference during 2Dx scan.
     * @return Chip Vmon - Vset difference.
     */
    TGraph* getChipVdiff2Dx(unsigned int i) const;

    /**
     * Set Chip Vmon - Vset difference during 2Dx scan.
     * @param[in] i chip id
     * @param[in] chipVdiff_2Dx Chip Vmon - Vset difference.
     */
    void setChipVdiff2Dx(unsigned int i, TGraph* chipVdiff_2Dx);

    /**
     * Get leakage current during 2Dx scan.
     * @return Leakage current.
     */
    TGraph* getChipLeak2Dx(unsigned int i) const;

    /**
     * Set leakage current during 2Dx scan.
     * @param[in] i chip id
     * @param[in] chipLeak_2Dx Leakage current.
     */
    void setChipLeak2Dx(unsigned int i, TGraph* chipLeak_2Dx);

    /**
     * Get high voltage during 2Dx scan.
     * @return TGraph high voltage during 2Dx scan.
     */
    TGraph* getHighVoltage2Dx() const {return m_HV_2Dx;}

    /**
     * Set high voltage during 2Dx scan.
     * @param[in] HV_2Dx TGraph high voltage during 2Dx scan.
     */
    void setHighVoltage2Dx(TGraph* HV_2Dx) { m_HV_2Dx = HV_2Dx;}

    /**
     * Get guard bias during 2Dy scan.
     * @return TGraph guard bias during 2Dy scan.
     */
    TGraph* getGuardBias2Dy() const {return m_guardBias_2Dy;}

    /**
     * Set guard bias during 2Dy scan.
     * @param[in] guardBias_2Dy TGraph guard bias during 2Dy scan.
     */
    void setGuardBias2Dy(TGraph* guardBias_2Dy) { m_guardBias_2Dy = guardBias_2Dy;}

    /**
     * Get Chip Vmon - Vset difference during 2Dy scan.
     * @return Chip Vmon - Vset difference.
     */
    TGraph* getChipVdiff2Dy(unsigned int i) const;

    /**
     * Set Chip Vmon - Vset difference during 2Dx scan.
     * @param[in] i chip id
     * @param[in] chipVdiff_2Dy Chip Vmon - Vset difference.
     */
    void setChipVdiff2Dy(unsigned int i, TGraph* chipVdiff_2Dy);

    /**
     * Get leakage current during 2Dy scan.
     * @return Leakage current.
     */
    TGraph* getChipLeak2Dy(unsigned int i) const;

    /**
     * Set leakage current during 2Dy scan.
     * @param[in] i chip id
     * @param[in] chipLeak_2Dy Leakage current.
     */
    void setChipLeak2Dy(unsigned int i, TGraph* chipLeak_2Dy);

    /**
     * Get high voltage during 2Dy scan.
     * @return TGraph high voltage 2Dy scan.
     */
    TGraph* getHighVoltage2Dy() const {return m_HV_2Dy;}

    /**
     * Set high voltage during 2Dy scan.
     * @param[in] HV_2Dy TGraph high voltage 2Dy scan.
     */
    void setHighVoltage2Dy(TGraph* HV_2Dy) { m_HV_2Dy = HV_2Dy;}

    /**
     * Get gain.
     * @return Gain.
     */
    TH1F* getGain() const {return m_gain;}

    /**
     * Set gain.
     * @param[in] gain Gain.
     */
    void setGain(TH1F* gain) { m_gain = gain;}

    /**
     * Get charge scan.
     * @return Charge scan.
     */
    TH2D* getChargeScan() const {return m_charge;}

    /**
     * Set charge scan.
     * @param[in] charge Charge scan.
     */
    void setChargeScan(TH2D* charge) { m_charge = charge;}

    /**
     * Get threshold scan.
     * @return Threshold scan.
     */
    TH2D* getTresholdScan() const {return m_th;}

    /**
     * Set threshold scan.
     * @param[in] th Threshold scan.
     */
    void setTresholdScan(TH2D* th) { m_th = th;}

    /**
     * Get laser scan - X direction.
     * @return Laser scan - X direction.
     */
    TH2D* getLaserScanX() const {return m_scanX;}

    /**
     * Set laser scan - X direction.
     * @param[in] scanX Laser scan - X direction.
     */
    void setLaserScanX(TH2D* scanX) { m_scanX = scanX;}

    /**
     * Get laser scan - Y direction.
     * @return Laser scan - Y direction.
     */
    TH2D* getLaserScanY() const {return m_scanY;}

    /**
     * Set laser scan - Y direction.
     * @param[in] scanY Laser scan - Y direction.
     */
    void setLaserScanY(TH2D* scanY) { m_scanY = scanY;}

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
     */
    void setComment(const std::string& comment) {m_comment = comment; }

  private:

    int m_febSN;                   /**< FEB serial number */
    std::string m_hapdSN;          /**< HAPD serial number */
    int m_hvbSN;                   /**< HVB serial number */
    int m_run;                     /**< Run number */
    int m_runposition;             /**< Position on setup */
    bool m_isOK;                   /**< Module is OK if true */
    std::vector<int> m_deadCh;     /**< List of dead channels (mapping: channels on HAPD) */
    std::vector<int> m_strangeCh;  /**< List of strange channels (mapping: channels on HAPD) */

    TGraph* m_guardBias_th;        /**< Guard bias during threshold scan */
    TGraph* m_chipVdiff_th[4];     /**< Chip Vmon - Vset difference during threshold scan */
    TGraph* m_chipLeak_th[4];      /**< Chip leakage current during threshold scan */
    TGraph* m_HV_th;               /**< High voltage during threshold scan */
    TGraph* m_guardBias_2Dx;       /**< Guard bias during 2D x scan */
    TGraph* m_chipVdiff_2Dx[4];    /**< Chip Vmon - Vset difference during 2D x scan */
    TGraph* m_chipLeak_2Dx[4];     /**< Chip leakage current during 2D x scan */
    TGraph* m_HV_2Dx;              /**< High voltage during 2D x scan */
    TGraph* m_guardBias_2Dy;       /**< Guard bias during 2D y scan */
    TGraph* m_chipVdiff_2Dy[4];    /**< Chip Vmon - Vset difference during 2D y scan */
    TGraph* m_chipLeak_2Dy[4];     /**< Chip leakage current during 2D y scan */
    TGraph* m_HV_2Dy;              /**< High voltage during 2D y scan */
    TH1F* m_gain;                  /**< Gain for each channel */
    TH2D* m_charge;                /**< Charge scan 2D */
    TH2D* m_th;                    /**< Threshold scan 2D */
    TH2D* m_scanX;                 /**< Laser scan 2D - x direction */
    TH2D* m_scanY;                 /**< Laser scan 2D - y direction */

    std::string m_comment;         /**< Optional comment */

    ClassDef(ARICHModuleTest, 2);  /**< ClassDef */
  };
} // end namespace Belle2
