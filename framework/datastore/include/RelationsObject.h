/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Christian Pulvermacher                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationEntry.h>

#include <TObject.h>

#include <string>

class TClonesArray;

namespace Belle2 {
  /** hide some implementation details. */
  namespace _RelationsInterfaceImpl {
    /** See RelationsObject::getInfo() */
    std::string htmlToPlainText(const std::string& html);
  };

  /** Defines interface for accessing relations of objects in StoreArray.
   *
   *  \note Please use the RelationsObject typedef instead of this class.
   *
   *  Your class then provides methods like addRelationTo or getRelationsTo for
   *  an easy handling of relations to and from objects of this class.
   *  Retrieving relations this way is handled using auto-generated indices, and so should
   *  not introduce a large overhead.
   *
   *  <h1>Retrieving relations</h1>
   *
   *  \note Remember to use references or pointers when iterating over a
   *        StoreArray. Accessing relations through a copied object will
   *        not work.
   *
   *  You can either retrieve a vector of relations using getRelations...(),
      \code
      //retrieve all CDCSimHits for given particle
      const MCParticle* particle = particles[i];

      //range-based for loop over RelationVector<CDCSimHit>
      for (const CDCSimHit& simhit : particle->getRelationsTo<CDCSimHit>()) {
        //...
      }
      \endcode
   *
   *  or, for 1:1 relations, the first related object (or NULL) using getRelated...():
   *
      \code
      //retrieve MCParticle (should be only one) for given simhit
      const CDCSimHit* simhit = cdcsimhits[i];
      const MCParticle* mcpart = simhit->getRelatedFrom<MCParticle>()
      if (!mcpart) {
        //nothing found, do some error handling here
      }
      \endcode
   *
   *  Weigths are available when looping over the RelationVector returned by getRelations(),
   *  as in this example:
      \code
      //retrieve all CDCSimHits for given particle
      const MCParticle* particle = particles[i];
      RelationVector<CDCSimHit> cdcRelations = particle->getRelationsTo<CDCSimHit>();
      for (unsigned int iHit = 0; iHit < cdcRelations.size(); iHit++) {
        const CDCSimHit *simhit = cdcRelations[iHit];
        float weight = cdcRelations.weight(iHit);
        //...
      }
      \endcode

   *  or for at most one relation, using getRelatedWithWeight(), getRelatedFromWithWeight(), and getRelatedToWithWeight().
   *
   *  <h1>Adding relations</h1>
   *  Creating new relations is also fairly straightforward:
   *
      \code
      //given an MCParticle* particle:
      particle->addRelationTo(someOtherObject)
      \endcode

   *  Note that you'll have to register the relation in your module's initialize() function:
   *
      \code
      StoreArray<MCParticle> mcparticles;
      StoreArray<SomethingOtherClass> otherArray;
      mcparticles.registerRelationTo(otherArray);
      \endcode
   */
  template <class BASE> class RelationsInterface: public BASE {
    /** template class cannot be removed without breaking ROOT I/O, so just disable it. */
    static_assert(std::is_same<TObject, BASE>::value,
                  "Using RelationsInterface<BASE> is no longer allowed. Please use RelationsObject as a base class.");
  public:
    /** Default constructor.
     */
    RelationsInterface(): m_cacheDataStoreEntry(NULL), m_cacheArrayIndex(-1) {}

    /** Constructor, forwards all arguments to BASE constructor. */
    template<class ...Args> explicit RelationsInterface(Args&& ... params):
      BASE(std::forward<Args>(params)...),
      m_cacheDataStoreEntry(NULL), m_cacheArrayIndex(-1) { }

    /** Copy constructor.
     *
     *  Cached values are cleared.
     *  @param relationsInterface  The object that should be copied.
     */
    RelationsInterface(const RelationsInterface& relationsInterface):
      BASE(relationsInterface),
      m_cacheDataStoreEntry(NULL), m_cacheArrayIndex(-1) { }

    /** Assignment operator.
     *
     *  cached values of 'this' are not touched, since position does not change.
     *  @param relationsInterface  The object that should be assigned.
     */
    RelationsInterface& operator=(const RelationsInterface& relationsInterface)
    {
      if (this != &relationsInterface)
        this->BASE::operator=(relationsInterface);
      return *this;
    }


    /** Add a relation from this object to another object (with caching).
     *
     *  @param object  The object to which the relation should point.
     *  @param weight  The weight of the relation.
     */
    void addRelationTo(const RelationsInterface<BASE>* object, float weight = 1.0) const
    {
      if (object)
        DataStore::Instance().addRelation(this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                                          object, object->m_cacheDataStoreEntry, object->m_cacheArrayIndex, weight);
    }

    /** Add a relation from this object to another object (no caching, can be quite slow).
     *
     *  @param object  The object to which the relation should point.
     *  @param weight  The weight of the relation.
     */
    void addRelationTo(const TObject* object, float weight = 1.0) const
    {
      StoreEntry* toEntry = nullptr;
      int toIndex = -1;
      DataStore::Instance().addRelation(this, m_cacheDataStoreEntry, m_cacheArrayIndex, object, toEntry, toIndex, weight);
    }

    /** Copies all relations of sourceObj (pointing from or to sourceObj) to this object (including weights).
     *
     * Useful if you want to make a complete copy of a StoreArray object to
     * make modifications to it, but retain all information on linked objects.
     *
     * Note: this only works if sourceObj inherits from the same base (e.g. RelationsObject),
     *       and only for related objects that also inherit from the same base.
     */
    void copyRelations(const RelationsInterface<BASE>* sourceObj)
    {
      if (!sourceObj)
        return;
      auto fromRels = sourceObj->getRelationsFrom<RelationsInterface<BASE>>("ALL");
      for (unsigned int iRel = 0; iRel < fromRels.size(); iRel++) {
        fromRels.object(iRel)->addRelationTo(this, fromRels.weight(iRel));
      }

      auto toRels = sourceObj->getRelationsTo<RelationsInterface<BASE>>("ALL");
      for (unsigned int iRel = 0; iRel < toRels.size(); iRel++) {
        this->addRelationTo(toRels.object(iRel), toRels.weight(iRel));
      }
    }

    //===========================================================================
    //These return a vector of relations:

    /** Get the relations that point from this object to another store array.
     *
     *  @tparam TO     The class of objects to which the relations point.
     *  @param name    The name of the store array to which the relations point.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        A vector of relations.
     */
    template <class TO> RelationVector<TO> getRelationsTo(const std::string& name = "") const
    {
      return RelationVector<TO>(DataStore::Instance().getRelationsWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry,
                                m_cacheArrayIndex, TO::Class(), name));
    }

