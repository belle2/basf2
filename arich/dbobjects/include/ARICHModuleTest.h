/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <vector>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for ARICH Aerogel Parameters.
   */

  class ARICHModuleTest: public TObject {
  public:

    /**
     * Default constructor
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
     * Destructor
     */
    ~ARICHModuleTest() {};

    /** Return FEB serial number
     * @return FEB serial number
     */
    int getFebSN() const {return m_febSN; }

    /** Set FEB serial number
     * @param FEB serial number
     */
    void setFebSN(int serial) {m_febSN = serial; }

    /** Return HAPD serial number
     * @return HAPD serial number
     */
    std::string getHapdSN() const {return m_hapdSN; }

    /** Set HAPD serial number
     * @param HAPD serial number
     */
    void setHapdSN(const std::string& hapd) {m_hapdSN = hapd; }

    /** Return HVB serial number
     * @return HVB serial number
     */
    int getHvbSN() const {return m_hvbSN; }

    /** Set HVB serial number
     * @param HVB serial number
     */
    void setHvbSN(int serial) {m_hvbSN = serial; }

    /** Return module test run
     * @return module test run
     */
    int getRun() const {return m_run; }

    /** Set module test run
     * @param module test run
     */
    void setRun(int run) {m_run = run; }

    /** Return module test run position
     * @return module test run position
     */
    int getRunPosition() const {return m_runposition; }

    /** Set module test run
     * @param module test run
     */
    void setRunPosition(int runposition) {m_runposition = runposition; }

    /** Return True if module is OK
     * @return True if module is OK
     */
    bool getOK() const {return m_isOK; }

    /** Set bool value - module is OK
     * @param module is OK
     */
    void setOK(bool isOK) {m_isOK = isOK; }

    /**
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return HAPD channel id
     */
    int getDeadCh(unsigned int i) const;

    /**
     * Add a channel number to the list of dead channels
     * @param HAPD channel id
     */
    void appendDeadCh(int channel) {m_deadCh.push_back(channel); }

    /**
     * Set vector of dead channel numbers
     * @param HAPD channel id
     */
    void setDeadChs(const std::vector<int>& deadChs) {m_deadCh = deadChs; }

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getDeadChsSize() const {return m_deadCh.size();}

    /**
     * Return a channel number from the list of strange channels
     * @param i index of the element in the list
     * @return HAPD channel id
     */
    int getStrangeCh(unsigned int i) const;

    /**
     * Add a channel number to the list of strange channels
     * @param HAPD channel id
     */
    void appendStrangeCh(int channel) {m_strangeCh.push_back(channel); }

    /**
     * Set vector of strange channel numbers
     * @param HAPD channel id
     */
    void setStrangeChs(const std::vector<int>& strangeChs) {m_strangeCh = strangeChs; }

    /**
     * Return size of the list of strange channels
     * @return size
     */
    int getStrangeChsSize() const {return m_strangeCh.size();}

    /**
     * Return guard bias during treshold scan
     * @return TGraph guard bias during treshold scan
     */
    TGraph* getGuardBiasTH() const {return m_guardBias_th;}

    /**
     * Set guard bias during treshold scan
     * @param TGraph guard bias during treshold scan
     */
    void setGuardBiasTH(TGraph* guardBias_th) { m_guardBias_th = guardBias_th;}

    /** Get Chip Vmon - Vset difference during treshold scan
     * @return Chip Vmon - Vset difference
     */
    TGraph* getChipVdiffTH(unsigned int i) const;

    /** Set Chip Vmon - Vset difference during treshold scan
     * @param Chip Vmon - Vset difference
     */
    void setChipVdiffTH(unsigned int i, TGraph* chipVdiff_th);

    /** Get leakage current during treshold scan
     * @return leakage current
     */
    TGraph* getChipLeakTH(unsigned int i) const;

    /** Set leakage current during treshold scan
     * @param leakage current
     */
    void setChipLeakTH(unsigned int i, TGraph* chipLeak_th);

    /**
     * Return high voltage during treshold scan
     * @return TGraph high voltage during treshold scan
     */
    TGraph* getHighVoltageTH() const {return m_HV_th;}

    /**
     * Set high voltage during treshold scan
     * @param TGraph high voltage during treshold scan
     */
    void setHighVoltageTH(TGraph* HV_th) { m_HV_th = HV_th;}

    /**
     * Return guard bias during 2Dx scan
     * @return TGraph guard bias during 2Dx scan
     */
    TGraph* getGuardBias2Dx() const {return m_guardBias_2Dx;}

    /**
     * Set guard bias during 2Dx scan
     * @param TGraph guard bias during 2Dx scan
     */
    void setGuardBias2Dx(TGraph* guardBias_2Dx) { m_guardBias_2Dx = guardBias_2Dx;}

    /** Get Chip Vmon - Vset difference during 2Dx scan
     * @return Chip Vmon - Vset difference
     */
    TGraph* getChipVdiff2Dx(unsigned int i) const;

    /** Set Chip Vmon - Vset difference during 2Dx scan
     * @param Chip Vmon - Vset difference
     */
    void setChipVdiff2Dx(unsigned int i, TGraph* chipVdiff_2Dx);

    /** Get leakage current during 2Dx scan
     * @return leakage current
     */
    TGraph* getChipLeak2Dx(unsigned int i) const;

    /** Set leakage current during 2Dx scan
     * @param leakage current
     */
    void setChipLeak2Dx(unsigned int i, TGraph* chipLeak_2Dx);

    /**
     * Return high voltage during 2Dx scan
     * @return TGraph high voltage during 2Dx scan
     */
    TGraph* getHighVoltage2Dx() const {return m_HV_2Dx;}

    /**
     * Set high voltage during 2Dx scan
     * @param TGraph high voltage during 2Dx scan
     */
    void setHighVoltage2Dx(TGraph* HV_2Dx) { m_HV_2Dx = HV_2Dx;}

    /**
     * Return guard bias during 2Dy scan
     * @return TGraph guard bias during 2Dy scan
     */
    TGraph* getGuardBias2Dy() const {return m_guardBias_2Dy;}

    /** Get Chip Vmon - Vset difference during 2Dy scan
     * @return Chip Vmon - Vset difference
     */
    TGraph* getChipVdiff2Dy(unsigned int i) const;

    /** Set Chip Vmon - Vset difference during 2Dx scan
     * @param Chip Vmon - Vset difference
     */
    void setChipVdiff2Dy(unsigned int i, TGraph* chipVdiff_2Dy);

    /**
     * Set guard bias during 2Dy scan
     * @param TGraph guard bias during 2Dy scan
     */
    void setGuardBias2Dy(TGraph* guardBias_2Dy) { m_guardBias_2Dy = guardBias_2Dy;}

    /** Get leakage current during 2Dy scan
     * @return leakage current
     */
    TGraph* getChipLeak2Dy(unsigned int i) const;

    /** Set leakage current during 2Dy scan
     * @param leakage current
     */
    void setChipLeak2Dy(unsigned int i, TGraph* chipLeak_2Dy);

    /**
     * Return high voltage during 2Dy scan
     * @return TGraph high voltage 2Dy scan
     */
    TGraph* getHighVoltage2Dy() const {return m_HV_2Dy;}

    /**
     * Set high voltage during 2Dy scan
     * @param TGraph high voltage 2Dy scan
     */
    void setHighVoltage2Dy(TGraph* HV_2Dy) { m_HV_2Dy = HV_2Dy;}

    /**
     * Return gain
     * @return gain
     */
    TH1F* getGain() const {return m_gain;}

    /**
     * Set gain
     * @param gain
     */
    void setGain(TH1F* gain) { m_gain = gain;}

    /**
     * Return charge scan
     * @return charge scan
     */
    TH2D* getChargeScan() const {return m_charge;}

    /**
     * Set charge scan
     * @param charge scan
     */
    void setChargeScan(TH2D* charge) { m_charge = charge;}

    /**
     * Return treshold scan
     * @return treshold scan
     */
    TH2D* getTresholdScan() const {return m_th;}

    /**
     * Set treshold scan
     * @param treshold scan
     */
    void setTresholdScan(TH2D* th) { m_th = th;}

    /**
     * Return laser scan - X direction
     * @return laser scan - X direction
     */
    TH2D* getLaserScanX() const {return m_scanX;}

    /**
     * Set laser scan - X direction
     * @param laser scan - X direction
     */
    void setLaserScanX(TH2D* scanX) { m_scanX = scanX;}

    /**
     * Return laser scan - Y direction
     * @return laser scan - Y direction
     */
    TH2D* getLaserScanY() const {return m_scanY;}

    /**
     * Set laser scan - Y direction
     * @param laser scan - Y direction
     */
    void setLaserScanY(TH2D* scanY) { m_scanY = scanY;}

    /** Set comment
     * @param comment
     */
    void setComment(const std::string& comment) {m_comment = comment; }

    /** Return comment
     * @return comment
     */
    std::string getComment() const {return m_comment; }

  private:

    int m_febSN;                   /**< FEB serial number */
    std::string m_hapdSN;          /**< HAPD serial number */
    int m_hvbSN;                   /**< HVB serial number */
    int m_run;                     /**< Run number */
    int m_runposition;             /**< Position on setup */
    bool m_isOK;                   /**< Module is OK if true */
    std::vector<int> m_deadCh;     /**< List of dead channels (mapping: channels on HAPD) */
    std::vector<int> m_strangeCh;  /**< List of strange channels (mapping: channels on HAPD) */

    TGraph* m_guardBias_th;        /**< Guard bias during treshold scan */
    TGraph* m_chipVdiff_th[4];     /**< Chip Vmon - Vset difference during treshold scan */
    TGraph* m_chipLeak_th[4];      /**< Chip leakage current during treshold scan */
    TGraph* m_HV_th;               /**< High voltage during treshold scan */
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
    TH2D* m_th;                    /**< Treshold scan 2D */
    TH2D* m_scanX;                 /**< Laser scan 2D - x direction */
    TH2D* m_scanY;                 /**< Laser scan 2D - y direction */

    std::string m_comment;         /**< Optional comment */

    ClassDef(ARICHModuleTest, 2);  /**< ClassDef */
  };
} // end namespace Belle2
