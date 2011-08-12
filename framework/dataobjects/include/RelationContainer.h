/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef RELATIONCONTAINER_H
#define RELATIONCONTAINER_H

#include <framework/dataobjects/RelationElement.h>

#include <TClonesArray.h>

#include <vector>
#include <string>

namespace Belle2 {

  /** Class to store Relations between StoreArrays in the DataStore. */
  class RelationContainer: public TObject {
  public:

    /** Default constructor, initialize all values to sane default. */
    RelationContainer():
        m_elements(RelationElement::Class()),
        m_fromName(""), m_fromDurability(-1),
        m_toName(""), m_toDurability(-1), m_modified(true) {}

    /** Get name of the StoreArray we relate from. */
    const std::string getFromName()       const { return m_fromName; }

    /** Get durability of the StoreArray we relate from. */
    const int         getFromDurability() const { return m_fromDurability; }

    /** Get name of the StoreArray we relate to. */
    const std::string getToName()         const { return m_toName; }

    /** Get durability of the StoreArray we relate to. */
    const int         getToDurability()   const { return m_toDurability; }

    /** Set name of the StoreArray we relate from. */
    void setFromName(const std::string &name)   { m_fromName = name; }

    /** Set durability of the StoreArray we relate from. */
    void setFromDurability(int durability)      { m_fromDurability = durability; }

    /** Set name of the StoreArray we relate to. */
    void setToName(const std::string &name)     { m_toName = name; }

    /** Set durability of the StoreArray we relate to. */
    void setToDurability(int durability)        { m_toDurability = durability; }

    /** check for modification since creation or deserialization. */
    void setModified(bool modified)             { m_modified = modified; }

    /** Clear container to free memory. */
    virtual void Clear(Option_t* = "") {
      m_elements.Delete();
      m_fromName = "";
      m_toName = "";
      m_modified = true;
    }

    /** Get reference to the elements. */
    TClonesArray &elements() { return m_elements; }

    /** Get const reference to the elements. */
    const TClonesArray &elements() const { return m_elements; }

    /** Get reference to Relation element i. */
    const RelationElement &elements(int i) const { return *static_cast<RelationElement*>(m_elements[i]); }

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
    std::string  m_toName;

    /** durability of the StoreArray we relate to. */
    int m_toDurability;

    /** check for modification since creation or deserialization. */
    bool m_modified; //!transient

    friend class RelationArray;

    ClassDef(RelationContainer, 1);
  };
}

#endif

