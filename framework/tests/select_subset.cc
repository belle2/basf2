#include <framework/datastore/SelectSubset.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/RelationArray.h>


#include <gtest/gtest.h>
#include <TObject.h>

#include <string>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm>

using namespace std;
using namespace Belle2;


namespace {
  /** Set up a few arrays and objects in the datastore */
  class SelectSubsetTest : public ::testing::Test {

  public:

    typedef UInt_t KeyElementType;
    typedef RelationsObject StoredElement;


    static bool SelectionCriterium(const KeyElementType a)
    {
      // The elements  whose UniqueID is prime are selected (well, if the UniqueID
      // is equal or greather than 1369 the sieve is too coarse)
      return (a % 2) != 0 && (a % 3) != 0 && (a % 5) != 0 && (a % 7) != 0 &&
             (a % 11) != 0 && (a % 13) != 0 && (a % 17) != 0 && (a % 19) != 0 && (a % 23) != 0 &&
             (a % 29) != 0 && (a % 31) != 0 ;
    }

    static bool SelectionCriteriumOnElement(const StoredElement*   a)
    {
      return  SelectionCriterium(a->GetUniqueID());
    }

    // The following map type is ment as a container of all the relations
    // from m_fromElement to al other elements in all the store arrays.
    // It maps the "To" KeyElement to the weight of Relation.
    typedef map< KeyElementType , double > FromTargetElementsToWeight;


    class Relations {

      // This class is a container of all the
      // relations from m_elementKey to all other elements
      // in all the store arrays of the TestBench

    public:

      // the following map is ment to map the name of StoreArray to the
      // map linking the target "to" element to the weight of the relation.
      typedef unordered_map< string, FromTargetElementsToWeight > FromStringToArrows;

    private:
      KeyElementType m_fromElement;
      FromStringToArrows m_allRelations;

    public:

      // constructor
      explicit Relations(const KeyElementType& from):
        m_fromElement(from)
      {
      }

      const FromStringToArrows&
      getConstSetOfAllRelations(void) const
      {
        return m_allRelations;
      }

      FromStringToArrows::const_iterator
      getConstSetOfToRelations(const string& toSetName) const
      {
        return m_allRelations.find(toSetName) ;
      }


      FromStringToArrows::iterator
      getSetOfToRelations(string toSetName)
      {
        return m_allRelations.find(toSetName) ;
      }

      FromStringToArrows::const_iterator
      getSetOfToRelations(string toSetName) const
      {
        return m_allRelations.find(toSetName) ;
      }


      void
      print(void)
      {
        cout << "From KeyElement " << m_fromElement << "to:" << endl;
        for (auto toSet : m_allRelations) {
          for (auto pair : toSet.second)
            cout << "Set name: "
                 << toSet.first << "  "
                 << pair.first
                 << " ( " << pair.second << " ) " << endl;

          cout << "-----------------------------------------------------------------" << endl;
        }
      }

      bool
      isPresentRelationTo(const string& toSetName, KeyElementType to) const
      {
        auto toRelations = getConstSetOfToRelations(toSetName);
        if (toRelations == m_allRelations.end())
          return false;
        return toRelations->second.find(to) != toRelations->second.end();
      }

      void
      appendNewRelationTo(const string& toName, KeyElementType to,
                          double weight)
      {
        auto toRelations = getSetOfToRelations(toName);
        if (toRelations == m_allRelations.end()) {
          // add the new named set to the relation
          m_allRelations.insert(pair <string, FromTargetElementsToWeight>
                                (toName,
          FromTargetElementsToWeight({ {to, weight} })));
          return;
        }
        pair< KeyElementType, double > relation(to, weight);
        toRelations->second.insert(relation);

      }

      const FromTargetElementsToWeight&
      getAllRelations(const  string& toOtherSetName) const
      {
        static FromTargetElementsToWeight nothing;
        auto toRelations = getSetOfToRelations(toOtherSetName);
        if (toRelations == m_allRelations.end()) {
          return nothing;
        };
        return toRelations->second;
      }

    };

//disable warnings about unused functions
#if defined(__INTEL_COMPILER)
#pragma warning disable 177
#endif

    class NamedSet {

      // The class NamedSet provides two representantions of the same entity.
      // The entity has a name, a set, and some relations with other sets.
      // The entity name is m_name.
      // The set is stored both as a StoreArray named m_name and as an unordered_map
      // The relations are stored both as relation and as unordered_map so that
      // one can check the correctness of the SelectSubset utility.

