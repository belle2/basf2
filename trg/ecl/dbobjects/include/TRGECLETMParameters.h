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
#include <map>

namespace Belle2 {

  //! DB object to store parameters used in ETM
  class TRGECLETMParameters : public TObject {
  public:

    //! Constructor
    TRGECLETMParameters() : m_npar(0) {}

    //! set the number of parameters
    void setnpar(int npar)
    {
      m_npar = npar;
    }
    //! set parameter map(name and value)
    void setparMap(std::string parname, double parvalue)
    {
      m_parmap.insert(std::make_pair(parname, parvalue));
    }
    //! get the number of parameters
    int getnpar(void) const
    {
      return m_npar;
    }
    //! get parameter map(name and value)
    const std::map<std::string, double>& getparMap(void) const
    {
      return m_parmap;
    }

  private :

    //! parameter map(name and value)
    std::map<std::string, double> m_parmap;
    //! the number of valid parameters
    int m_npar;

    //! the class title
    ClassDef(TRGECLETMParameters, 1); /*< the class title */
  };
}
#endif
