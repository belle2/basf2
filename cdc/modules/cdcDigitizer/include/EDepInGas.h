/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace CDC {
    //! The Class for Energy deposit in the gas
    /*! This class provides the energy deposit in the gas.
    */
    class EDepInGas {

    public:

      //! Destructor
      virtual ~EDepInGas();

      //! Static method to get a reference to the EDepInGas instance.
      /*!
      \return A reference to an instance of this class.
      */
      static EDepInGas& getInstance();

      /**
       * Initialize theclass.
       */
      void initialize();

      /**
       * Return the energy deosite in the gas.
       * @param mode 0: simple scaling; 1: scaling based on mpe; 2: based on probab.
       * @param pdg pdg code of incoming particle
       * @param p absolute momentum (GeV)
       * @param dx thickness of material (cm)
       * @param e3 energy deposit in gas+wire
       */
      double getEDepInGas(int mode, int pdg, double p, double dx, double e3) const;

      /**
       * Return the energy deosite in the material.
       * @param p absolute momentum (GeV)
       * @param mass lmass of incoming particle (GeV)
       * @param zi z of incoming particle
       * @param dx thickness            of material (cm)
       * @param z  atomic number        of material
       * @param a  atomic weight        of material
       * @param i  ionization potential of material (eV)
       * @param rho density             of material
       */
      double getMostProbabEDep(double p, double mass, double zi, double dx, double z, double a, double i, double rho) const;

    private:
      /** Singleton class */
      EDepInGas();
      /** Singleton class */
      EDepInGas(const EDepInGas&);
      /** Singleton class */
      EDepInGas& operator=(const EDepInGas&);

      double m_z1 = 0;   /*!< Z   of He-C2H6 gas */
      double m_a1 = 0;   /*!< A   of He-C2H6 gas */
      double m_i1 = 0;   /*!< I   of He-C2H6 gas */
      double m_rho1 = 0; /*!< rho of He-C2H6 gas */

      double m_z2 = 0;   /*!< Z   of "wire gas" */
      double m_a2 = 0;   /*!< A   of "wire gas" */
      double m_i2 = 0;   /*!< I   of "wire gas" */
      double m_rho2 = 0; /*!< rho of "wire gas" */

      double m_z0 = 0;   /*!< Z   of wire+gas */
      double m_a0 = 0;   /*!< A   of wire+gas */
      double m_i0 = 0;   /*!< I   of wire+gas */
      double m_rho0 = 0; /*!< rho of wire+gas */

      const double m_massE = 0.51099895e6; /*!< electron mass */
      double m_ak1[6][2] = {{ }}; /*!< coeffs a for density effect */
      double m_bk1[6][2] = {{ }}; /*!< coeffs b for density effect */
      double m_ck1[6][2] = {{ }}; /*!< coeffs c for density effect */

      static EDepInGas* m_pntr; /*!< Pointer that saves the instance of this class. */

    };

  } // end of namespace CDC
} // end of namespace Belle2