      string m_name;                                    // My name
      map< KeyElementType, Relations > m_set;           // What I am
      StoreArray< StoredElement > m_storeArray;         // How I am stored
      RelationArray* to;    // How my relation to the original set is stored
      RelationArray* from;  // How my relation from the original set is stored
      RelationArray* self;  // How the relation with myself is stored (yes, I am
      // the original set)
    public:


      void print(void)
      {
        cout << "The NamedSet: "  << m_name << " contains:" << endl ;
        for (auto set : m_set) {
          cout << set.first << endl;
          set.second.print();
        }
        cout << "~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      }

      // constructor
      explicit NamedSet(const string& name = "") :
        m_name(name), m_storeArray(name),
        to(NULL), from(NULL), self(NULL)
      {
      }

      // order relation
      bool operator()(const NamedSet& a, const NamedSet& b) const
      {
        return a.getName() < b.getName();
      }

      // accessors
      const string getName(void) const
      {
        return m_name;
      }

      StoreArray< StoredElement >& storedArray(void)
      {
        return m_storeArray;
      }


      void initializeDatastore(void)
      {
        // this method take care of the initialization of the
        // StoreArray
        DataStore::Instance().setInitializeActive(true);

        StoreArray< StoredElement > array(getName());
        array.registerInDataStore();
        array.registerRelationTo(array);

        DataStore::Instance().setInitializeActive(false);
      }

      void initializeDatastore(string SetName)
      {
        // this method take care of the initialization of the
        // store array and of the RelationArrays from and to
        // the StoreArray named SetName

        DataStore::Instance().setInitializeActive(true);

        StoreArray< StoredElement > array(getName());
        StoreArray< StoredElement > otherSet(SetName);
        array.registerInDataStore();
        array.registerRelationTo(otherSet);
        otherSet.registerRelationTo(array);
        //TODO one of these is wrong! -> getRelationsWith() would return both directions

        DataStore::Instance().setInitializeActive(false);
      }

      void consolidate()
      {
        // this method consolidate the StoreArray and the RelationArrays
        // to and from the set
        if (self != NULL)
          self->consolidate();
        if (from != NULL)
          from->consolidate();
        if (to != NULL)
          to->consolidate();
      }


      void appendNewElement(KeyElementType element)
      {
        m_set.insert(pair< KeyElementType, Relations > (element, Relations(element)));
      }

      void appendNewRelationTo(KeyElementType fromKey, const string& toName, KeyElementType toKey,
                               double weight)
      {
        auto fromElement = m_set.find(fromKey);
        if (fromElement == m_set.end()) {
          cout << "??" << endl;
          return; // quietly skip
        }
        fromElement->second.appendNewRelationTo(toName, toKey, weight);
      }

      bool isPresentRelationFromTo(const KeyElementType& fromKey, const string& otherSetName ,
                                   const KeyElementType& toKey) const
      {
        auto fromElement = m_set.find(fromKey);
        if (fromElement != m_set.end()) {
          return fromElement->second.isPresentRelationTo(otherSetName, toKey);
        } else {
          static int i(0);
          if (i ++ < 999 || (i % 100) == 0) {
            cout << "Error: from: " << getName() << " id " <<  fromKey  <<
                 " -> " << otherSetName << " id " << toKey << endl;
            for (auto element : m_set)
              cout << element.first << "\t";
            cout << endl;
            for (auto element : m_storeArray)
              cout << element.GetUniqueID() << "\t";
            cout << endl;
          } else if (i == 1000) {
            cout << "Skipping 99% of the following errors" << endl;
          }
          return false;
        }
      }

      const FromTargetElementsToWeight& getAllRelations(const KeyElementType& fromKey, const string& toOtherSetName) const
      {
        auto fromElement = m_set.find(fromKey);
        return fromElement->second.getAllRelations(toOtherSetName);
      }

      typedef map< KeyElementType , FromTargetElementsToWeight > StlRelationArray;

      StlRelationArray getRestrictedDomainRelationTo(const string& toOtherSetName)const
      {
        StlRelationArray theInducedRelation;
        for (auto element : m_set) {
          if (SelectionCriterium(element.first) &&
              element.second.getAllRelations(toOtherSetName).size() != 0)
            theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                      (element.first, element.second.getAllRelations(toOtherSetName)));
        }
        return theInducedRelation;
      }

