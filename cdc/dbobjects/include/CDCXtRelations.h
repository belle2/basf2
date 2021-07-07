/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <map>
#include <array>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <TObject.h>

namespace Belle2 {
  /**
   * Database object for xt-relations.
   */
  class CDCXtRelations: public TObject {

    typedef std::array<float, 3> array3; /**< angle bin info. */
    typedef unsigned short XtID;         /**< id. for xt */

  public:
    /**
     * Constants
     */
    enum {c_nSLayers      = 56, /**< no. of layers */
          c_maxNAlphaBins = 18, /**< max. no. of alpha angle bins */
          c_maxNThetaBins =  7, /**< max. no. of theta angle bins */
          c_maxNXtParams  =  8  /**< max. no. of params. for xt */
         };

    /**
     * Default constructor
     */
    CDCXtRelations() : m_xtParamMode(0), m_nXtParams(0)
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
     * Set xt parameterization mode
     */
    void setXtParamMode(unsigned short mode)
    {
      m_xtParamMode = mode;
    }

    /**
     * Set xt parameters for the specified id. (=bin)
     */
    void setXtParams(const XtID xtID, const std::vector<float>& params)
    {
      unsigned short nXtParams = params.size();

      if (nXtParams <= c_maxNXtParams) {
        m_nXtParams = nXtParams;
        m_xts.insert(std::pair<XtID, std::vector<float>>(xtID, params));
        //  std::cout <<"xtID in setXtParams= " << xtID << std::endl;
      } else {
        B2FATAL("The no. of xt params. > limit !");
      }
    }

    /**
     * Set xt parameters for the specified id. (=bin)
     */
    void setXtParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta,
                     const std::vector<float>& params)
    {
      const XtID xtID = getXtID(iCLayer, iLR, iAlpha, iTheta);
      setXtParams(xtID, params);
    }

    /**
     * Update xt parameters for the specified id. (=bin)
     */
    void addXTParams(const XtID xtID, const std::vector<float>&  delta)
    {
      std::map<XtID, std::vector<float>>::iterator it = m_xts.find(xtID);

      if (it != m_xts.end()) {
        for (unsigned short i = 0; i < m_nXtParams; ++i) {
          (it->second)[i] += delta[i];
        }
      } else {
        B2FATAL("Specified xt params not found in addXTParams !");
      }
    }

