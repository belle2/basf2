/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <string>
#include <functional>
#include <vector>
#include <type_traits>


namespace Belle2 {
  /** Type-independent implementation details of SelectSubset. */
  class SelectSubsetBase {
  public:
    /** Get list of arrays we inherit relations from. */
    std::vector<std::string> getInheritFromArrays() const { return m_inheritFromArrays; }
    /** Get list of arrays we inherit relations to. */
    std::vector<std::string> getInheritToArrays() const { return m_inheritToArrays; }
    /** Do we inherit relations from original set to itself? */
    bool getInheritToSelf() const { return m_inheritToSelf; }

    /** Get accessor for original set. */
    virtual StoreAccessorBase* getSet() = 0;
    /** Get accessor for reduced set. */
    virtual StoreAccessorBase* getSubSet() = 0;

  protected:
    SelectSubsetBase() { }
    virtual ~SelectSubsetBase() { }

    /** Swap set and subset (+relations), and keep only the reduced set.
     *
     * Subset and associated relations will be empty afterwards.
     */
    void swapSetsAndDestroyOriginal();

    /** array names we inherit relations from. */
    std::vector<std::string> m_inheritFromArrays;
    /** array names we inherit relations to. */
    std::vector<std::string> m_inheritToArrays;
    /** If true, relations from set objects to set objects are copied. (if both objects are selected!). */
    bool m_inheritToSelf = false;
    /** If true, non-selected candidates are removed from m_set, m_subset only exists temporarily. */
    bool m_reduceExistingSet = false;
  };

  /** Class to create a subset of a given StoreArray together with the relations with other StoreArrays.
   *
   * The class SelectSubset selects a subset of objects contained in a given StoreArray
   * creating at the same time a set of relations with objects contained in other StoreArrays
   * that are the natural restrictions on the subset of the relations "from" or "to" the original
   * one.
   *
   *  <h1>Creating a subset</h1>
   *  Assuming you have a StoreArray called 'particles' that contains objects of type Particle
   *  you can use SelectSubset to select particles with a given feature and put them
   *  in another StoreArray called, as an example,'oddParticles'.
   *
   *  <h2> Instantiation</h2>
   *  First you need to add a SelectSubset to your module as a member variable:
   *  \code
      SelectSubset< Particle > m_selector;
      \endcode
   *
   *  <h2> Initialization </h2>
   * In the initialize method of your module you have to initialize the SelectSubset
   * \code

     StoreArray< Particle > set( "particles" );
     set.isRequired(); // or isOptional(). The choice is up to you.
     m_selector.registerSubset( set, "oddParticles");

     \endcode

   * The SelectSubset class will take care of creating the new StoreArray<Particle>,
   * register it into the datastore with name "oddParticles" same durability and
   * same persistent attributes of the original one.
   *
   * Inheritance of relations can also be configured here, see "Relations" section below.
   *
   * <h2> Selection </h2>
   *
   * To create the subset you have to specify the selection criterium.
   * You can do that in two possible ways.
   * <h3>via C++ function </h3>
   * You can define your C++ function and then use it. E.g:
   *  \code
      bool
      MySelectionFunction( const Particle * particle){
      // this function tells if the particle is odd
      return ( particle->UniqueId() % 2 ) == 1 ;
      }
      \endcode
   * Then in the event method of your module:
   * \code
     m_selector.select( MySelectionFunction );
     \endcode
   *
   * <h3>via C++ lambda function </h3>
   * You can specify a lambda expression as parameter of the select method. E.g.:
   * in the event method of your code:
   * \code
     m_selector.select( []( const Particle * particle )
     { return  ( particle->UniqueId() % 2 ) == 1 ; } );
     \endcode
   * with the advantage of an easy capture of module parameters.
   * E.g. to count the number of rejected particles:
   * \code
     int rejected(0);
     m_selector.select( [& rejected]( const Particle * particle )
     {
       if ( ( particle->UniqueId() % 2 ) == 1 )
         return true;
       rejected ++;
       return false;
     });
     B2INFO("The selector rejected " << rejected << " particles." );
     \endcode
   *
   * <h1> Relations </h1>
   * By default the class SelectSubset produces a one to one relation
   * from the set to the subset by which you can interpret all the relations
   * from and to the original set. E.g. The original StoreArray<Particle> is
   * in relation To the StoreArray<MCParticle>. You can use the relation from
   * the set to the subset and then from the set to the MCParticles. This can
   * be quite tedious, so you can ask SelectSubset to produce the natural
   * restrictions of the relations from and to the original set.

   * <h2>Automatic inheritance</h2>
   * If you want your subset to have the same relations as for the original set,
   * you can simply use
   * \code
     m_selector.inheritAllRelations();
     \endcode
   *
   * <h2>Manually specifying relations to inherit</h2>
   * Alternatively, you can specify the arrays you want to inherit from/to manually:
   *
   * <h3> Relations to other StoreArrays</h3>
   * Assuming there is a relation from your original set to other arrays A and B,
   * you can inherit these relations for all objects selected into your subset using:
   * \code
     StoreArray<A> a;
     StoreArray<B> b;
     m_selector.inheritRelationsTo(a, b);

     //alternatively, you can also use multiple calls to the function
     m_selector.inheritRelationsTo(a);
     m_selector.inheritRelationsTo(b);
     \endcode
   *
   * <h3> Relations from other StoreArrays</h3>
   * Relations pointing from objects in other arrays to objects in the original set
   * can also be inherited in a very similar way:
   * \code
     StoreArray<C> c;
     StoreArray<D> d;
     m_selector.inheritRelationsFrom(c, d);

     //alternatively, you can also use multiple calls to the function
     m_selector.inheritRelationsFrom(c);
     m_selector.inheritRelationsFrom(d);
     \endcode
   *
   * <h3> Relations from the StoreArray to itself</h3>
   * If there are relations from objects in the original set to other objects in the same array
   * (e.g. Particles -> Particles), you can also inherit these by doing
   * \code
     m_selector.inheritRelationsFrom(set);
     // or:
     // m_selector.inheritRelationsTo(set);
     \endcode
   * Note that both objects related must pass the selection criteria, or there would
   * be one missing partner in the relation.
   */
  template < typename StoredClass >
  class SelectSubset : public SelectSubsetBase {
    static_assert(std::is_base_of<RelationsObject, StoredClass>::value,
                  "SelectSubset<T> only works with classes T inheriting from RelationsObject.");
  public:
    /** Constructor */
    SelectSubset(): SelectSubsetBase() {};