      StlRelationArray getRestrictedCodomainRelationTo(const string& setName)const
      {
        StlRelationArray theInducedRelation;
        for (auto element : m_set) {
          if (element.second.getConstSetOfToRelations(setName) ==
              element.second.getConstSetOfAllRelations().end())
            continue;
          for (auto image : element.second.getConstSetOfToRelations(setName)->second) {
            if (SelectionCriterium(image.first)) {
              if (theInducedRelation.find(element.first) == theInducedRelation.end())
                theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                          (element.first, FromTargetElementsToWeight()));
              theInducedRelation.find(element.first)->second.
              insert(FromTargetElementsToWeight::value_type
                     (image.first, image.second));
            }
          }
        }
        return theInducedRelation;
      }

      StlRelationArray getRestrictedSelfRelation(void) const
      {
        StlRelationArray theInducedRelation;
        for (auto element : m_set) {
          if (! SelectionCriterium(element.first) ||
              element.second.getAllRelations(m_name).size() == 0)
            continue;
          for (auto image : element.second.getConstSetOfToRelations(m_name)->second) {
            if (SelectionCriterium(image.first)) {
              if (theInducedRelation.find(element.first) == theInducedRelation.end())
                theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                          (element.first, FromTargetElementsToWeight()));
              theInducedRelation.find(element.first)->second.
              insert(FromTargetElementsToWeight::value_type
                     (image.first, image.second));
            }
          }
        }
        return theInducedRelation;

      }
    };

    class CollectionOfSets {

      // This is the actual test bench. It contains the set to be subsetted
      // together with the subset and the other sets in relation with the subset
    private:
      string m_setName;     // We start from this set
      string m_subsetName;  // we will select this subset
      NamedSet m_set;
      NamedSet m_subset;

      vector< string > m_otherSetsNames; // other sets in the universe
      unordered_map< string, NamedSet > m_otherSets;

    public:
      // constructor
      CollectionOfSets(): m_set("theSet"),
        m_subset("theSubset"),
        m_otherSetsNames {string("G"), string("F"), string("E"),
                          string("D"), string("C"), string("B"), string("A")
                         }
      {
        m_setName = m_set.getName();
        m_subsetName = m_subset.getName();
        for (auto aSetName : m_otherSetsNames)
          m_otherSets.insert(pair<string, NamedSet> (aSetName, NamedSet(aSetName)));
      };

      //accessors
      const string&
      getSetName(void) const
      {
        return m_setName    ;
      };

      StoreArray< StoredElement >&
      getSet(void)
      {
        return m_set.storedArray();
      };

      NamedSet&
      getNamedSet(void)
      {
        return m_set;
      };


      const string&
      getSubsetName(void) const
      {
        return m_subsetName ;
      };

      StoreArray< StoredElement >&
      getSubset(void)
      {
        return m_subset.storedArray();
      };

      int
      getNSets(void) const
      {
        return m_otherSets.size() ;
      }

      const vector< string >&
      getOtherSetsNames(void) const
      {
        return m_otherSetsNames ;
      }

      const string&
      getOtherSetsNames(int i) const
      {
        return m_otherSetsNames[i] ;
      }

      StoreArray< StoredElement >&
      getOtherSet(int i)
      {
        auto  otherSet = m_otherSets.find(m_otherSetsNames[ i ]);
        if (otherSet != m_otherSets.end())
          return otherSet->second.storedArray();
        else
          cout << " ???????????? " << i << endl;
        return otherSet->second.storedArray();
      }

      pair< const string, NamedSet>&
      getOtherNamedSet(int i)
      {
        return * m_otherSets.find(m_otherSetsNames[ i ]);
      }

      unordered_map< string, NamedSet >&
      getOtherSets(void)
      {
        return m_otherSets;
      }

      void
      initializeDatastore(void)
      {
        // initialitation phase

        m_set.initializeDatastore();

        for (auto namedSet : m_otherSets)
          namedSet.second.initializeDatastore(getSetName());

      }

      void
      populateDatastore(void)
      {
        unsigned int nElements = 1368;
        for (unsigned int i = 0; i < nElements ; i++)
          appendNewElement(m_set);

        int n(0);

        for (auto iterator = m_otherSets.begin() ; iterator != m_otherSets.end(); iterator++) {
          unsigned int nOtherElements = nElements - n * 100;

          for (unsigned int i = 0; i < nOtherElements ; i++)
            appendNewElement(iterator->second);

        }

        int j(2);
        for (auto iterator = m_otherSets.begin() ; iterator != m_otherSets.end(); iterator++) {


          unsigned int nArrows = nElements * j;
          j++;

          for (unsigned int arrow = 0; arrow < nArrows ; arrow ++) {
            appendNewRelationToOther(*iterator);
            appendNewRelationFromOther(*iterator);
          }

        }


        for (unsigned int arrow = 0; arrow < 5 * nElements; arrow ++)
          appendNewSelfRelation();

#ifdef DEBUG
        m_set.print();
#endif

      }


      double
      getWeight(void)
      {
        static double counter(0.0);
        return counter += 1.;
      };

      unsigned int
      flat_random(unsigned int max)
      {
        /* This algorithm is mentioned in the ISO C standard, here extended for 32 bits.
           the initial seed is arbitrary set to 11036
           the seed is 11036 and is fixed i.e. it is strictly reproducible and does
           not interact with other random generators*/
        static unsigned int next = 11036;
        int result;

        next *= 1103515245;
        next += 12345;
        result = (unsigned int)(next / 65536) % 2048;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        return result % max;

      }

      void
      appendNewElement(NamedSet& namedSet)
      {
        StoreArray< StoredElement >&   set = namedSet.storedArray();
        int i = set.getEntries() ;
        static int uniqueId(0);

        namedSet.appendNewElement(uniqueId);
        set.appendNew();
        set[ i ]->SetUniqueID(uniqueId);
        uniqueId++;
      }


      void
      appendNewRelationToOther(pair< const string, NamedSet>& set)
      {

        int from_index = flat_random(getSet().getEntries());
        KeyElementType from_key = getSet()[from_index]->GetUniqueID();
        int to_index = flat_random(set.second.storedArray().getEntries());
        KeyElementType to_key   = set.second.storedArray()[to_index]->GetUniqueID();


        if (getNamedSet().isPresentRelationFromTo(from_key, set.first , to_key))
          return; // Arrow already drawn


        double weight = getWeight();

        getNamedSet().appendNewRelationTo(from_key, set.first, to_key , weight);

        RelationArray setToOtherSet(getSet(), set.second.storedArray());
        setToOtherSet.add(from_index, to_index, weight);

      }


      void
      appendNewRelationFromOther(pair< const string, NamedSet>& otherSet)
      {

        //cout << otherSet.first << "\t";
        unsigned int from_index = flat_random(otherSet.second.storedArray().getEntries());
        //cout << from_index << "\t";
        KeyElementType from_key = otherSet.second.storedArray()[from_index]->GetUniqueID();
        //cout << from_key << endl;
        unsigned int to_index   = flat_random(getSet().getEntries());
        KeyElementType to_key = getSet()[to_index]->GetUniqueID();

        if (otherSet.second.isPresentRelationFromTo(from_key, getSetName(), to_key))

          return; // Arrow already drawn

        double weight = getWeight();

        otherSet.second.appendNewRelationTo(from_key, getSetName(), to_key , weight);

        //otherSet.second.storedArray()[ from_index ]->SetBinContent( 0 , to_index , weight);

        RelationArray otherSetToSet(otherSet.second.storedArray(), getSet());
        otherSetToSet.add(from_index, to_index, weight);

      }

      void
      appendNewSelfRelation(void)
      {
        unsigned int from_index = flat_random(getSet().getEntries());
        KeyElementType from_key = getSet()[from_index]->GetUniqueID();
        unsigned int to_index   = flat_random(getSet().getEntries());
        KeyElementType to_key = getSet()[to_index]->GetUniqueID();


        if (getNamedSet().isPresentRelationFromTo(from_key, getSetName() , to_key))
          return; // Arrow already drawn

        double weight = getWeight();

        getNamedSet().appendNewRelationTo(from_key, getSetName(), to_key , weight);

        RelationArray setToSet(getSet(), getSet());

        setToSet.add(from_index, to_index, weight);


      }


    };


  protected:

    CollectionOfSets m_TestBench;

    virtual void SetUp()
    {
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }


    bool
    testRelationToOther(pair< const string, NamedSet>& otherSet)
    {

      StoreArray< StoredElement > theSet(m_TestBench.getSetName());
      StoreArray< StoredElement > subset(m_TestBench.getSubsetName());
      RelationArray subsetToOtherSet(subset,
                                     otherSet.second.storedArray());


      NamedSet::StlRelationArray theInducedRelation;

      for (int relation = 0 ; relation < subsetToOtherSet.getEntries() ; relation++) {
        size_t relationSize = subsetToOtherSet[ relation ].getSize();

        RelationElement::index_type from = subsetToOtherSet[ relation ].getFromIndex();
        KeyElementType fromElementKey =  subset[from]->GetUniqueID();
        if (theInducedRelation.find(fromElementKey) == theInducedRelation.end())
          theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                    (fromElementKey, FromTargetElementsToWeight()));

        for (unsigned int to_index = 0 ; to_index < relationSize ; to_index++) {
          RelationElement::index_type to = subsetToOtherSet[ relation ].getToIndex(to_index);
          double weight = subsetToOtherSet[ relation ].getWeight(to_index);
          KeyElementType toElementKey =  otherSet.second.storedArray()[to]->GetUniqueID();

          theInducedRelation.find(fromElementKey)->second.insert(pair< KeyElementType, double> (toElementKey , weight));

        }
      }


      NamedSet::StlRelationArray theExpectedRelation = m_TestBench.getNamedSet().
                                                       getRestrictedDomainRelationTo(otherSet.first) ;

      bool OKorKO = equal(theExpectedRelation.begin(), theExpectedRelation.end(), theInducedRelation.begin(),
                          [](const NamedSet::StlRelationArray::value_type &  a,
      const NamedSet::StlRelationArray::value_type &  b) {
#ifdef DEBUG
        cout << a.first << " vs " << b.first << endl;
#endif
        return a.first == b.first &&
               equal(a.second.begin(), a.second.end(), b.second.begin(),
                     [](const FromTargetElementsToWeight::value_type & x,
        const FromTargetElementsToWeight::value_type & y) {
#ifdef DEBUG
          cout
              << x.first << "," << x.second
              << " <-> "
              << y.first << "," << y.second  << "\t";
          cout << (x.first == y.first) << " , " << (x.second == y.second) << endl;
#endif
          return x.first == y.first && x.second == y.second;
        }
                    ) ;
      });

