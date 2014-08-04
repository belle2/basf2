/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni ( INFN & University of Pisa )            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SELECT_SUBSET_HH
#define SELECT_SUBSET_HH


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>

#include <string>
#include <list>
#include <unordered_map>
#include <functional>

#include <TObject.h>


namespace Belle2 {
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
   *  First you need to instantiate a SelectSubset object in your module:
   *  \code
   SelectSubset< Particle > m_selector;
   \endcode
   *
   *  <h2> Initialization </h2>
   * In the initialize method of your module you have to initialize the SelectSubset
   * \code

   StoreArray< Particle > set( "particles" );
   StoreArray< Particle >::required( "particles" ); // or optional. The choice is up to you.
   m_selector.registerSubset( set, "oddParticles");

   \endcode

   * The SelectSubset class will take care of creating the new StoreArray<Particle>,
   * register it into the datastore with name "oddParticles" same durability and
   * same persistent attributes of the original one.
   *
   * <h2> Selection </h2>
   *
   * To create the subset you have to specify the selection criterium.
   * You can do that in two possible ways.
   * <h3> C++ function </h3>
   * You can define your C++ function and then use it. E.g:
   *  \code
   bool
   MySelectionFunction( const Particle * particle){
  // this function tells if the particle is odd
  return ( particle->UniqueId() % 2 ) == 1 ;
  }
  \endcode
  Then in the event method of your module:
  \code

  m_selector.select( MySelectionFunction );
  \endcode

   * <h3> C++ lambda function </h3>
   * You can specify a lambda expression as parameter of the select method. E.g.:
   * in the event method of your code:
   *  \code
   m_selector.select( []( const Particle * particle )
   { return  ( particle->UniqueId() % 2 ) == 1 ; } );
   \endcode
   * with the advantage of an easy capture of module parameters.
   * E.g. to count on the fly the number of rejected particles:
   *  \code
   int rejected(0);
   m_selector.select(
   [& rejected]( const Particle * particle )
   {
   if ( ( particle->UniqueId() % 2 ) == 1 )
   return true;
   rejected ++;
   return false;
   }
   );
  B2INFO("The selector rejected " << rejected << " particles." );
  \endcode
    * <h1> Relations </h1>
    * By default the class SelectSubset produces a one to one relation
    * from the subset to the set by which you can interpret all the relations
    * from and to the original set. E.g. The original StoreArray<Particle> is
    * in relation To the StoreArray<MCParticle>. You can use the relation from
    * the subset to the set and then from the set to the MCParticles. This can
    * be quite tedious, so you can ask SelectSubset to produce the natural
    * restrictions of the relations  from and to the original set.