    /** Get the relations that point from another store array to this object.
     *
     *  @tparam FROM   The class of objects from which the relations point.
     *  @param name    The name of the store array from which the relations point.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        A vector of relations.
     */
    template <class FROM> RelationVector<FROM> getRelationsFrom(const std::string& name = "") const
    {
      return RelationVector<FROM>(DataStore::Instance().getRelationsWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry,
                                  m_cacheArrayIndex, FROM::Class(), name));
    }

    /** Get the relations between this object and another store array.
     *
     *  Relations in both directions are returned.
     *
     *  @tparam T      The class of objects to or from which the relations point.
     *  @param name    The name of the store array to or from which the relations point.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        A vector of relations.
     */
    template <class T> RelationVector<T> getRelationsWith(const std::string& name = "") const
    {
      return RelationVector<T>(DataStore::Instance().getRelationsWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry,
                               m_cacheArrayIndex, T::Class(), name));
    }

    //===========================================================================
    //These return only the first related object

    /** Get the object to which this object has a relation.
     *
     *  @tparam TO     The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class TO> TO* getRelatedTo(const std::string& name = "") const
    {
      return static_cast<TO*>(DataStore::Instance().getRelationWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                              TO::Class(), name).object);
    }

    /** Get the object from which this object has a relation.
     *
     *  @tparam FROM   The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class FROM> FROM* getRelatedFrom(const std::string& name = "") const
    {
      return static_cast<FROM*>(DataStore::Instance().getRelationWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry,
                                m_cacheArrayIndex, FROM::Class(), name).object);
    }

    /** Get the object to or from which this object has a relation.
     *
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The first related object or a null pointer.
     */
    template <class T> T* getRelated(const std::string& name = "") const
    {
      return static_cast<T*>(DataStore::Instance().getRelationWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                             T::Class(), name).object);
    }


    //===========================================================================
    //These return an object/weight pair

    /** Get first related object & weight of relation pointing to an array.
     *
     *  @tparam TO     The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class TO> std::pair<TO*, float> getRelatedToWithWeight(const std::string& name = "") const
    {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_ToSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                                                                  TO::Class(), name);
      return std::make_pair(static_cast<TO*>(entry.object), entry.weight);
    }

    /** Get first related object & weight of relation pointing from an array.
     *
     *  @tparam FROM   The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class FROM will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class FROM> std::pair<FROM*, float> getRelatedFromWithWeight(const std::string& name = "") const
    {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_FromSide, this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                                                                  FROM::Class(), name);
      return std::make_pair(static_cast<FROM*>(entry.object), entry.weight);
    }

    /** Get first related object & weight of relation pointing from/to an array.
     *
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        Pair of first related object and the relation weight, or (NULL, 1.0) if none found.
     */
    template <class T> std::pair<T*, float> getRelatedWithWeight(const std::string& name = "") const
    {
      RelationEntry entry = DataStore::Instance().getRelationWith(DataStore::c_BothSides, this, m_cacheDataStoreEntry, m_cacheArrayIndex,
                                                                  T::Class(), name);
      return std::make_pair(static_cast<T*>(entry.object), entry.weight);
    }


    //===========================================================================
    // Information about this object (not strictly relation-related)

    /** Return a short name that describes this object, e.g. pi+ for an MCParticle. */
    virtual std::string getName() const { return ""; }

    /** Return a short summary of this object's contents in HTML format.
     *
     * Reimplement this in your own class to provide useful output for display
     * or debugging purposes. For example, you might do something like:
     *
      \code
      std::stringstream out;
      out << "<b>PDG</b>: " << m_pdg << "<br>";
      out << "<b>Covariance Matrix</b>: " << HTML::getString(getCovariance5()) << "<br>";
      return out.str();
      \endcode
     *
     * @sa Particle::getInfoHTML() for a more complex example.
     * @sa HTML for some utility functions.
     * @sa Use getInfo() to get a raw text version of this output.
     */
    virtual std::string getInfoHTML() const { return ""; }

    /** Return a short summary of this object's contents in raw text format.
     *
     * Returns the contents of getInfoHTML() while translating line-breaks etc.
     * @note: You don't need to implement this function (it's not virtual),
     *        getInfoHTML() is enough.
     */
    std::string getInfo() const
    {
      return _RelationsInterfaceImpl::htmlToPlainText(getInfoHTML());
    }


    /** Get name of array this object is stored in, or "" if not found. */
    std::string getArrayName() const
    {
      DataStore::Instance().findStoreEntry(this, m_cacheDataStoreEntry, m_cacheArrayIndex);
      return m_cacheDataStoreEntry ? m_cacheDataStoreEntry->name : "";
    }


    /** Returns this object's array index (in StoreArray), or -1 if not found. */
    int getArrayIndex() const
    {
      DataStore::Instance().findStoreEntry(this, m_cacheDataStoreEntry, m_cacheArrayIndex);
      return m_cacheArrayIndex;
    }


