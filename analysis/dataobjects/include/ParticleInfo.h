/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEINFO_H
#define PARTICLEINFO_H

#include <framework/datastore/RelationsObject.h>

#include<string>
#include<map>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /*! Class which holds additional information about a Particle.
   *  Use the ParticleInfoModule to create a ParticleInfo and relate it to a Particle.
   *  This module is used by the TMVAExpertModule to store the calculated Target Variable for every particle
   */

  class ParticleInfo : public RelationsObject {

  public:

    /*! default constructor:  */
    ParticleInfo();

    /*! adds variable with specified name and given value to the list of variables
     * @param name Name of variable which is stored in ParticleInfo
     * @param value Value of variable which is stored in ParticleInfo
     * @return returns true if operation is successful and false if operation fails (if the variable with same name is already included)
     */
    bool addVariable(const std::string name, const float value);

    /*! checks if a variable is available in the ParticleInfo
     * @param name Name of the variable
     * @return returns true if if ParticleInfo contains a value for the given variable name, otherwise false
     */
    bool isAvailable(const std::string name) const;

    /*! get stored value of variable
     * @param name Name of the variable
     * @return value of variable or 0 if given name doesn't exist
     */
    float getValue(const std::string name) const;


  private:

    std::map<std::string, float> m_variables;  /**< map which holds the values for every variable */

    ClassDef(ParticleInfo, 1)

  };

  /** @}*/

} // end namespace Belle2

#endif
