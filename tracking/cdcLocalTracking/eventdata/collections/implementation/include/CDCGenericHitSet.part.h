
//Beginning of the descripitive section
//---------------------------------------------------------------------------------------------------------------------------
#ifdef CDCLOCALTRACKING_SHOW_SECTION_IN_DOXYGEN_ONLY


namespace Belle2 {
  namespace CDCLocalTracking {
/// Set of tracking entities.
    /** Set template to store tracking entities in a sorted range. It can inherit from ROOT's TObject \n
     *  what makes the collection accessable from python, but can be optimized by switching off the ROOT \n
     *  inheritance by the MockRoot paradigm. It is basically a wrapper around std::set<Item> usable with \n
     *  ROOT, while being prefectly usable with stl algorithms. It is also designed to have the same interface \n
     *  as its counter part CDCGenericHitVector based on vectors. \n
     *  Moreover some methods for accumulating information from the track entities have been added.

     *  Unfortunatly, despite relentless effort, ROOT cint resisted to instantiate the template in the normal C++ \n
     *  form shown here. So we have to fall back on a poor mans template instantiation methode, if we do not want \n
     *  to copy the code many times for each instance of the template. The instantiation mechanism works by defining \n
     *  the methods in a seperate .h file (this file) without the wrapping class definition. For each of the instances\n
     *  we open then a class definition and define the set container to be wrapped. Subsequently to contained item is then \n
     *  defined with a typedef to Item.
     *  Finally we import the method definitions in to the class by an #include.
     *
     *  If by some magic event ROOT will be able to instantiate templates containing std::set you can easily erase \n
     *  the poor mans template by replacing all instatiations with a typedef to the real template. \n
     *  The author admits that the templating way feels awkward and is definitly at the minus 100 range of programming \n
     *  style, but it achives the goal of begin accessable from python at wish while still being fast, without any changes \n
     *  to the using code. \n
     *  In case you do not like the poor mans template at all just copy all method definitions from here into the \n
     *  instantiated classes. */
    template<class Item>
    class CDCGenericHitSet : public UsedTObject {


#endif //CDCLOCALTRACKING_SHOW_SECTION_IN_DOXYGEN_ONLY
//---------------------------------------------------------------------------------------------------------------------------
//End of the descripitive section


      //typedef std::set<Item> Container;

    public:
      typedef const Item& StoredItemRef;  ///< Reference type to a stored item. Is constant for sets.

      typedef Container::value_type value_type; ///< Value type of this container

      typedef Container::iterator iterator; ///< Iterator type of this container
      typedef Container::const_iterator const_iterator;    ///< Constant iterator type of this container

      typedef Container::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
      typedef Container::const_reverse_iterator const_reverse_iterator;  ///< Constant reversed iterator type of this container

      typedef std::pair<iterator, iterator> range; ///< Pair type of iterators indicating a range
      typedef std::pair<const_iterator, const_iterator> const_range; ///< Pair type of constant iterators indication a range

      /// Input iterator type usable with stl algorithms
      class input_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
      protected:
        Collection& m_collection;

      public:
        explicit input_iterator(Collection& collection) : m_collection(collection) {}

        input_iterator& operator= (Item const& item) {
          m_collection.insert(item); return *this;
        }
        input_iterator& operator* ()
        { return *this; }
        input_iterator& operator++ ()
        { return *this; }
        input_iterator operator++ (int)
        { return *this; }
      };

      /// The begin of the container
      iterator begin() { return m_items.begin(); }

      /// The begin of the constant container
      const_iterator begin() const { return m_items.begin(); }

      /// The end of the container
      iterator end() { return m_items.end(); }

      /// The end of the constant container
      const_iterator end() const { return m_items.end(); }

      /// The reverse begin of the container
      reverse_iterator rbegin() { return m_items.rbegin(); }

      /// The reverse begin of the constant container
      const_reverse_iterator rbegin() const { return m_items.rbegin(); }

      /// The reverse end of the container
      reverse_iterator rend() { return m_items.rend(); }

      /// The reverse end of the constant container
      const_reverse_iterator rend() const { return m_items.rend(); }

      /// Getter for the element by its position, rather inefficient but easy read access from python
      /** This gets the element by its position in the set. Because the set is not a random access sequence \n
       *  this method of access is rather inefficient. Also, it returns a copy of the object.
       *  However it provides an easy access method for read from python and is intended for this purpose only */
      Item at(size_t idx) const {
        if (idx < size()) {
          const_iterator itItem = begin();
          std::advance(itItem, idx);
          return *itItem;
        } else {
          return Item();
        }
      }