    * <h2> Relations to other StoreArrays</h2>
    * Assuming that the StoreArray<Particle> has relations To others
    * StoreArrays of Something and SomethingElse you can tell SelectSubset to produce
    * a new Relation from the subset to the other StoreArrays by
    * the following code in your implementation of the Module::initialize() method.
    * \code
    void MyModule::initialize(){
      StoreArray<Something>     StoreArrayOfSomething;
      StoreArray<SomethingElse> StoreArrayOfSomethingElse;
      StoreArray<Stuff>         StoreArrayOfStuff;

      m_selector.registerRelationsFromSubsetToOther( StoreArrayOfSomething, "", "",
          StoreArrayOfSomethingElse, "", "");

      // Or you can add the relations one by one
      m_selector.registerRelationsFromSubsetToOther( StoreArrayOfStuff, "", "");

      // Or you can use non default relation name. DEPRECATED!
      m_selector.registerRelationsFromSubsetToOther( StoreArrayOfSomethingElse,
          "NonDefaultRelationName", "AnotherCrypticalName",);

      \endcode
        * That's it.

        * <h2> Relations from other StoreArrays</h2>
        * Assuming that the StoreArray<Particle> has relations From others
        * StoreArrays of Something and SomethingElse you can tell SelectSubset to produce
        * a new Relation to the subset from the other StoreArrays by
        * the following code in your implementation of the Module::initialize() method.
        * \code
        void MyModule::initialize(){
          StoreArray<Something>     StoreArrayOfSomething;
          StoreArray<SomethingElse> StoreArrayOfSomethingElse;
          StoreArray<Stuff>         StoreArrayOfStuff;

          m_selector.registerRelationsFromOtherToSubset( StoreArrayOfSomething, "", "",
              StoreArrayOfSomethingElse, "", "");

          // Or you can add the relations one by one
          m_selector.registerRelationsFromOtherToSubset( StoreArrayOfStuff, "", "");

          // Or you can use non default relation name. DEPRECATED!
          m_selector.registerRelationsFromOtherToSubset( StoreArrayOfSomethingElse,
              "NonDefaultRelationName", "AnotherCrypticalName",);

          \endcode
            * That's it.
            * <h2> Relations from the StoreArray to itself</h2>
            * In some case the original StoreArray can have relation with itself
            * (E.g. MCparticle mother - daughter relation ).
            * In this case you have several options:
            * -  you can restrict the set of "from" elements by using the method
            * registerRelationsFromSubsetToOther and specifying the original set
            * as first argument.
            *
            * -  you can restrict the set of "to" elements by using the method
            * registerRelationsFromOtherToSubset and specifying the original set
            * as first argument.
            *
            * - you can restrict bot the "from" element side and the "to" element
            * side by using:
            * \code
            m_selector.registerRelationsFromSubsetToSubset( "","" );

          // Or you can use non default relation name. DEPRECATED!
          m_selector.registerRelationsFromSubsetToSubset(
              "NonDefaultRelationName", "AnotherCrypticalName",);
          \endcode
            * the choice among the 3 options really depend on the use case, since
            * generally speaking the 3 relations are not at all equivalents.
            */

  template < typename StoredClass >
  class SelectSubset {

    std::string m_setName;
    DataStore::EDurability m_setDurability;

    StoreArray<StoredClass>* m_subset;

    /** array names we inherit relations from. */
    std::vector<std::string> m_inheritFromArrays;
    /** array names we inherit relations to. */
    std::vector<std::string> m_inheritToArrays;


    std::list< std::pair<std::string, std::string > > m_fromSubsetToSubsetNames;

    const bool m_reportErrorIfExisting = true;
  public:
    /** Constructor */
    SelectSubset():
      m_setDurability(DataStore::c_Event), m_subset(nullptr)
    {};

    /** Destructor */
    ~SelectSubset() {};

    /** Register the StoreArray<StoredClass> that will contain the subset of selected elements
     *  @param set         The StoreArray<StoredClass> from which the elements will be selected
     *  @param subsetName  The name of the StoreArray<StoredClass> that will contain the selected elements
     */
    bool registerSubset(const StoreArray< StoredClass >& set, const std::string& subsetName) {
      if (m_subset) {
        B2FATAL("SelectSubset::registerSubset() can only be called once!");
        return false;
      }

      m_setName                 = set.getName() ;
      m_setDurability           = set.getDurability() ;


      bool set_is_transient = DataStore::Instance().getEntry(set)->isTransient;

      m_subset = new StoreArray<StoredClass>(subsetName, m_setDurability);

      if (set_is_transient) {
        m_subset->registerAsTransient(m_reportErrorIfExisting);
        RelationArray relation(*m_subset, set, "", m_subset->getDurability());
        relation.registerAsTransient(m_reportErrorIfExisting);
      } else {
        m_subset->registerAsPersistent(m_reportErrorIfExisting);
        RelationArray relation(*m_subset, set, "", m_subset->getDurability());
        relation.registerAsPersistent(m_reportErrorIfExisting);
      }

      return true;

    }


    /** Register the Relation from the selected subset to itself.
     *  @param setRelationName  The name of the relation from the original set to itself.
     *  @param subsetRelationName The name of the relation from the subset to itself.
     */

    template<class ... RelationTail >
    bool registerRelationsFromSubsetToSubset(const std::string& setRelationName,
                                             const std::string& subsetRelationName,
                                             RelationTail... relations) {

      StoreArray< StoredClass > set(m_setName, m_setDurability);
      RelationArray relationFromSetToSet(set, set, setRelationName, m_setDurability);
      relationFromSetToSet.isRequired();

      RelationArray relationFromSubsetToSubset(*m_subset, *m_subset, subsetRelationName,
                                               m_setDurability);

      bool array_is_transient = DataStore::Instance().getEntry(set)->isTransient;
      if (array_is_transient)
        relationFromSubsetToSubset.registerAsTransient(m_reportErrorIfExisting);
      else
        relationFromSubsetToSubset.registerAsPersistent(m_reportErrorIfExisting);

      m_fromSubsetToSubsetNames.push_back({ relationFromSetToSet.getName() ,
                                            relationFromSubsetToSubset.getName()
                                          });

      return registerRelationsFromSubsetToSubset(relations ...);
    }

    /** Inherit relations pointing from Other to objects selected into this subset.
     *
     * You can specify an unlimited number of arrays as arguments to this function.
     */
    template<class T, class ... MoreArguments >
    void inheritRelationsFrom(const StoreArray<T>& array, MoreArguments... moreArgs) {

      const_cast<StoreArray<T>&>(array).isRequired();

      RelationArray relation(array, *m_subset, "", m_subset->getDurability());
      if (m_subset->isTransient() or array.isTransient())
        relation.registerAsTransient(m_reportErrorIfExisting);
      else
        relation.registerAsPersistent(m_reportErrorIfExisting);

      m_inheritFromArrays.push_back(array.getName());

      inheritRelationsFrom(moreArgs ...);
    }

    /** Inherit relations pointing from objects selected into this subset to Other.
     *
     * You can specify an unlimited number of arrays as arguments to this function.
     */
    template<class T, class ... MoreArguments >
    void inheritRelationsTo(const StoreArray<T>& array, MoreArguments... moreArgs) {

      const_cast<StoreArray<T>&>(array).isRequired();

      RelationArray relation(*m_subset, array, "", m_subset->getDurability());
      if (m_subset->isTransient() or array.isTransient())
        relation.registerAsTransient(m_reportErrorIfExisting);
      else
        relation.registerAsPersistent(m_reportErrorIfExisting);

      m_inheritToArrays.push_back(array.getName());

      inheritRelationsTo(moreArgs ...);
    }

    /** This method is the actual worker. It selects the elements, fill the subset and
     * all the relations in which the subset is involved.
     *  @param f the pointer to the function (or a nameless lambda expression) returning
     *  true for the elements to be selected and false for the others.
     */

    void select(std::function<bool (const StoredClass*)> f);

  private:
    /** Empty method to stop the recursion of the variadic template.
    */
    void inheritRelationsFrom() { }
    /** Empty method to stop the recursion of the variadic template.
    */
    void inheritRelationsTo() { }

    /** Empty method to stop the recursion of the variadic template.
    */
    bool registerRelationsFromSubsetToSubset(void) { return true; }


  };


  template < class StoredClass>
  void
  SelectSubset< StoredClass >::select(std::function<bool (const StoredClass*)> f)
  {

    StoreArray<StoredClass> set(m_setName);

    RelationArray subsetToSetRelation(*m_subset, set, "", m_subset->getDurability());

    typedef RelationElement::index_type index_type;

    std::unordered_map< index_type, index_type> setToSubset(set.getEntries());
    std::unordered_map< index_type, index_type> subsetToSet(set.getEntries());


    for (index_type indexInSet(0); indexInSet < (index_type) set.getEntries(); indexInSet++) {
      const StoredClass* setObject = set[indexInSet];
      if (f(setObject)) {
        index_type indexInSubset(m_subset->getEntries());
        setToSubset[ indexInSet    ] = indexInSubset;
        subsetToSet[ indexInSubset ] = indexInSet;
        subsetToSetRelation.add(indexInSubset, indexInSet);
        const StoredClass* subsetObject = m_subset->appendNew(*setObject);
        for (std::string fromArray : m_inheritFromArrays) {
          const RelationVector<TObject>& relations = setObject->template getRelationsFrom<TObject>(fromArray);
          for (int iRel = 0; iRel < relations.size(); iRel++) {
            //TODO this might be slow, but members of other array might not inherit from RelationsObject. Once genfit::track is fixed,
            //this should be changed into RelationsObject members.
            DataStore::addRelationFromTo(relations.object(iRel), subsetObject, relations.weight(iRel));
          }
        }
        for (std::string toArray : m_inheritToArrays) {
          const RelationVector<TObject>& relations = setObject->template getRelationsTo<TObject>(toArray);
          for (int iRel = 0; iRel < relations.size(); iRel++) {
            subsetObject->addRelationTo(relations.object(iRel), relations.weight(iRel));
          }
        }

      }
    }


    // Restrict the image of the relations From set to the subset
    for (auto relationName : m_fromSubsetToSubsetNames) {
      RelationArray setRelation(relationName.first);
      RelationArray subsetRelation(*m_subset, *m_subset, relationName.second);


      for (int arrowIndex = 0; arrowIndex < setRelation.getEntries(); arrowIndex ++) {
        const RelationElement& setRelationElement(setRelation[ arrowIndex ]);

        if (setToSubset.find(setRelationElement.getFromIndex()) == setToSubset.end())
          continue;
        for (size_t n = 0 ; n < setRelationElement.getSize() ; n++)
          if (setToSubset.find(setRelationElement.getToIndex(n)) !=  setToSubset.end())
            subsetRelation.add(setToSubset[ setRelationElement.getFromIndex() ],
                               setToSubset[ setRelationElement.getToIndex(n) ],
                               setRelationElement.getWeight(n));

      }

    }

  }



}

#endif