#ifdef DEBUG
      for (auto relation : theExpectedRelation)
        for (auto to : relation.second)
          cout << relation.first << " -> " << to.first << "  ( " << to.second << " )" << endl;

      for (auto relation : theInducedRelation)
        for (auto to : relation.second)
          cout << relation.first << " --> " << to.first << "  ( " << to.second << " )" << endl;
#endif


      return OKorKO;
    }

    bool
    testRelationFromOther(pair< const string, NamedSet>& otherNamedSet)
    {

      StoreArray< StoredElement > subset(m_TestBench.getSubsetName());
      StoreArray< StoredElement > other = otherNamedSet.second.storedArray();


      RelationArray subsetFromOtherSet(otherNamedSet.second.storedArray(), subset);


      NamedSet::StlRelationArray theInducedRelation;

      for (int relation = 0 ; relation < subsetFromOtherSet.getEntries() ; relation++) {
        size_t relationSize = subsetFromOtherSet[ relation ].getSize();
        RelationElement::index_type from = subsetFromOtherSet[ relation ].getFromIndex();
        KeyElementType fromElementKey =  other[from]->GetUniqueID();
        if (theInducedRelation.find(fromElementKey) == theInducedRelation.end()) {
          theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                    (fromElementKey, FromTargetElementsToWeight()));
        }

        for (unsigned int to_index = 0 ; to_index < relationSize ; to_index++) {
          RelationElement::index_type to =
            subsetFromOtherSet[ relation ].getToIndex(to_index);
          double weight = subsetFromOtherSet[ relation ].getWeight(to_index);
          KeyElementType toElementKey =  subset[to]->GetUniqueID();

          theInducedRelation.find(fromElementKey)->second.insert(pair< KeyElementType, double> (toElementKey , weight));

        }
      }


      NamedSet::StlRelationArray theExpectedRelation
        = otherNamedSet.second.getRestrictedCodomainRelationTo(m_TestBench.getSetName());
      bool OKorKO = equal(theExpectedRelation.begin(), theExpectedRelation.end(), theInducedRelation.begin(),
                          [](const NamedSet::StlRelationArray::value_type &  a,
      const NamedSet::StlRelationArray::value_type &  b) {
#ifdef DEBUG
        cout << a.first << " vs " << b.first << endl;
#endif
        return a.first == b.first &&
               equal(a.second.begin(), a.second.end(), b.second.begin(),
                     [](const FromTargetElementsToWeight::value_type & x,
        const FromTargetElementsToWeight::value_type & y) {
#ifdef DEBUG
          cout
              << x.first << "," << x.second
              << " <-> "
              << y.first << "," << y.second  << "\t";
          cout << (x.first == y.first) << " , " << (x.second == y.second) << endl;
#endif
          return  x.first == y.first && x.second == y.second;
        }
                    ) ;
      });

