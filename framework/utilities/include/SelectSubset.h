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
#include <framework/logging/Logger.h>

#include <string>
#include <list>
#include <unordered_map>
#include <functional>


using namespace Belle2;
using namespace std;

/** Class to create a subset of a given StoreArray together with the relations with other StoreArrays.
 *
 * The class SelectSubset selects a subset of objects contained in a given StoreArray
 * creating at the same time a set of relations with objects contained in other StoreArrays
 * that are the natural restrictions to the subset of the relations from or to the original
 * one.
 *
 *  <h1>Creating a subset</h1>
 *  Assuming you have a StoreArray called 'particles' that contains objects of type Particle
 *  you can use SelectSubset to select particles with a given feature and put them
 *  in another StoreArray called 'oddParticles'.
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
 StoreArray< Particle >::required( "particles" ); // or optional. It is up to you.
 m_selector.registerSubset( set, "oddParticles");

 \endcode

 * The SelectSubset class will take care of creating the new StoreArray<Particle>,
  * register it into the datastore with name "oddParticles" same durability and
 * same persistent attribute of the original one.
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

  m_selector.Select( MySelectionFunction );
 \endcode

 * <h3> C++ lambda function </h3>
 * You can specify a lambda expression as parameter of the Select method. E.g.:
 * in the event method of your code:
 *  \code
m_selector.Select( []( const Particle * particle )
 { return  ( particle->UniqueId() % 2 ) == 1 ; } );
 \endcode
 * with the advantage of an easy capture of module parameters.
 * E.g. to count on the fly the number of rejected particles:
 *  \code
int rejected(0);
m_selector.Select(
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
 * <h2> Relations from the StoreArray to himself</h2>
 * In some case the original StoreArray can have relation with himself
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

  string m_setName;
  DataStore::EDurability m_setDurability;
  DataStore::EDurability m_subsetDurability;
  //  AccessorParams m_subsetAccessorParams;

  string m_subsetName;
  string m_subsetToSetRelationName;

  list< pair< pair<string, string>, AccessorParams > > m_fromSubsetToOtherNames;
  list< pair< pair<string, string>, AccessorParams > > m_fromOtherToSubsetNames;
  list< pair<string, string > > m_fromSubsetToSubsetNames;

  const bool m_reportErrorIfExisting = true;
public:
  /** Constructor */
  SelectSubset() {};

  /** Destructor */
  ~SelectSubset() {};

  /** Register the StoreArray<StoredClass> that will contain the subset of selected elements
   *  @param set         The StoreArray<StoredClass> from which the elements will be selected
   *  @param subsetName  The name of the StoreArray<StoredClass> that will contain the selected elements
   *  @param subsetToSetRelationName The optional name of the relation from the subset to the set.
   */
  bool registerSubset(const StoreArray< StoredClass >& set, const std::string& subsetName,
                      const std::string& subsetToSetRelationName = std::string("")) {

    m_setName                 = set.getName() ;
    m_setDurability           = set.getDurability() ;
    m_subsetDurability        = m_setDurability ;
    m_subsetName              = subsetName ;
    m_subsetToSetRelationName = subsetToSetRelationName != "" ?
                                subsetToSetRelationName : DataStore::relationName(m_subsetName, m_setName);


    bool set_is_transient = DataStore::Instance().getEntry(set)->isTransient;

    StoreArray< StoredClass > subset(subsetName, m_subsetDurability);

    if (set_is_transient) {
      subset.registerAsTransient(m_reportErrorIfExisting);
      RelationArray relation(subset, set, subsetToSetRelationName, m_subsetDurability);
      relation.registerAsTransient(m_reportErrorIfExisting);
    } else {
      subset.registerAsPersistent(m_reportErrorIfExisting);
      RelationArray relation(subset, set, subsetToSetRelationName, m_subsetDurability);
      relation.registerAsPersistent(m_reportErrorIfExisting);
    }

    //    m_subsetAccessorParams = subset.getAccessorParams();
    return true;

  }

  /** Register the Relation from the selected subset to another StoreArray.
   *  @param array       The StoreArray<StoredClass> to which the relation is pointing.
   *  @param setRelationName  The name of the relation from the original set.
   *  @param subsetRelationName The name of the relation from the subset.
   */

  template<class RelationHead, class ... RelationTail >
  bool registerRelationsFromSubsetToOther(const StoreArray<RelationHead>& array,
                                          const std::string& setRelationName,
                                          const std::string& subsetRelationName,
                                          RelationTail... relations) {

    StoreArray< StoredClass > set(m_setName, array.getDurability());
    RelationArray relationFromSetTo(set, array, setRelationName, array.getDurability());
    relationFromSetTo.isRequired();

    StoreArray< StoredClass > subset(m_subsetName, array.getDurability());
    RelationArray relationFromSubsetTo(subset, array, subsetRelationName, array.getDurability());

    bool array_is_transient = DataStore::Instance().getEntry(array)->isTransient;
    if (array_is_transient)
      relationFromSubsetTo.registerAsTransient(m_reportErrorIfExisting);
    else
      relationFromSubsetTo.registerAsPersistent(m_reportErrorIfExisting);

    m_fromSubsetToOtherNames.push_back({
      { relationFromSetTo.getName() , relationFromSubsetTo.getName() },
      relationFromSetTo.getToAccessorParams()
    });

    return registerRelationsFromSubsetToOther(relations ...);
  }


  /** Register the Relation from another StoreArray to the selected subset.
   *  @param array       The StoreArray<StoredClass> from which the relation is pointing.
   *  @param setRelationName  The name of the relation to the original set.
   *  @param subsetRelationName The name of the relation to the subset.
   */

  template<class RelationHead, class ... RelationTail >
  bool registerRelationsFromOtherToSubset(const StoreArray<RelationHead>& array,
                                          const std::string& setRelationName,
                                          const std::string& subsetRelationName,
                                          RelationTail... relations) {

    StoreArray< StoredClass > set(m_setName, array.getDurability());
    RelationArray relationFromArrayToSet(array, set, setRelationName, array.getDurability());
    relationFromArrayToSet.isRequired();

    StoreArray< StoredClass > subset(m_subsetName, array.getDurability());

    RelationArray relationFromArrayToSubset(array, subset, subsetRelationName, array.getDurability());

    bool array_is_transient = DataStore::Instance().getEntry(array)->isTransient;
    if (array_is_transient)
      relationFromArrayToSubset.registerAsTransient(m_reportErrorIfExisting);
    else
      relationFromArrayToSubset.registerAsPersistent(m_reportErrorIfExisting);

    m_fromOtherToSubsetNames.push_back({ { relationFromArrayToSet.getName() , relationFromArrayToSubset.getName() },
      relationFromArrayToSubset.getFromAccessorParams()
    });

    return registerRelationsFromOtherToSubset(relations ...);
  }


  /** Register the Relation from the selected subset to himself.
   *  @param setRelationName  The name of the relation from the original set to himself.
   *  @param subsetRelationName The name of the relation from the subset to himself.
   */

  template<class ... RelationTail >
  bool registerRelationsFromSubsetToSubset(const std::string& setRelationName,
                                           const std::string& subsetRelationName,
                                           RelationTail... relations) {

    StoreArray< StoredClass > set(m_setName, m_setDurability);
    RelationArray relationFromSetToSet(set, set, setRelationName, m_setDurability);
    relationFromSetToSet.isRequired();

    StoreArray< StoredClass > subset(m_subsetName, m_setDurability);
    RelationArray relationFromSubsetToSubset(subset, subset, subsetRelationName,
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

  bool registerRelationsFromOtherToSubset(void) { return true; }
  bool registerRelationsFromSubsetToOther(void) { return true; }
  bool registerRelationsFromSubsetToSubset(void) { return true; }

  void Select(std::function<bool (const StoredClass*)> f);

};


template < class StoredClass>
void
SelectSubset< StoredClass >::Select(std::function<bool (const StoredClass*)> f)
{

  StoreArray<StoredClass> subset(m_subsetName);
  StoreArray<StoredClass> set(m_setName);
  subset.create();

  RelationArray subsetToSetRelation
  (m_subsetToSetRelationName);

  typedef RelationElement::index_type index_type;

  unordered_map< index_type, index_type> setToSubset(set.getEntries());
  unordered_map< index_type, index_type> subsetToSet(set.getEntries());

  for (index_type indexInSet(0); indexInSet < (index_type) set.getEntries(); indexInSet++)
    if (f(set[indexInSet])) {
      index_type indexInSubset(subset.getEntries());
      subsetToSetRelation.add(indexInSubset, indexInSet);
      setToSubset[ indexInSet    ] = indexInSubset;
      subsetToSet[ indexInSubset ] = indexInSet;
      subset.appendNew(* set[indexInSet]);
    }

  // Restrict the domain set relations from the subset To Others
  for (auto relationName : m_fromSubsetToOtherNames) {
    RelationArray setRelationTo(relationName.first.first);
    StoreArray< TObject > fakeTo(relationName.second.first, relationName.second.second);
    //    RelationArray subsetRelationTo( relationName.first.second );
    RelationArray subsetRelationTo(subset, fakeTo, relationName.first.second);


    subsetRelationTo.create();

    for (int arrowIndex = 0; arrowIndex < setRelationTo.getEntries(); arrowIndex ++) {
      const RelationElement& relationElement(setRelationTo[ arrowIndex ]);
      if (setToSubset.find(relationElement.getFromIndex()) != setToSubset.end())
        subsetRelationTo.add(setToSubset[ relationElement.getFromIndex() ],
                             relationElement.getToIndices(),
                             relationElement.getWeights());
    }

  }

  // Restrict the image of the relations From others to the subset
  for (auto relationName : m_fromOtherToSubsetNames) {
    RelationArray setRelationFrom(relationName.first.first);
    StoreArray< TObject > fakeFrom(relationName.second.first, relationName.second.second);

    RelationArray subsetRelationFrom(fakeFrom, subset, relationName.first.second);

    for (int arrowIndex = 0; arrowIndex < setRelationFrom.getEntries(); arrowIndex ++) {
      const RelationElement& setRelationElement(setRelationFrom[ arrowIndex ]);

      for (size_t n = 0 ; n < setRelationElement.getSize() ; n++)
        if (setToSubset.find(setRelationElement.getToIndex(n)) != setToSubset.end()) {
          subsetRelationFrom.add(setRelationElement.getFromIndex(),
                                 setToSubset[ setRelationElement.getToIndex(n) ],
                                 setRelationElement.getWeight(n));
        }
    }

  }

  // Restrict the image of the relations From set to the subset
  for (auto relationName : m_fromSubsetToSubsetNames) {
    RelationArray setRelation(relationName.first);
    RelationArray subsetRelation(subset, subset, relationName.second);


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

};




#endif

