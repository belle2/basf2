/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHIT_H
#define CDCHIT_H

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>


#include <TObject.h>

namespace Belle2 {

  /** Class for saving raw hit data of the CDC.
   *
   *  This class is optimized for low disc usage. For reconstruction purposes use the corresponding
   *  CDCRecoHit class. <br>
   *
   *  It stores <br>
   *  Drift Time***, the accumulated charge in the hit cell,
   *  the Super Layer of which the hit wire is part,
   *  the Layer within the Super Layer,
   *  and the wire number within the Layer.
   *
   *  ***Currently DriftLength
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=DataStore">Martin Heck</a>
   *  @todo At some point indeed drift time instead of drift length should be saved here.
   *  @todo Perhaps the class has to be modified to be able to prevent streaming of the TObject elements.
   */
  class CDCHit : public TObject {
  public:

    /** Empty constructor for ROOT IO. */
    CDCHit() {
      B2DEBUG(150, "Empty CDCHit Constructor called.");
    }

    /** Useful Constructor.
     *
     *  Currently the setters are called for the actual assignation, for reducing the number of places where to encode
     *  the transformation into the internal encoding.
     *
     *  @param driftTime   currently assumes you give a drift LENGTH.
     *  @param charge      accumulated charge in the drift cell for this hit.
     *  @param iSuperLayer Super Layer of the wire.
     *  @param iLayer      Layer number inside the Super Layer.
     *  @param iWire       Wire number in the Layer.
     */
    CDCHit(const double& driftTime, const double& charge,
           const int& iSuperLayer, const int& iLayer, int iWire);

    /** Setter for Wire ID.
     *
     *  @param iSuperLayer Values should be between [0,   8].
     *  @param iLayer      Values should be between [0,   7], depending on the SuperLayer.
     *  @param iWire       Values should be between [0, 511], depending on the wire radius.
     */
    void setWireId(const int& iSuperLayer, const int& iLayer, const int& iWire) {
      B2DEBUG(150, "setWireId called with" << iSuperLayer << ", " << iLayer << ", " << iWire);
      m_wireId = static_cast<unsigned short int>(iWire + 512 * iLayer + 4096 * iSuperLayer);
    }

    /** Setter for Drift Time.
     *
     *  CAUTION!
     *  Currently Drift Length is stored.
     *
     *  @param driftTime Drift Length of electrons from closest ionisation cluster.
     */
    void setDriftTime(double driftTime) {
      B2DEBUG(150, "setDriftTime called with " << driftTime);
      m_driftTime = static_cast<unsigned short int>(driftTime / Unit::um);
    }

    /** Setter for charge.
     *
     *  The current value is just a reasonable value from looking into the simulation.
     *  No special theoretical evaluation.
     */
    void setCharge(double charge) {
      B2DEBUG(150, "setCharge called with " << charge);
      m_charge = static_cast<unsigned short int>(charge * 1e7);
    }

    /** Getter for iWire. */
    int getIWire() const {
      return (m_wireId % 512);
    }

    /** Getter for iLayer. */
    int getILayer() const {
      return ((m_wireId % 4096) / 512);
    }

    /** Getter for iSuperLayer. */
    int getISuperLayer() const {
      return (m_wireId / 4096);
    }

    /** Getter for Drift Time.
     *
     *  CAUTION! <br>
     *  Currently a drift length is given back, not a drift time!
     */
    double getDriftTime() const {
      return (static_cast<double>(m_driftTime) * Unit::um);
    }

    /** Getter for integrated charge.
     *
     *  Note: This is the integrated charge over the cell.
     *        In principle, this charge can come from more than just the
     *        track, this hit belongs to.
     */
    double getCharge() const {
      return (1e-7 * static_cast<double>(m_charge));
    }


  protected:

    /** Layer encoding.
     *
     *  SuperLayer: bits 1 -  4 (/4096)          <br>
     *  Layer:      bits 5 -  7 (% 4096, / 512)   <br>
     *  Wire:       bits 8 - 16 (% 512)
     */
    unsigned short int m_wireId;

    /** Drift Time.
     *
     *  CAUTION! <br>
     *  Currently a drift lenght is saved instead of a drift time.
     *
     *  Drift time should be saved in such a unit, that at least um drift lenght precision is given.
     *  Drift lenght is saved in um.
     */
    unsigned short int m_driftTime;

    /** Integrated charge in the cell.
     *
     *  Unit = ???
     *
     *  Input from external setters is multiplied by 10^-7.
     */
    unsigned short int m_charge;

  private:

    /** ROOT Macro.*/
    ClassDef(CDCHit, 1);

  };

} // end namespace Belle2

#endif
