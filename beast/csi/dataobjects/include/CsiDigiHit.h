/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CSIDIGIT_H
#define CSIDIGIT_H

#include <framework/datastore/RelationsObject.h>

#include <vector> //< For fixed-size integers
namespace Belle2 {

  /*! Class to store Csi digitized hits (output of CsIDigitizer)
   * relation to CsiHit
   * filled in beast/csi/modules/src/CsIDigitizerModule.cc
   */

  class CsiDigiHit : public RelationsObject {
  public:
    /** default constructor for ROOT */
    CsiDigiHit()
    {
      m_CellId   = 0;
      m_Charge   = 0;
      m_Baseline = 0;
      m_MaxVal   = 0;
      m_Time     = 0;
      m_TrueEdep = 0;
      m_PDG = 0;
      m_Waveform.clear();
      m_StatusBits.clear();
    }

    /*! Set  Cell ID
     */
    void setCellId(uint8_t CellId) { m_CellId = CellId; }

    /*! Set Integrated Charge
     */
    void setCharge(uint64_t charge) { m_Charge = charge; }


    /*! Set Trigger Time
     */
    void setTime(uint64_t time) { m_Time = time; }




    /*! Get Cell ID
     * @return cell ID
     */
    uint8_t getCellId() const { return m_CellId; }


    /*! Get Integrated Charge
     * @return Integrated Charge [ADC*sample]
     */
    uint32_t getCharge() const { return m_Charge; }

    /*! Get Trigger Time
     * @return Trigger Time
     */
    uint32_t getTime() const { return m_Time; }

    /*! Get true deposited energy
     * @return deposited energy in the event in GeV
     */
    double getTrueEdep() const { return m_TrueEdep; }

    /*! Set True Edep
     */
    void setTrueEdep(double edep) { m_TrueEdep = edep; }


    /*! Get Baseline
     * @return Baseline
     */
    uint16_t getBaseline() const { return m_Baseline; }

    /*! Set Baseline
     */
    void setBaseline(uint16_t baseline) { m_Baseline = baseline; }

    /*! Get Maximal Value
     */
    uint16_t getMaxVal() const { return m_MaxVal; }

    /*! Set Maximal Value
     */
    void setMaxVal(uint16_t maxval) { m_MaxVal = maxval; }


    /*! Get Waveform array
     * @return A pointer to the std::vector containing the waveform
     */
    std::vector<uint16_t>* getWaveform() { return &m_Waveform; }

    /*! Set Waveform Array
     */
    void setWaveform(std::vector<uint16_t>* waveform) { m_Waveform = *waveform; }

    /*! Get Status bits array
     * @return A pointer to the std::vector containing the status
     */
    std::vector<uint8_t>* getStatusBits() { return &m_StatusBits; }

    /*! Set Status Bits Array
     */
    void setStatusBits(std::vector<uint8_t>* status) { m_StatusBits = *status; }


    /*! Set incoming particle PDG code
     */
    void setPDG(int pdg) {m_PDG = pdg;}

  private:

    uint8_t   m_CellId;                /**< Cell ID */
    uint16_t  m_Baseline;              /**< Baseline (pedestal) frozen during charge integration */
    uint16_t  m_MaxVal;                /**< Maximal value */
    uint32_t  m_Charge;                /**< Integrated Charge */
    uint32_t  m_Time;                  /**< Trigger Time index*/
    double    m_TrueEdep;              /**< True deposited energy per event-crystal */
    int       m_PDG;                   /**< PDG code of the initial particle */
    std::vector<uint16_t>  m_Waveform; /**< Saved waveform*/
    std::vector<uint8_t>   m_StatusBits; /**< Saved waveform*/


    ClassDef(CsiDigiHit, 1);/**< ClassDef */

  };
} // end namespace Belle2

#endif