      /// Returns the number of items in this collection
      size_t size() const { return m_items.size(); }

      /// Indicates whether the collections is empty
      bool empty() const { return m_items.empty(); }

      /// Removes all element from the collection
      void clear() { m_items.clear(); }

      /// For homogenity to vector. Can do nothing in case of a std::set as Container
      void reserve(size_t n __attribute__((unused))) {;}

      /// Returns an input iterator to the set
      input_iterator input() { return input_iterator(*this); }

      /// Inserts a single element in the set
      void insert(Item const& item) { m_items.insert(item); }

      /// Inserts a range of elements from an other set
      void insert(iterator first, iterator last) { for (; first != last; ++first) insert(*first); }

      /// Inserts a whole other set into this set
      void insert(const Collection& collection)
      { reserve(size() + collection.size()); insert(collection.begin(), collection.end()); }

      /// Stores an element into the set. Same method as in the vector for easy storage.
      /** The STL unfortunatly does not define a common method to store elements in all kinds \n
       *  of container types. With this method we provide a common method name to just store instances \n
       *  no matter we are storing to a set or to vector */
      inline StoredItemRef store(Item const& item)
      { return *(m_items.insert(item).first); }

      iterator insert(iterator posHint, const Item& item)
      { return m_items.insert(posHint , item); }



      /// Sorts the collection
      /** Does not do a thing for sets. For homogenity with the vector counterpart */
      void sort() {;}

      /// Checks if the collection is sorted
      /** Sets are always sorted .There for it returns always true. For homogenity with the vector counterpart */
      bool isSorted() const { return true; }

      /// Explicitly checks if the collection is sorted
      /** Sets are always sorted. However we explicitly validate this condition by a single interation over the set. */
      bool checkSorted() const {
        if (empty()) return true;
        bool result = true;

        const_iterator itFirst = begin();
        const_iterator itSecond = begin();
        ++itSecond;
        while (itSecond != end() and result) {
          result = *itFirst++ < *itSecond++ ? true : false;
        }
        return result;
      }

      /// Returns an iterator to the found item. end() if not found.
      const_iterator find(const Item& item) const { return m_items.find(item); }

      /// Returns returns the lower bound iterator of the item in the set.
      const_iterator lower_bound(const Item& item) const { return m_items.lower_bound(item); }

      /// Returns returns the equal_range iterator pair of the item in the set.
      range equal_range(const Item& item) { return m_items.equal_range(item); }

      /// Returns returns the equal_range constant iterator pair of item in the set.
      const_range equal_range(const Item& item) const { return m_items.equal_range(item); }

      /// Erases a single element denoted by its iterator from the set
      /** Erases a single element denoted by its iterator from the set. The given iterators should point into the set. */
      void erase(iterator itItem) { m_items.erase(itItem); }

      /// Erases a range from the set.
      /** Erases a range of items from the set. The given iterators should point into the set.*/
      void erase(iterator itBegin, iterator itEnd) { m_items.erase(itBegin, itEnd); }

      /// Erases a single element from the set
      void erase(const Item& item) { m_items.erase(item); }

      /// Evaluates the number of occurences of the item in the set
      /** This returns one if the element is in the set and zero if not*/
      size_t count(const Item& item) const { return m_items.count(item); }

      /// Checks if the element is in the set
      bool contains(const Item& item) const { return find(item) != end(); }

      /// Checks if the given set is completely contained in this set
      bool contains(const Collection& items) const {
        if (isSorted() and items.isSorted()) {
          return std::includes(begin(), end(), items.begin(), items.end());
        } else {
          //for homogenity to CDCSegment for sets this is always the case
          for (const_iterator itItem = items.begin(); itItem != end() ; ++itItem) {
            if (!contains(*itItem)) return false;
          }
          return true;
        }
      }

    private:
      /// Helper class for STL algorithms searching for specific wire
      class HasWirePredicate {
      public:
        HasWirePredicate(const CDCWire& wire) : m_wire(wire) {;}
        bool operator()(const Item& item) { return item->hasWire(m_wire); }
      private:
        const CDCWire& m_wire;
      };
      /// Helper class for STL algorithms searching for specific wire hit
      class HasWireHitPredicate {
      public:
        HasWireHitPredicate(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) : m_wirehit(wirehit) {;}
        bool operator()(const Item& item) { return item->hasWireHit(m_wirehit); }
      private:
        const  Belle2::CDCLocalTracking::CDCWireHit& m_wirehit;
      };


