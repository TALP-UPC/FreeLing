/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   simple-hash-table.h
 * \brief  Defines class SimpleHashTable
 * \author Terry Koo
 * \note   This file was ported from egstra
 */


#ifndef TREELER_SIMPLE_HASHT_H
#define TREELER_SIMPLE_HASHT_H



/* default initial size */
#define EGSTRA_SIMPLE_HASHT_DEFSIZE 1024

namespace treeler {

  /**
     \brief A memory-efficient hash table 
     \author Terry Koo 
     \note This file was ported from egstra

     SimpleHashtable : This is a modified version of the hashtable in
     the kdp package.  Structures are kept as compact as possible to
     conserve memory.  The hashtable is templated on four classes: a key
     type Key, a value type Val, a functor KeyHash() that hashes Keys to
     unsigned ints, and a functor KeyEq() that returns a bool indicating
     Key equality.  The hashtable supports the following operations:
     
     - Val get(key, nullvalue)
        If key is in the table, return the associated value.  If key
        is not in the table, return nullvalue and leave the table
        unchanged.  Time O(H + B), where H is the time required to
        compute the hash function, and B is the number of elements in
        the bucket of the given key.

     - Val getput(key, defaultvalue)
        If key is in the table, return the associated value.  If key
        is not in the table, insert it in the table with the value
        defaultvalue and return defaultvalue.  Time O(H + B).

     - Val inc(key, initvalue)
        If key is in the table, increment the associated value using
        the "++" operator and return the incremented value.  If key is
        not in the table, insert it with the value initvalue and
        return initvalue.  Time O(H + B), assuming that the "++"
        operator is constant time.

     - Val* pget(key)
        If key is in the table, return a pointer to the associated
        value, or return NULL if key is not in the table.  This
        pointer can be used to assign a new value to that key.  Time
        O(H + B).

     - Val* pgetput(key, defaultvalue)
        If key is in the table, return a pointer to the associated
        value.  If key is not in the table, insert it in the table
        with the value defaultvalue and return a pointer to the
        newly-inserted value.  The returned pointer can be used to
        assign a new value to that key.  Time O(H + B).

     - void del(key)
        If key is in the table, remove that key from the table.  It is
        an error if key is not in the table.  Time O(H + B).

     - void clear()
        Remove all mappings from the table.  Time O(N + S), where N is
        the number of elements in the table, and S is the size of the
        underlying array (generally, S will be O(N) unless many keys
        are inserted and then subsequently deleted using del()).


     - enumerator mbegin()
     - const_enumerator begin()
        Facilities for enumerating the elements of the hashtable in a
        read-write or read-only fashion.  Time O(N + S) to complete an
        enumeration of all elements (amortized).

   The hashtable assumes that the datatypes being hashed are small and
   therefore easy to copy; thus literal Keys and Vals are passed,
   rather than references.  This is an appropriate assumption for the
   intended usages of this object, which are as a feature dictionary
   for fixed-length bit-string features or as a sparse parameter
   vector.
  */
  template <class Key, class Val, class KeyHash, class KeyEq>
  class simple_hasht {
  private:
    /* an element in the hashtable */
    struct elt {
      struct elt* next; /* forward iterator for bucket list */
      Key k;
      Val v;
    };

    /* array of buckets */
    struct elt** _arr;
    /* size of the bucket array (always a power of 2) */
    int _size;
    /* bitmask to truncate hashcodes to array indices */
    int _mask;
    /* number of elements in the hash */
    int _n;
    /* number of active buckets */
    int _nactive;

    /* hashing functors */
    const KeyHash _hash;
    const KeyEq _cmp;

    /* helper function that enlarges the bucket array */
    void enlarge();

  public:
    simple_hasht(const int initsize = EGSTRA_SIMPLE_HASHT_DEFSIZE);
    ~simple_hasht();

    /* return the value associated with k or nullvalue if k is not in
       the table */
    Val get(const Key& k, const Val nullvalue) const;
    /* return the value associated with k or insert k with value
       defaultvalue if k is not in the table */
    Val getput(const Key& k, const Val defaultvalue);
    /* increment and return the value associated with k, or insert k
       with value initvalue if k is not in the table */
    Val inc(const Key& k, const Val initvalue);
    /* return a pointer to the value associated with k, or return NULL
       otherwise.  the returned pointer can be used to modify the
       value assigned to k */
    Val* pget(const Key& k);
    /* return a pointer to the value associated with k, or insert k
       with value defaultvalue, and return a pointer to the
       newly-inserted value.  the returned pointer can be used to
       modify the value assigned to k */
    Val* pgetput(const Key& k, const Val defaultvalue);
    /* remove key k from the table.  it is an error to remove a
       nonexistant key */
    void del(const Key& k);
    /* remove all keys from this table */
    void clear();


    /* basic accessors */
    int size() const    { return _size; }
    int n() const       { return _n; }
    int nactive() const { return _nactive; }



    /* object that enumerates the keys in a mutable fashion.  an
       enumerator is invalidated by any operation that alters the keys
       in the hashtable, e.g., by inserting or deleting keys.
       enumerators are not invalidated by operations that change the
       values associated with existing keys and leave the keys
       themselves unaltered.  a facility for setting new values during
       the course of the enumeration is provided by the next()
       function. */
    class enumerator {
    private:
      struct elt** _p;
      struct elt* _q;
      struct elt** const _end;

    public:
      enumerator(struct elt** const p, const int size);

      /* fetch the next key and value and return true, or return false
	 if there are no more (key,val) pairs */
      bool next(Key& k, Val& v);
      /* fetch the next key and return a pointer to the associated
	 value, or return NULL if there are no more (key,val) pairs.
	 the returned pointer can be used to assign a new value. */
      Val* next(Key& k);
      /* fetch a pointer to the associated value, or return NULL if
	 there are no more (key,val) pairs.  the returned pointer can
	 be used to assign a new value. */
      Val* next();
    };
    enumerator mbegin();


    /* object that enumerates the keys in a const fashion.  a
       const_enumerator is invalidated if any operation other than
       get() is called on the table. */
    class const_enumerator {
    private:
      const struct elt** _p;
      const struct elt* _q;
      const struct elt** const _end;

    public:
      const_enumerator(const struct elt** const p, const int size);

      /* fetch the next key and value and return true, or return false
	 if there are no more (key,val) pairs */
      bool next(Key& k, Val& v);
      /* fetch the next value and return true, or return false if
	 there are no more values */
      bool next(Val& v);
    };
    const_enumerator begin() const;
  };

}

/* insert templated code */
#include "treeler/util/simple-hash-table.tcc"

#endif /* TREELER_SIMPLE_HASHT_H */