    /**
     * Update xt parameters for the specified id. (=bin)
     */
    void addXTParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta,
                     const std::vector<float>& delta)
    {
      const XtID xtID = getXtID(iCLayer, iLR, iAlpha, iTheta);
      addXTParams(xtID, delta);
    }

    /**
     * Replace xt parameters for the specified id. (=bin)
     */
    void replaceXTParams(const XtID xtID, const std::vector<float>&  param)
    {
      std::map<XtID, std::vector<float>>::iterator it = m_xts.find(xtID);

      if (it != m_xts.end()) {
        for (unsigned short i = 0; i < m_nXtParams; ++i) {
          (it->second)[i] = param[i];
        }
      } else {
        B2FATAL("Specified xt params not found in replaceXTParams !");
      }
    }

    /**
     * Replace xt parameters for the specified id. (=bin)
     */
    void replaceXTParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta,
                         const std::vector<float>& param)
    {
      const XtID xtID = getXtID(iCLayer, iLR, iAlpha, iTheta);
      replaceXTParams(xtID, param);
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
     * Get xt parameterization mode
     */
    unsigned short getXtParamMode() const
    {
      return m_xtParamMode;
    }

    /**
     * Get xt id. (=bin id.)
     * bit#00 - 05: used for layer no.
     * bit#06 - 06: used for left/right
     * bit#07 - 11: used for alpha angle bin
     * bit#12 - 15: used for theta angle bin
     */
    XtID getXtID(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha, unsigned short iTheta) const
    {
      XtID id = iCLayer + 64 * iLR + 128 * iAlpha + 4096 * iTheta;
      return id;
    }

    /**
     * Get xt id. (=bin id.)
     */
    XtID getXtID(unsigned short iCLayer, unsigned short iLR, float alpha, float theta) const
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
      if (iAlpha == 999) B2FATAL("Alpha bin not found ! " << alpha);

      return getXtID(iCLayer, iLR, iAlpha, iTheta);
    }

    /**
     * Get xt parameters for the specified id. (=bin)
     */
    const std::vector<float>& getXtParams(const XtID xtID) const
    {
      std::map<XtID, std::vector<float>>::const_iterator it = m_xts.find(xtID);
      if (it != m_xts.end()) {
        return it->second;
      } else {
        B2FATAL("Specified xt params. not found in getXtParams !");
      }
    }

    /**
     * Get xt parameters for the specified id. (=bin)
     */
    const std::vector<float>& getXtParams(unsigned short iCLayer, unsigned short iLR, unsigned short iAlpha,
                                          unsigned short iTheta) const
    {
      const XtID xtID = getXtID(iCLayer, iLR, iAlpha, iTheta);
      //  std::cout <<"xtID in getXtParams= " << xtID << std::endl;
      return getXtParams(xtID);
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Contents of xt db" << std::endl;
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
      std::cout << "coefficients for xt" << std::endl;

      for (unsigned short iT = 0; iT < nThetaBins; ++iT) {
        for (unsigned short iA = 0; iA < nAlphaBins; ++iA) {
          for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
            for (unsigned short iLR = 0; iLR < 2; ++iLR) {
              unsigned short iLRp = abs(iLR - 1);
              std::cout << iCL << "  " << deg* m_thetaBins[iT][2] << "  " << deg* m_alphaBins[iA][2] << "  " << iLRp;
              const std::vector<float> params = getXtParams(iCL, iLRp, iA, iT);
              for (unsigned short i = 0; i < m_nXtParams; ++i) {
                std::cout << "  " << params[i];
              }
              std::cout << "  " << std::endl;
            }
          }
        }
      }
    }

    /**
     * Output the contents in test file format
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

        fout << m_xtParamMode << "  " << m_nXtParams << std::endl;

        signed short phiAngle = 0.;
        for (unsigned short iT = 0; iT < nThetaBins; ++iT) {
          for (unsigned short iA = 0; iA < nAlphaBins; ++iA) {
            for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
              for (unsigned short iLR = 0; iLR < 2; ++iLR) {
                unsigned short iLRp = abs(iLR - 1);
                fout << std::setw(2) << std::right << std::fixed << iCL << "  " << std::setw(5) << std::setprecision(
                       1) << deg* m_thetaBins[iT][2] << "  " << std::setw(5) << std::right << deg* m_alphaBins[iA][2] << "  " << std::setw(
                       1) << std::setprecision(1) << phiAngle << "  " << std::setw(1) << iLRp;
                const std::vector<float> params = getXtParams(iCL, iLRp, iA, iT);
                for (unsigned short i = 0; i < m_nXtParams; ++i) {
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

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 29;}
    /// Get global parameter for i-th component of the specified xtId
    double getGlobalParam(unsigned short xtId, unsigned short i) const
    {
      return getXtParams(xtId).at(i);
    }
    /// Set global parameter for i-th component of the specified xtId
    void setGlobalParam(double value, unsigned short xtId, unsigned short i)
    {

      std::map<XtID, std::vector<float>>::const_iterator it = m_xts.find(xtId);
      if (it != m_xts.end()) {
        std::vector<float> allParams =  it->second;
        allParams.at(i) = value;
        setXtParams(xtId, allParams);
      } else {
        B2INFO("Specified xt params. not found in getXtParams.");
        std::vector<float> allParams {0., 0., 0., 0., 0., 0., 0., 0.};
        allParams.at(i) = value;
        setXtParams(xtId, allParams);
      }
    }
    /// list stored global parameters TODO FIXME CDC not ready
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() const
    {
      std::vector<std::pair<unsigned short, unsigned short>> result;
      for (auto ixt : m_xts) {
        for (int i = 0; i < 8; ++i) {
          result.push_back({ixt.first, i});
        }
      }
      return result;
    }
  private:
    unsigned short m_xtParamMode;    /*!< Mode for xt parameterization */
    unsigned short m_nXtParams;      /*!< no. of xt parameters per bin */
    std::vector<array3> m_alphaBins; /*!< alpha bins for xt (rad) */
    std::vector<array3> m_thetaBins; /*!< theta bins for xt (rad) */
    std::map<XtID, std::vector<float>>
                                    m_xts;    /*!< XT-relation coefficients for each layer, Left/Right, entrance angle and polar angle.  */

    ClassDef(CDCXtRelations, 2); /**< ClassDef */
  };

} // end namespace Belle2