    public:
      /// Erases all tracking entities assoziated with specific wire from the set.
      void eraseAll(const CDCWire& wire) {
        //only works this way with set
        for (iterator itItem = m_items.begin(); itItem != m_items.end();) {
          if ((*itItem)->hasWire(wire)) m_items.erase(itItem++);
          else ++itItem;
        }
        //from effective STL by Scott Meyers
      }

      /// Erases all tracking entities assoziated with specific wire hit from the set.
      void eraseAll(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) {
        //only works this way with set
        for (iterator itItem = m_items.begin(); itItem != m_items.end();) {
          if ((*itItem)->hasWireHit(wirehit)) m_items.erase(itItem++);
          else ++itItem;
        }
        //from effective STL by Scott Meyers
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire.
      bool hasWire(const CDCWire& wire) const {
        iterator found = std::find_if(begin(), end(), HasWirePredicate(wire));
        return found != end();
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire hit.
      bool hasWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) const {
        iterator found = std::find_if(begin(), end(), HasWireHitPredicate(wirehit));
        return found != end();
      }

      /// Copy all entities in this collection assoziated with a specific wire to the given collection
      void collectForWire(const CDCWire& wire, Collection& collect) const {
        input_iterator inputTo = collect.input();
        for (iterator itItem = begin(); itItem != end(); ++itItem) {
          if ((*itItem)->hasWire(wire)) inputTo = *itItem; ++inputTo;
        }
      }

      /// Copy all entities in this collection assoziated with a specific wire hit to the given collection
      void collectForWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit, Collection& collect) const {
        input_iterator inputTo = collect.input();
        for (iterator itItem = begin(); itItem != end(); ++itItem) {
          if ((*itItem)->hasWireHit(wirehit)) inputTo = *itItem; ++inputTo;
        }
      }

      /*
      void collectForWire( const CDCWire & wire, Collection & collect) const{
        for (iterator itItem = begin(); itItem != end(); ++itItem ){
          if ( (*itItem)->hasWire(wire) ) collect.insert(*itItem);
        }
      }

      void collectForWireHit( const CDCWireHit & wirehit, Collection & collect) const{
        for (iterator itItem = begin(); itItem != end(); ++itItem ){
          if ( (*itItem)->hasWireHit(wirehit) ) collect.insert(*itItem);
        }
      }*/

      /// Calculates the average center of mass of all stored tracking entities.
      Vector2D getCenterOfMass2D() const {
        Vector2D accumulate(0.0, 0.0);
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          accumulate.add((*itItem)->getCenterOfMass2D());
        }
        accumulate.divide(size());
        return accumulate;
      }

      /// Returns the common axial type of all tracking entities.
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the axial type of the later. Returns INVALID_AXIALTYPE if the superlayer \n
       *  is not shared among the tracking entities. */
      AxialType getAxialType() const
      { return getISuperLayer() == INVALIDSUPERLAYER ? INVALID_AXIALTYPE : (*begin())->getAxialType(); }

      /// Returns the common super layer id of all stored tracking entities
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the superlayer id of the later. Returns INVALIDSUPERLAYER if the superlayer \n
       *  is not shared among the tracking entities. */
      ILayerType getISuperLayer() const {
        if (empty()) return INVALIDSUPERLAYER;
        const_iterator itItem = begin();
        const Item& firstItem =  *itItem;
        ILayerType iSuperlayer = firstItem->getISuperLayer();
        for (++itItem; itItem != end(); ++itItem) {
          if (iSuperlayer != (*itItem)->getISuperLayer()) return INVALIDSUPERLAYER ;
        }
        return iSuperlayer;
      }

      /// Calculates the sum of all squared distances of the stored tracking entities to the circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {

        FloatType accumulate = 0;
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          const Item& item = *itItem;
          accumulate += item->getSquaredDist2D(trajectory2D);
        }
        return accumulate;

      }

    private:
      Container m_items; ///< Memory of the wrapped stl container.


#ifdef CDCLOCALTRACKING_SHOW_SECTION_IN_DOXYGEN_ONLY
    } //end class GenericHitSet
  } //end namespace CDCLocalTracking
} //end namespace Belle2
#endif
