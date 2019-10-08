/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <map>
#include <array>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <TObject.h>

namespace Belle2 {
  /**
   * Database object for space resolutions.
   */
  class CDCSpaceResols: public TObject {

    typedef std::array<float, 3> array3; /**< angle bin info. */
    typedef unsigned short SigmaID;         /**< id. for resol. */

  public:
    /**
     * Constants
     */
    enum {c_nSLayers      = 56, /**< no. of layers */
          c_maxNAlphaBins = 18, /**< max. no. of alpha angle bins */
          c_maxNThetaBins =  7, /**< max. no. of theta angle bins */
          c_maxNSigmaParams =  9  /**< max. no. of params. for sigma */
         };

    /**
     * Default constructor
     */
    CDCSpaceResols() : m_sigmaParamMode(0), m_nSigmaParams(0)
    {}

    /**
     * Set alpha-angle bin (rad)
     */
    void setAlphaBin(const array3& alpha)
    {
      if (m_alphaBins.size() <= c_maxNAlphaBins) {
        m_alphaBins.push_back(alpha);
        sort(m_alphaBins.begin(), m_alphaBins.end(), comp);
      } else {
        //  std::cout<< m_alphaBins.size() <<" "<< c_maxNAlphaBins <<std::endl;
        B2FATAL("The no. of alpha bins > limit !");
      }
    }

    /**
     * Set theta-angle bin (rad)
     */
    void setThetaBin(const array3& theta)
    {
      if (m_thetaBins.size() <= c_maxNThetaBins) {
        m_thetaBins.push_back(theta);
        sort(m_thetaBins.begin(), m_thetaBins.end(), comp);
      } else {
        B2FATAL("The no. of theta bins > limit !");
      }
    }

    /**
     * Static function for sorting
     */
    static bool comp(const array3& lhs, const array3& rhs)
    {
      return lhs[0] < rhs[0];
    }

    /**
     * Set sigma parameterization mode
     */
    void setSigmaParamMode(unsigned short mode)
    {
      m_sigmaParamMode = mode;
    }

    /**
     * Set sigma parameters for the specified id. (=bin)
     */
    void setSigmaParams(const SigmaID sigmaID, const std::vector<float>& params)
    {
      unsigned short nSigmaParams = params.size();

      if (nSigmaParams <= c_maxNSigmaParams) {
        m_nSigmaParams = nSigmaParams;
        m_sigmas.insert(std::pair<SigmaID, std::vector<float>>(sigmaID, params));
        //  std::cout <<"sigmaID in setSigmaParams= " << sigmaID << std::endl;
      } else {
        B2FATAL("The no. of sigma params. > limit !");
      }
    }

    /**
     * Set sigma parameters for the specified id. (=bin)
     */
    void setSigmaParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta,
                        const std::vector<float>& params)
    {
      const SigmaID sigmaID = getSigmaID(iCLayer, iLR, iAlpha, iTheta);
      setSigmaParams(sigmaID, params);
    }

    /**
     * Update parameters for the specified id. (=bin)
     */
    void addSigmaParams(const SigmaID sigmaID, const std::vector<float>&  delta)
    {
      std::map<SigmaID, std::vector<float>>::iterator it = m_sigmas.find(sigmaID);

      if (it != m_sigmas.end()) {
        for (unsigned short i = 0; i < m_nSigmaParams; ++i) {
          (it->second)[i] += delta[i];
        }
      } else {
        B2FATAL("Specified params not found in addSigmaParams !");
      }
    }