    /** Destructor */
    ~SelectSubset()
    {
      delete m_set;
      delete m_subset;
    }

    /** Remove all non-selected objects from set. All relations registered so far are retained.
     * TODO: consider moving this into StoreArray itself
     *
     *  @param set         The StoreArray<StoredClass> from which to retain only selected elements
     *  @param storeFlags  flags used for temporary arrays and relations. Should be changed
     *                     from the default if you want multiple instances.
     *                     c_DontWriteOut is always used.
     */
    void registerSubset(const StoreArray< StoredClass >& set,
                        DataStore::EStoreFlags storeFlags = DataStore::c_ErrorIfAlreadyRegistered)
    {
      m_reduceExistingSet = true;
      registerSubset(set, set.getName() + "_tmpSubset", storeFlags | DataStore::c_DontWriteOut);

      inheritAllRelations();
    }

    /** Register the StoreArray<StoredClass> that will contain the subset of selected elements
     *  @param set         The StoreArray<StoredClass> from which the elements will be selected
     *  @param subsetName  The name of the StoreArray<StoredClass> that will contain the selected elements
     *  @param storeFlags ORed combination of DataStore::EStoreFlags.
     */
    void registerSubset(const StoreArray< StoredClass >& set, const std::string& subsetName,
                        DataStore::EStoreFlags storeFlags = DataStore::c_ErrorIfAlreadyRegistered)
    {
      if (m_set or m_subset) {
        B2FATAL("SelectSubset::registerSubset() can only be called once!");
      }

      m_set = new StoreArray<StoredClass>(set);

      m_subset = new StoreArray<StoredClass>(subsetName, m_set->getDurability());
      m_subset->registerInDataStore(storeFlags);
      m_subsetFlags = storeFlags;

      set.registerRelationTo(*m_subset, m_subset->getDurability(), storeFlags);
    }

    /** Inherit relations pointing from Other to objects selected into this subset.
     *
     * You can specify an unlimited number of arrays as arguments to this function.
     */
    template<class T, class ... MoreArguments >
    void inheritRelationsFrom(const StoreArray<T>& array, MoreArguments... moreArgs)
    {
      if (array.getName() == m_set->getName()) {
        m_inheritToSelf = true;
        inheritRelationsFrom(*m_subset, moreArgs...);
      } else {
        const_cast<StoreArray<T>&>(array).isRequired();

        DataStore::EStoreFlags flags = m_subsetFlags;
        if (m_subset->notWrittenOut() or array.notWrittenOut())
          flags |= DataStore::c_DontWriteOut;
        array.registerRelationTo(*m_subset, m_subset->getDurability(), flags);

        if (array.getName() != m_subset->getName())
          m_inheritFromArrays.push_back(array.getName());

        inheritRelationsFrom(moreArgs ...);
      }
    }

    /** Inherit relations pointing from objects selected into this subset to Other.
     *
     * You can specify an unlimited number of arrays as arguments to this function.
     */
    template<class T, class ... MoreArguments >
    void inheritRelationsTo(const StoreArray<T>& array, MoreArguments... moreArgs)
    {
      if (array.getName() == m_set->getName()) {
        m_inheritToSelf = true;
        inheritRelationsTo(*m_subset, moreArgs...);
      } else {
        const_cast<StoreArray<T>&>(array).isRequired();

        DataStore::EStoreFlags flags = m_subsetFlags;
        if (m_subset->notWrittenOut() or array.notWrittenOut())
          flags |= DataStore::c_DontWriteOut;
        m_subset->registerRelationTo(array, m_subset->getDurability(), flags);

        if (array.getName() != m_subset->getName())
          m_inheritToArrays.push_back(array.getName());

        inheritRelationsTo(moreArgs ...);
      }
    }

