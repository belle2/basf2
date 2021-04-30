/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Parametrization of background PDF in pixels of single module
     */
    class BackgroundPDF {

    public:

      /**
       * Class constructor
       * @param moduleID slot ID
       */
      explicit BackgroundPDF(int moduleID);

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns PDF value for given pixel
       * @param pixelID pixel ID
       * @return PDF value
       */
      double getPDFValue(int pixelID) const;

      /**
       * Returns pixel part of PDF
       * @return pixel part of PDF (index = pixelID - 1)
       */
      const std::vector<double>& getPDF() const {return m_pdf;}

      /**
       * Returns average of pixel relative efficiencies
       * @return average of pixel relative efficiencies
       */
      double getEfficiency() const {return m_effi;}

    private:

      /**
       * Sets the PDF
       */
      void set();

      int m_moduleID; /**< slot ID */
      std::vector<double> m_pdf; /**< pixel part of PDF (index = pixelID - 1) */
      double m_effi = 0; /**< average relative efficiency */

      friend class TOPRecoManager;

    };

  } // namespace TOP
} // namespace Belle2


