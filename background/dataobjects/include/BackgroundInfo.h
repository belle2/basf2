/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BACKGROUNDINFO_H
#define BACKGROUNDINFO_H

#include <TObject.h>
#include <string>

namespace Belle2 {

  /**
   * This class stores information about the background origin of a MCParticle.
   */
  class BackgroundInfo: public TObject {

  public:

    /** Default constructor. */
    BackgroundInfo() : m_component(""), m_generator("") {}

    /** Constructor.
     * @param component The background component (e.g. Touschek).
     * @param generator The background generator (e.g. SAD_LER)
     */
    BackgroundInfo(const std::string& component, const std::string& generator) :
      m_component(component), m_generator(generator) {}


    /** Destructor. */
    virtual ~BackgroundInfo() {}

    /**
     * Returns the background component.
     * @return The background component.
     */
    const std::string& getComponent() const { return m_component; }

    /**
     * Returns the background generator.
     * @return The background generator.
     */
    const std::string& getGenerator() const { return m_generator; }


  protected:

    std::string m_component; /**< The background component (e.g. Touschek). */
    std::string m_generator; /**< The background generator (e.g. SAD_LER). */

    /** Class definition required for the creation of the ROOT dictionary. */
    ClassDef(BackgroundInfo, 1);
  };
}


#endif /* BACKGROUNDINFO_H */
