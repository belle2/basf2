/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VXD_SIMULATION_ELECTRONDEPOSIT_H
#define VXD_SIMULATION_ELECTRONDEPOSIT_H

#include <algorithm>
#include <cmath>

namespace Belle2 {
  /** Packed class to represent energy deposit along a path in electrons.
   *
   * This class is used by the VXD to efficiently store the number of electrons
   * deposited in the silicon along a given path by storing the fraction along
   * the path and the number of electrons at that point in a packed way. The
   * fraction is encoded between 0 and 1 using 8 bits and the number of
   * electrons is stored as 24bit unsigned integer which allows for 16.7
   * million electrons per step. As the electron hole pair production energy is
   * 3eV in silicon this corresponds to an energy loss of around 50GeV so there
   * should be no problem at all.
   */
  class VXDElectronDeposit {
  public:
    /** Enum storing the bit informations for the packed information */
    enum EBits {
      /** Number of bits to encode the fraction along the path */
      c_FractionBits = 10,
      /** Number of bits to encode the number of electrons */
      c_ElectronBits = 22,
      /** Total number of bits needed for the packed structure */
      c_TotalBits = c_FractionBits + c_ElectronBits,
      /** Maximum integer value allowed for the fraction part */
      c_MaxFraction = (1 << c_FractionBits) - 1,
      /** Maximum number of electrons allowed */
      c_MaxElectrons = (1 << c_ElectronBits) - 1,
    };

    /** Create a new instance from a packed value
     * @param packed value to be interpreted as packed fraction/electrons */
    explicit VXDElectronDeposit(unsigned int packed): m_packed(packed) {}

    /** create a new instance from a fraction along the path and a number of
     * electrons.
     * @param fraction fraction along the path, should be 0<=x<=1
     * @param electrons number of electrons deposited
     */
    VXDElectronDeposit(float fraction, float electrons)
    {
      const unsigned int intfrac = std::round(std::max(0.f, fraction) * c_MaxFraction);
      const unsigned int intelec = std::round(std::max(0.f, electrons));
      m_packed = std::min(intfrac, (unsigned int) c_MaxFraction)
                 + (std::min(intelec, (unsigned int) c_MaxElectrons) << c_FractionBits);
    }
    /** get the fraction along the path */
    float getFraction() const
    {
      return (m_packed & c_MaxFraction) * 1.0 / c_MaxFraction;
    }
    /** get the number of deposited electrons */
    unsigned int getElectrons() const
    {
      return (m_packed >> c_FractionBits) & c_MaxElectrons;
    }
    /** get the packed value */
    unsigned int getPacked() const { return m_packed; }
    /** convert to unsigned int using the packed representation */
    operator unsigned int() const { return m_packed; }
  private:
    /** packed representation of the fraction and electron deposition */
  unsigned int m_packed: c_TotalBits;
  };

} //Belle2 namespace
#endif // VXD_SIMULATION_ELECTRONDEPOSIT_H