    /** Automatically inherit all relations to or from the original set (if registered when calling this function).
     *
     * Equivalent to calling inheritRelationsFrom()/To() for all related arrays.
     *
     * Note: Do not combine with inheritRelationsFrom() and inheritRelationsTo().
     */
    void inheritAllRelations()
    {
      auto arrays = DataStore::Instance().getListOfRelatedArrays(*m_set);

      for (std::string arrayName : arrays) {
        StoreArray<TObject> array(arrayName, m_set->getDurability());
        if (array == *m_subset)
          continue; // from registerSubset(), ignore

        if (array.optionalRelationTo(*m_set, m_set->getDurability()))
          inheritRelationsFrom(array);
        if (m_set->optionalRelationTo(array, m_set->getDurability()))
          inheritRelationsTo(array);
      }
    }

    /** This method is the actual worker. It selects the elements, fill the subset and
     * all the relations in which the subset is involved.
     *  @param f the pointer to the function (or a nameless lambda expression) returning
     *  true for the elements to be selected and false for the others.
     */
    void select(const std::function<bool (const StoredClass*)>& f);

    /** Get accessor for original set. */
    StoreAccessorBase* getSet() override { return m_set; }
    /** Get accessor for reduced set. */
    StoreAccessorBase* getSubSet() override { return m_subset; }

  protected:
    /** Selects the elements, fill the subset and copies all the relations in which the set is involved. */
    std::map<int, int> copySetWithRelations(std::function<bool (const StoredClass*)> f);

    /** Copy any set -> set relations between selected objects. */
    void copyRelationsToSelf();

    /** Empty method to stop the recursion of the variadic template.  */
    void inheritRelationsFrom() { }
    /** Empty method to stop the recursion of the variadic template.  */
    void inheritRelationsTo() { }

    /** The array we use as input. */
    StoreArray<StoredClass>* m_set = nullptr;
    /** The array we create. */
    StoreArray<StoredClass>* m_subset = nullptr;
    /** Flags used for m_subset. */
    DataStore::EStoreFlags m_subsetFlags = DataStore::c_WriteOut;
  };

  template < class StoredClass>
  std::map<int, int>
  SelectSubset< StoredClass >::copySetWithRelations(std::function<bool (const StoredClass*)> f)
  {
    std::map<int, int> oldToNew;
    for (const StoredClass& setObject : *m_set) {
      if (!f(&setObject))
        continue;

      oldToNew[setObject.getArrayIndex()] = m_subset->getEntries();
      const StoredClass* subsetObject = m_subset->appendNew(setObject);
      if (!m_reduceExistingSet)
        setObject.addRelationTo(subsetObject);
    }


    //TODO this is the slow bit, can probably be improved by directly dealing with indices
    for (const auto& oldToNewPair : oldToNew) {
      const StoredClass* setObject = (*m_set)[oldToNewPair.first];
      const StoredClass* subsetObject = (*m_subset)[oldToNewPair.second];

      for (std::string fromArray : m_inheritFromArrays) {
        const RelationVector<RelationsObject>& relations = setObject->template getRelationsFrom<RelationsObject>(fromArray);
        for (unsigned int iRel = 0; iRel < relations.size(); iRel++) {
          relations.object(iRel)->addRelationTo(subsetObject, relations.weight(iRel));
        }
      }
      for (std::string toArray : m_inheritToArrays) {
        const RelationVector<RelationsObject>& relations = setObject->template getRelationsTo<RelationsObject>(toArray);
        for (unsigned int iRel = 0; iRel < relations.size(); iRel++) {
          subsetObject->addRelationTo(relations.object(iRel), relations.weight(iRel));
        }
      }
    }

    return oldToNew;
  }


  template < class StoredClass>
  void
  SelectSubset< StoredClass >::copyRelationsToSelf()
  {
    for (const StoredClass& subsetObject1 : *m_subset) {
      //TODO: change relation direction to set -> subset?
      const StoredClass* setObject1 = subsetObject1.template getRelatedFrom<StoredClass>(m_set->getName());

      //get all objects in original set related to setObject1
      const RelationVector<StoredClass>& relations = setObject1->template getRelationsTo<StoredClass>(m_set->getName());
      for (unsigned int iRel = 0; iRel < relations.size(); iRel++) {
        const StoredClass* setObject2 = relations.object(iRel);
        const double weight = relations.weight(iRel);

        //if setObject2 was selected into subset, inherit relation
        const StoredClass* subsetObject2 = setObject2->template getRelatedTo<StoredClass>(m_subset->getName());
        if (subsetObject2) {
          subsetObject1.addRelationTo(subsetObject2, weight);
        }
      }
    }
  }

  template < class StoredClass>
  void
  SelectSubset< StoredClass >::select(const std::function<bool (const StoredClass*)>& f)
  {
    copySetWithRelations(f);

    if (m_inheritToSelf) {
      copyRelationsToSelf();
    }

    if (m_reduceExistingSet) {
      swapSetsAndDestroyOriginal();
    }
  }
}
