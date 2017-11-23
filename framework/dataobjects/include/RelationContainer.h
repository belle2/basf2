/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <TClonesArray.h>
#include <string>

namespace Belle2 {
  class RelationElement;

  /** Class to store relations between StoreArrays in the DataStore.
   *
   *  This class is only used internally, users should use RelationsObject/RelationsInterface to access/add relations.
   *
   *  \sa RelationElement
   */
  class RelationContainer: public TObject {
  public:

    /** Default constructor, initialize all values to sane default. */
    RelationContainer();

    /** Get name of the StoreArray we relate from. */
    const std::string& getFromName()       const { return m_fromName; }

    /** Get durability of the StoreArray we relate from. */
    int               getFromDurability() const { return m_fromDurability; }

    /** Get name of the StoreArray we relate to. */
    const std::string& getToName()         const { return m_toName; }

    /** Get durability of the StoreArray we relate to. */
    int               getToDurability()   const { return m_toDurability; }

    /** Set name of the StoreArray we relate from. */
    void setFromName(const std::string& name)   { m_fromName = name; }

    /** Set durability of the StoreArray we relate from. */
    void setFromDurability(int durability)      { m_fromDurability = durability; }

    /** Set name of the StoreArray we relate to. */
    void setToName(const std::string& name)     { m_toName = name; }

    /** Set durability of the StoreArray we relate to. */
    void setToDurability(int durability)        { m_toDurability = durability; }

    /** check for modification since creation or deserialization. */
    void setModified(bool modified)             { m_modified = modified; }

    /** Returns true if no information was set yet or Clear() was called. */
    bool isDefaultConstructed() const
    {
      return (m_fromDurability == -1 and m_toDurability == -1 and
              m_fromName.empty() and m_toName.empty() and getEntries() == 0);
    }

    /** Clear container to free memory. Prefer to deleting objects of this type. */
    virtual void Clear(Option_t* = "");

    /** Get reference to the elements. */
    TClonesArray& elements() { return m_elements; }

    /** Get const reference to the elements. */
    const TClonesArray& elements() const { return m_elements; }

    /** Get reference to RelationElement at index i. */
    const RelationElement& getElement(int i) const;

    /** Get number of elements. */
    int getEntries() const { return m_elements.GetEntriesFast(); }

    /** check for modification since creation or deserialization. */
    bool getModified() const { return m_modified; }

  protected:

    /** TClonesArray to store all elements. */
    TClonesArray m_elements;

    /** name of the StoreArray we relate from. */
    std::string m_fromName;

    /** durability of the StoreArray we relate from. */
    int m_fromDurability;

    /** name of the StoreArray we relate to. */
    std::string m_toName;

    /** durability of the StoreArray we relate to. */
    int m_toDurability;

    /** check for modification since creation or deserialization. */
    bool m_modified; //!transient

    friend class RelationArray;
    friend class DataStore;

    ClassDef(RelationContainer, 1); /**< Class to store relations between StoreArrays in the DataStore. */
  };
}
