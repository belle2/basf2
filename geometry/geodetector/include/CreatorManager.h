/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORMANAGER_H_
#define CREATORMANAGER_H_

#include <string>
#include <map>
#include <list>

#include <framework/core/FrameworkExceptions.h>

namespace Belle2 {

  class CreatorBase;

  /**
   * The CreatorManager class.
   *
   * This class manages the self registered creators.
   * It is designed as a singleton.
   */
  class CreatorManager {

  public:

    //Define exceptions
    /** Exception is thrown if the user tries to create a Creator having an empty name. */
    BELLE2_DEFINE_EXCEPTION(GeometryCreatorNameEmptyError, "Can't create a Creator with an empty name !");
    /** Exception is thrown if the system tries to register a Creator carrying the same name as an already existing Creator. */
    BELLE2_DEFINE_EXCEPTION(GeometryCreatorAlreadyExistsError, "A Creator with the name '%1%' already exists !");
    /** Exception is thrown if the system tries to access a Creator by name but could not find it.. */
    BELLE2_DEFINE_EXCEPTION(GeometryCreatorNotExistsError, "A Creator with the name '%1%' does not exist !");

    /**
     * Static method to get a reference to the CreatorManager instance.
     *
     * @return A reference to an instance of this class.
     */
    static CreatorManager& Instance();

    /**
     * Registers a new creator to the manager. The manager owns the creator and will delete it automatically.
     *
     * Each creator registers itself to the Creator Manager using this method.
     *
     * Throws an exception of type GeometryCreatorNameEmptyError if the name of the creator is empty.
     * Throws an exception of type GeometryCreatorAlreadyExistsError if a creator carrying the same name already exists
     * and force is set to false.
     *
     * @param creator Pointer to the creator which should be registered.
     * @param force If this flag is true the creator will overwrite another creator carrying the same name.
     */
    void registerCreator(CreatorBase* creator, bool force = false) throw(GeometryCreatorNameEmptyError, GeometryCreatorAlreadyExistsError);

    /**
     * Returns a reference to a creator carrying the given name.
     *
     * Throws an exception of type GeometryCreatorNameEmptyError if the name given is empty.
     * Throws an exception of type GeometryCreatorNotExistsError if a creator carrying the given name does not exist.
     *
     * @param name The name of the creator which should be returned.
     * @return A reference to the creator carrying the given name.
     */
    CreatorBase& getCreator(const std::string& name) const throw(GeometryCreatorNameEmptyError, GeometryCreatorNotExistsError);

    /**
     * Returns a list of names of the available creators.
     *
     * @return A list of names of the available creators.
     */
    std::list<std::string> getCreatorList() const;


  protected:

    std::map<std::string, CreatorBase*> m_creatorMap; /**< Map of all creators registered to the manager with their name as key. */


  private:

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    CreatorManager();

    /** Disable/Hide the copy constructor. */
    CreatorManager(const CreatorManager&);

    /** Disable/Hide the copy assignment operator. */
    CreatorManager& operator=(const CreatorManager&);

    /** The CreatorManager destructor. */
    ~CreatorManager();

    static CreatorManager* m_instance; /**< Pointer that saves the instance of this class. */

    /** Destroyer class to delete the instance of the CreatorManager class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (CreatorManager::m_instance != NULL) delete CreatorManager::m_instance;
      }
    };
    friend class SingletonDestroyer;
  };

} //end of namespace Belle2

#endif /* CREATORMANAGER_H_ */