    /**
     * Update parameters for the specified id. (=bin)
     */
    void addSigmaParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta,
                        const std::vector<float>& delta)
    {
      const SigmaID sigmaID = getSigmaID(iCLayer, iLR, iAlpha, iTheta);
      addSigmaParams(sigmaID, delta);
    }


    /**
     * Get no. of alpha-angle bin
     */
    unsigned short getNoOfAlphaBins() const
    {
      return m_alphaBins.size();
    }

    /**
     * Get no. of theta-angle bin
     */
    unsigned short getNoOfThetaBins() const
    {
      return m_thetaBins.size();
    }

    /**
     * Get i-th alpha-angle bin info. (rad)
     */
    const array3& getAlphaBin(unsigned short i) const
    {
      return m_alphaBins[i];
    }

    /**
     * Get i-th alpha-angle point (rad)
     */
    float getAlphaPoint(unsigned short i) const
    {
      return m_alphaBins[i][2];
    }

    /**
     * Get i-th theta-angle bin info. (rad)
     */
    const array3& getThetaBin(unsigned short i) const
    {
      return m_thetaBins[i];
    }

    /**
     * Get i-th theta-angle point (rad)
     */
    float getThetaPoint(unsigned short i) const
    {
      return m_thetaBins[i][2];
    }

    /**
     * Get parameterization mode
     */
    unsigned short getSigmaParamMode() const
    {
      return m_sigmaParamMode;
    }

    /**
     * Get id. (=bin id.)
     * bit#00 - 05: used for layer no.
     * bit#06 - 06: used for left/right
     * bit#07 - 11: used for alpha angle bin
     * bit#12 - 15: used for theta angle bin
     */
    SigmaID getSigmaID(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta) const
    {
      SigmaID id = iCLayer + 64 * iLR + 128 * iAlpha + 4096 * iTheta;
      return id;
    }

    /**
     * Get id. (=bin id.)
     */
    SigmaID getSigmaID(unsigned short iCLayer, unsigned short iLR, float alpha, float theta) const
    {
      /*
      unsigned short iTheta = 999;
      unsigned short ibin = 0;
      for (std::vector<array3>::const_iterator it = m_thetaBins.begin(); it != m_thetaBins.end(); ++it) {
      if ((*it)[0] <= theta && theta <= (*it)[1]) {
      iTheta = ibin;
      break;
      }
      ++ibin;
      }
      */
      unsigned short iTheta = 999;
      unsigned short ibin = 0;
      for (auto const& it : m_thetaBins) {
        if (it[0] <= theta && theta <= it[1]) {
          iTheta = ibin;
          break;
        }
        ++ibin;
      }
      if (iTheta == 999) B2FATAL("Theta bin not found !");

      /*
      unsigned short iAlpha = 999;
      ibin = 0;
      for (std::vector<array3>::const_iterator it = m_alphaBins.begin(); it != m_alphaBins.end(); ++it) {
      if ((*it)[0] <= alpha && alpha <= (*it)[1]) {
      iAlpha = ibin;
      break;
      }
      ++ibin;
      }
      */
      unsigned short iAlpha = 999;
      ibin = 0;
      for (auto const& it : m_alphaBins) {
        if (it[0] <= alpha && alpha <= it[1]) {
          iAlpha = ibin;
          break;
        }
        ++ibin;
      }
      if (iAlpha == 999) B2FATAL("Alpha bin not found !");

      return getSigmaID(iCLayer, iLR, iAlpha, iTheta);
    }

    /**
     * Get sigma parameters for the specified id. (=bin)
     */
    const std::vector<float>& getSigmaParams(const SigmaID sigmaID) const
    {
      std::map<SigmaID, std::vector<float>>::const_iterator it = m_sigmas.find(sigmaID);
      if (it != m_sigmas.end()) {
        return it->second;
      } else {
        B2FATAL("Specified params. not found in getSigmaParams !");
      }
    }

    /**
     * Get sigma parameters for the specified id. (=bin)
     */
    const std::vector<float>& getSigmaParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha,
                                             unsigned short iTheta) const
    {
      const SigmaID sigmaID = getSigmaID(iCLayer, iLR, iAlpha, iTheta);
      //  std::cout <<"sigmaID in getSigmaParams= " << sigmaID << std::endl;
      return getSigmaParams(sigmaID);
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Contents of sigma db" << std::endl;
      std::cout << "alpha bins" << std::endl;

      const double deg = 180. / M_PI;

      unsigned short nAlphaBins = m_alphaBins.size();
      for (unsigned short i = 0; i < nAlphaBins; ++i) {
        std::cout << " " << deg* m_alphaBins[i][0] << " " << deg* m_alphaBins[i][1] << " " << deg* m_alphaBins[i][2] << " " << std::endl;
      }

      std::cout << " " << std::endl;
      std::cout << "theta bins" << std::endl;

      unsigned short nThetaBins = m_thetaBins.size();
      for (unsigned short i = 0; i < nThetaBins; ++i) {
        std::cout << " " << deg* m_thetaBins[i][0] << " " << deg* m_thetaBins[i][1] << " " << deg* m_thetaBins[i][2] << " " << std::endl;
      }

      std::cout << " " << std::endl;
      std::cout << "coefficients for sigma" << std::endl;

      for (unsigned short iT = 0; iT < nThetaBins; ++iT) {
        for (unsigned short iA = 0; iA < nAlphaBins; ++iA) {
          for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
            for (unsigned short iLR = 0; iLR < 2; ++iLR) {
              unsigned short iLRp = abs(iLR - 1);
              std::cout << iCL << "  " << deg* m_thetaBins[iT][2] << "  " << deg* m_alphaBins[iA][2] << "  " << iLRp;
              const std::vector<float> params = getSigmaParams(iCL, iLRp, iA, iT);
              for (unsigned short i = 0; i < m_nSigmaParams; ++i) {
                std::cout << "  " << params[i];
              }
              std::cout << "  " << std::endl;
            }
          }
        }
      }
    }

    /**
     * Output the contents in text file format
     */
    void outputToFile(std::string fileName) const
    {
      std::ofstream fout(fileName);

      if (fout.bad()) {
        B2ERROR("Specified output file could not be opened!");
      } else {
        const double deg = 180. / M_PI;

        unsigned short nAlphaBins = m_alphaBins.size();
        fout << nAlphaBins << std::endl;

        for (unsigned short i = 0; i < nAlphaBins; ++i) {
          fout << deg* m_alphaBins[i][0] << "  " << deg* m_alphaBins[i][1] << "  " << deg* m_alphaBins[i][2] << std::endl;
        }

        unsigned short nThetaBins = m_thetaBins.size();
        fout << nThetaBins << std::endl;

        for (unsigned short i = 0; i < nThetaBins; ++i) {
          fout << deg* m_thetaBins[i][0] << "  " << deg* m_thetaBins[i][1] << "  " << deg* m_thetaBins[i][2] << std::endl;
        }

        fout << m_sigmaParamMode << "  " << m_nSigmaParams << std::endl;

        for (unsigned short iT = 0; iT < nThetaBins; ++iT) {
          for (unsigned short iA = 0; iA < nAlphaBins; ++iA) {
            for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
              for (unsigned short iLR = 0; iLR < 2; ++iLR) {
                unsigned short iLRp = abs(iLR - 1);
                fout << std::setw(2) << std::right << std::fixed << iCL << "  " << std::setw(5) << std::setprecision(
                       1) << deg* m_thetaBins[iT][2] << "  " << std::setw(5) << std::right << deg* m_alphaBins[iA][2] << "  " << std::setw(1) << iLRp;
                const std::vector<float> params = getSigmaParams(iCL, iLRp, iA, iT);
                for (unsigned short i = 0; i < m_nSigmaParams; ++i) {
                  fout << "  " << std::setw(15) << std::scientific << std::setprecision(8) << params[i];
                }
                fout << std::endl;
              }
            }
          }
        }
        fout.close();
      }
    }

  private:
    unsigned short m_sigmaParamMode;    /*!< Mode for sigma parameterization */
    unsigned short m_nSigmaParams;      /*!< no. of sigma parameters per bin */
    std::vector<array3> m_alphaBins; /*!< alpha bins for sigma (rad) */
    std::vector<array3> m_thetaBins; /*!< theta bins for sigma (rad) */
    std::map<SigmaID, std::vector<float>>
                                       m_sigmas;    /*!< Sigma coefficients for each layer, Left/Right, entrance angle and polar angle.  */

    ClassDef(CDCSpaceResols, 1); /**< ClassDef */
  };

} // end namespace Belle2