#ifdef DEBUG
      for (auto relation : theExpectedRelation)
        for (auto to : relation.second)
          cout << relation.first << " -> " << to.first << "  ( " << to.second << " )" << endl;

      for (auto relation : theInducedRelation)
        for (auto to : relation.second)
          cout << relation.first << " --> " << to.first << "  ( " << to.second << " )" << endl;
#endif


      return OKorKO;
    }


    bool
    testSelfRelation(void)
    {

      StoreArray< StoredElement > subset(m_TestBench.getSubsetName());
      RelationArray selfRelation(subset, subset);


      NamedSet::StlRelationArray theInducedRelation;

      for (int relation = 0 ; relation < selfRelation.getEntries() ; relation++) {
        size_t relationSize = selfRelation[ relation ].getSize();
        RelationElement::index_type from = selfRelation[ relation ].getFromIndex();
        KeyElementType fromElementKey =  subset[from]->GetUniqueID();
        if (theInducedRelation.find(fromElementKey) == theInducedRelation.end()) {
          theInducedRelation.insert(pair< KeyElementType, FromTargetElementsToWeight>
                                    (fromElementKey, FromTargetElementsToWeight()));
        }

        for (unsigned int to_index = 0 ; to_index < relationSize ; to_index++) {
          RelationElement::index_type to =
            selfRelation[ relation ].getToIndex(to_index);
          double weight = selfRelation[ relation ].getWeight(to_index);
          KeyElementType toElementKey =  subset[to]->GetUniqueID();

          theInducedRelation.find(fromElementKey)->second.insert(pair< KeyElementType, double> (toElementKey , weight));

        }
      }


      NamedSet::StlRelationArray theExpectedRelation = m_TestBench.getNamedSet().
                                                       getRestrictedSelfRelation();

      bool OKorKO = equal(theExpectedRelation.begin(), theExpectedRelation.end(), theInducedRelation.begin(),
                          [](const NamedSet::StlRelationArray::value_type &  a,
      const NamedSet::StlRelationArray::value_type &  b) {

#ifdef DEBUG
        cout << a.first << " vs " << b.first << endl;
#endif
        return (true || a.first == b.first) &&
               equal(a.second.begin(), a.second.end(), b.second.begin(),
                     [](const FromTargetElementsToWeight::value_type & x,
        const FromTargetElementsToWeight::value_type & y) {
#ifdef DEBUG
          cout
              << x.first << "," << x.second
              << " <-> "
              << y.first << "," << y.second  << "\t";
          cout << (x.first == y.first) << " , " << (x.second == y.second) << endl;
#endif
          return x.first == y.first && x.second == y.second;
        }
                    ) ;
      });

#ifdef DEBUG

      cout << "We do expect the following set of relations" << endl;
      for (auto relation : theExpectedRelation)
        for (auto to : relation.second)
          cout << relation.first << " -> " << to.first << "  ( " << to.second << " )" << endl;

      cout << "We found the following ones" << endl;
      for (auto relation : theInducedRelation)
        for (auto to : relation.second)
          cout << relation.first << " --> " << to.first << "  ( " << to.second << " )" << endl;
      cout << "~~~~";
#endif


      return OKorKO;
    }
  };





  TEST_F(SelectSubsetTest, ExtensiveTest)
  {
    m_TestBench.initializeDatastore();
    m_TestBench.populateDatastore();

    SelectSubset< StoredElement > selector;
    StoreArray< StoredElement > set(m_TestBench.getSetName());

    DataStore::Instance().setInitializeActive(true);

    selector.registerSubset(set, m_TestBench.getSubsetName());

    for (auto other : m_TestBench.getOtherSets()) {
      selector.inheritRelationsTo(other.second.storedArray());
      selector.inheritRelationsFrom(other.second.storedArray());
    }

    //inherit set <-> set relations
    selector.inheritRelationsFrom(set);

    DataStore::Instance().setInitializeActive(false);

    selector.select(SelectionCriteriumOnElement);

    StoreArray< StoredElement > subset(m_TestBench.getSubsetName());

    bool allSubsetElementsAreGood(true);


    // First of all let us check that all the elements in the subset
    // do satisfy the SelectionCriterium and that
    for (auto element : subset) {
      if (! SelectionCriteriumOnElement(& element))
        allSubsetElementsAreGood = false;


    }

    EXPECT_TRUE(allSubsetElementsAreGood);


    // Then we check that the size of the subset equals the number
    // of elements in the set satisfiing the SelectionCriterium
    int NSelected(0);
    for (auto element : set)
      NSelected += SelectionCriteriumOnElement(&element) ? 1 : 0;

    EXPECT_TRUE(NSelected == subset.getEntries());


    // Then we test that all the relations are correct
    EXPECT_TRUE(testSelfRelation());

    for (int i = 0 ; i < m_TestBench.getNSets(); i++) {

      auto otherSet = m_TestBench.getOtherNamedSet(i);
      EXPECT_TRUE(testRelationToOther(otherSet));
      EXPECT_TRUE(testRelationFromOther(otherSet));

    }
  }

  bool hasOddIndex(const RelationsObject* a)
  {
    return (a->getArrayIndex() % 2) == 1;
  }

  TEST_F(SelectSubsetTest, TestWithManyRelations)
  {
    //array 'main' with relations: a -> main -> b
    //create subset:    a -> subsetOfMain -> b

    DataStore::Instance().setInitializeActive(true);
    StoreArray< RelationsObject > arrayMain("main");
    StoreArray< RelationsObject > arrayA("a");
    StoreArray< RelationsObject > arrayB("b");
    arrayMain.registerInDataStore();
    arrayA.registerInDataStore();
    arrayB.registerInDataStore();
    arrayA.registerRelationTo(arrayMain);
    arrayMain.registerRelationTo(arrayB);

    //create subset and relations
    SelectSubset< RelationsObject > selectorMain;
    selectorMain.registerSubset(arrayMain, "subsetOfMain");

    selectorMain.inheritRelationsFrom(arrayA);
    selectorMain.inheritRelationsTo(arrayB);

    DataStore::Instance().setInitializeActive(false);

    //fill some data
    for (int i = 0; i < 10; i++) {
      auto* mainObj = arrayMain.appendNew();
      for (int j = 0; j < 2; j++) {
        auto* aObj = arrayA.appendNew();
        aObj->addRelationTo(mainObj);
      }
      for (int j = 0; j < 10; j++) {
        auto* bObj = arrayB.appendNew();
        mainObj->addRelationTo(bObj);
      }
    }

    //run selector
    selectorMain.select(hasOddIndex);


    //verify original contents
    EXPECT_EQ(10, arrayMain.getEntries());
    EXPECT_EQ(20, arrayA.getEntries());
    EXPECT_EQ(100, arrayB.getEntries());


    StoreArray< RelationsObject > arraySubset("subsetOfMain");

    //verify subset
    EXPECT_EQ(5, arraySubset.getEntries());
    for (const RelationsObject& r : arraySubset) {
      EXPECT_EQ(2u, r.getRelationsFrom<RelationsObject>("a").size());
      EXPECT_EQ(0u, r.getRelationsFrom<RelationsObject>("b").size());
      EXPECT_EQ(10u, r.getRelationsTo<RelationsObject>("b").size());
      EXPECT_EQ(0u, r.getRelationsTo<RelationsObject>("a").size());
      EXPECT_TRUE(nullptr == r.getRelatedTo<RelationsObject>("main"));

      //go back to main set, check selection condidition holds
      EXPECT_EQ(1u, r.getRelationsWith<RelationsObject>("main").size());
      const RelationsObject* originalObject = r.getRelatedFrom<RelationsObject>("main");
      EXPECT_TRUE(hasOddIndex(originalObject));
    }
  }

  TEST_F(SelectSubsetTest, InheritAll)
  {
    //array 'main' with relations: a -> main -> b
    //create subset:    a -> subsetOfMain -> b

    DataStore::Instance().setInitializeActive(true);
    StoreArray< RelationsObject > arrayMain("main");
    StoreArray< RelationsObject > arrayA("a");
    StoreArray< RelationsObject > arrayB("b");
    arrayMain.registerInDataStore();
    arrayA.registerInDataStore();
    arrayB.registerInDataStore();
    arrayA.registerRelationTo(arrayMain);
    arrayMain.registerRelationTo(arrayB);

    //create subset and relations
    SelectSubset< RelationsObject > selectorMain;
    selectorMain.registerSubset(arrayMain, "subsetOfMain");

    selectorMain.inheritAllRelations();

    DataStore::Instance().setInitializeActive(false);

    //check if the state matches what we expect
    EXPECT_FALSE(selectorMain.getInheritToSelf());
    EXPECT_EQ(std::vector<string>({"a"}), selectorMain.getInheritFromArrays());
    EXPECT_EQ(std::vector<string>({"b"}), selectorMain.getInheritToArrays());
  }

  TEST_F(SelectSubsetTest, TestExistingSetWithManyRelations)
  {
    //array 'main' with relations: a -> main -> b
    //then remove half of main.

    DataStore::Instance().setInitializeActive(true);
    StoreArray< RelationsObject > arrayMain("main");
    StoreArray< RelationsObject > arrayA("a");
    StoreArray< RelationsObject > arrayB("b");
    arrayMain.registerInDataStore();
    arrayA.registerInDataStore();
    arrayB.registerInDataStore();
    arrayA.registerRelationTo(arrayMain);
    arrayMain.registerRelationTo(arrayB);

    //create subset and relations
    SelectSubset< RelationsObject > selectorMain;
    selectorMain.registerSubset(arrayMain);

    DataStore::Instance().setInitializeActive(false);

    //fill some data
    for (int i = 0; i < 10; i++) {
      auto* mainObj = arrayMain.appendNew();
      for (int j = 0; j < 2; j++) {
        auto* aObj = arrayA.appendNew();
        aObj->addRelationTo(mainObj);
      }
      for (int j = 0; j < 10; j++) {
        auto* bObj = arrayB.appendNew();
        mainObj->addRelationTo(bObj);
      }
    }

    //verify original contents
    EXPECT_EQ(10, arrayMain.getEntries());
    EXPECT_EQ(20, arrayA.getEntries());
    EXPECT_EQ(100, arrayB.getEntries());

    //run selector
    selectorMain.select(hasOddIndex);


    //verify subset
    EXPECT_EQ(5, arrayMain.getEntries());
    for (const RelationsObject& r : arrayMain) {
      EXPECT_EQ(2u, r.getRelationsFrom<RelationsObject>("a").size());
      EXPECT_EQ(0u, r.getRelationsFrom<RelationsObject>("b").size());
      EXPECT_EQ(10u, r.getRelationsTo<RelationsObject>("b").size());
      EXPECT_EQ(0u, r.getRelationsTo<RelationsObject>("a").size());
      EXPECT_TRUE(nullptr == r.getRelatedTo<RelationsObject>("main"));
    }
    int i = 0;
    for (const RelationsObject& r : arrayA) {
      EXPECT_EQ((i / 2 % 2 == 1) ? 1u : 0u, r.getRelationsTo<RelationsObject>("main").size());
      i++;
    }
  }

  TEST_F(SelectSubsetTest, TestEmptyArray)
  {
    //array 'main' with relations: a -> main -> b
    //then remove half of main.

    DataStore::Instance().setInitializeActive(true);
    StoreArray< RelationsObject > arrayMain("main");
    StoreArray< RelationsObject > arrayA("a");
    StoreArray< RelationsObject > arrayB("b");
    arrayMain.registerInDataStore();
    arrayA.registerInDataStore();
    arrayB.registerInDataStore();
    arrayA.registerRelationTo(arrayMain);
    arrayMain.registerRelationTo(arrayB);

    //create subset and relations
    SelectSubset< RelationsObject > selectorMain;
    selectorMain.registerSubset(arrayMain);

    DataStore::Instance().setInitializeActive(false);

    //test with empty inputs
    EXPECT_EQ(0, arrayMain.getEntries());
    selectorMain.select(hasOddIndex);
    EXPECT_EQ(0, arrayMain.getEntries());

    //fill some data
    for (int i = 0; i < 10; i++) {
      auto* mainObj = arrayMain.appendNew();
      for (int j = 0; j < 2; j++) {
        auto* aObj = arrayA.appendNew();
        aObj->addRelationTo(mainObj);
      }
      for (int j = 0; j < 10; j++) {
        auto* bObj = arrayB.appendNew();
        mainObj->addRelationTo(bObj);
      }
    }

    //verify original contents
    EXPECT_EQ(10, arrayMain.getEntries());
    EXPECT_EQ(20, arrayA.getEntries());
    EXPECT_EQ(100, arrayB.getEntries());

    //delete all contents
    auto never = [](const RelationsObject*) -> bool { return false; };
    selectorMain.select(never);


    //verify subset
    EXPECT_EQ(0, arrayMain.getEntries());
    for (const RelationsObject& r : arrayA) {
      EXPECT_EQ(0u, r.getRelationsTo<RelationsObject>("main").size());
    }
  }

}  // namespace
