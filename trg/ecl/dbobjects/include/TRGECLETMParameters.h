/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLETMPARAMETERS_H
#define TRGECLETMPARAMETERS_H

#include <TObject.h>
#include <stdexcept>

namespace Belle2 {

  //! DB object to store parameters used in ETM
  class TRGECLETMParameters : public TObject {
  public:

    //! Constructor
    TRGECLETMParameters() : m_parNames{{0}}, m_parValues{0}, m_npar(0) {}

    //! set the number of parameters
    void setnpar(int npar)
    {
      m_npar = npar;
    }
    //! set parameter name
    void setparNames(int iii, const std::string& parname)
    {
      m_parNames[iii] = parname;
    }
    //! set parameter value
    void setparValues(int iii, double parvalue)
    {
      m_parValues[iii] = parvalue;
    }
    //! get the number of parameters
    int getnpar(void) const
    {
      return m_npar;
    }
    //! get parameter name
    const std::string getparNames(int iii) const
    {
      return m_parNames[iii];
    }
    //! get parameter value from the index
    double getparValues(int iii) const
    {
      return m_parValues[iii];
    }
    //! get parameter value from the parameter name
    double getparValues(const std::string& parname) const
    {
      for (int iii = 0; iii < m_npar; iii++) {
        if (getparNames(iii) == parname) {
          return getparValues(iii);
        }
      }
      throw std::invalid_argument("[TRGECLParameters] The parameter does not exist : "
                                  + parname);
    }

  private :

    //! parameter names
    std::string m_parNames[300];
    //! parameter values
    double m_parValues[300];
    //! the number of valid parameters
    int m_npar;

    //! the class title
    ClassDef(TRGECLETMParameters, 1); /*< the class title */
  };
}
#endif