#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)

    /**
     * @{
     * Define versions without template arguments, only available from python modules.
     */
    RelationVector<TObject> getRelationsTo(const std::string& name) const
    {
      return getRelationsTo<TObject>(name);
    }
    RelationVector<TObject> getRelationsFrom(const std::string& name) const
    {
      return getRelationsFrom<TObject>(name);
    }
    RelationVector<TObject> getRelationsWith(const std::string& name) const
    {
      return getRelationsWith<TObject>(name);
    }
    TObject* getRelatedTo(const std::string& name) const { return getRelatedTo<TObject>(name); }
    TObject* getRelatedFrom(const std::string& name) const { return getRelatedFrom<TObject>(name); }
    TObject* getRelated(const std::string& name) const { return getRelated<TObject>(name); }
    /** @} */
#endif
  protected:

    /** Returns the pointer to the raw DataStore array holding this object (protected since these arrays are easy to misuse). */
    TClonesArray* getArrayPointer() const
    {
      DataStore::Instance().findStoreEntry(this, m_cacheDataStoreEntry, m_cacheArrayIndex);
      if (!m_cacheDataStoreEntry)
        return nullptr;
      return m_cacheDataStoreEntry->getPtrAsArray();
    }

  private:

    /** Cache of the data store entry to which this object belongs. */
    mutable DataStore::StoreEntry* m_cacheDataStoreEntry;  //!

    /** Cache of the index in the TClonesArray to which this object belongs. */
    mutable int                    m_cacheArrayIndex;      //!

    friend class DataStore;

    //version 0 to disable streaming
    ClassDef(RelationsInterface, 0); /**< defines interface for accessing relations of objects in StoreArray. */
  };

  /** Provides interface for getting/adding relations to objects in StoreArrays. See RelationsInterface for details. */
  typedef RelationsInterface<TObject> RelationsObject;
}
