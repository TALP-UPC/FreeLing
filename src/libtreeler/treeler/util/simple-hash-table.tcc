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
 * \file   simple-hash-table.tcc
 * \brief  Defines template methods for class SimpleHashTable
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#include <stdlib.h>
#include <assert.h>

/* a hard stop on the size of the hashtable array (this prevents
   models with slightly more than 2^25 features from triggering the
   array-doubling and resulting in an out-of-memory error */
#define EGSTRA_SIMPLE_HASHT_MAXSIZE 33554432
/* 67108864 */

namespace treeler {
  /* create an empty hash table with the initial array size */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline
  simple_hasht<Key,Val,KeyHash,KeyEq>::simple_hasht(const int initsize)
    : _size(initsize), _mask(_size - 1), _n(0), _nactive(0),
      _hash(), _cmp() {
    assert(_size > 0);
    /* check that initsize is a power of 2 */
    for(int i = 1; i < _size; i <<= 1) {
      assert((_size & i) == 0);
    }
    /* allocate bucket array */
    _arr = (struct elt**)calloc(_size, sizeof(struct elt*));
  }

  /* deallocate a hash table */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline
  simple_hasht<Key,Val,KeyHash,KeyEq>::~simple_hasht() {
    if(_n > 0) {
      struct elt** p = _arr;
      struct elt** const end = _arr + _size;
      /* for each bucket, empty that bucket */
      for(; p != end; ++p) {
	struct elt* q = *p;
	while(q != NULL) {
	  struct elt* const next = q->next;
	  free(q);
	  q = next;
	}
      }
    }
    free(_arr);
  }

  /* remove all elements */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline
  void simple_hasht<Key,Val,KeyHash,KeyEq>::clear() {
    if(_n > 0) {
      struct elt** p = _arr;
      struct elt** const end = _arr + _size;
      /* for each bucket, empty that bucket */
      for(; p != end; ++p) {
	struct elt* q = *p;
	while(q != NULL) {
	  struct elt* const next = q->next;
	  free(q);
	  q = next;
	}
      }
    }

    /* restore initial table size */
    free(_arr);
    _size = EGSTRA_SIMPLE_HASHT_DEFSIZE;
    _mask = _size - 1;
    _n = 0;
    _nactive = 0;
    _arr = (struct elt**)calloc(_size, sizeof(struct elt*));
  }
  

  /* double the array size of a given table and rehash all elements */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline void
  simple_hasht<Key,Val,KeyHash,KeyEq>::enlarge() {
    /* we need to allocate a larger array and rehash all elements into
       the new array.  we do this in three steps:

       1. collect all elements in an independent array
       2. create a larger bucket array
       3. rehash all elements

       the intermediate array of step 1 is necessary because the
       process of rehashing all elements will modify the elt->next
       pointers. */

    /* 1. collect all elements in an independent array */
    struct elt** const saved =
      (struct elt**)malloc(_n*sizeof(struct elt*));
    { /* for each bucket, grab everything in that bucket */
      struct elt** s = saved;
      struct elt** p = _arr;
      struct elt** const end = _arr + _size;
      for(; p != end; ++p) {
	for(struct elt* q = *p; q != NULL; q = q->next) {
	  *s = q; ++s;
	}
      }
      assert((s - saved) == _n);
    }

    /* 2. create a larger bucket array */
    const int newsize = (_size << 1);
    _size = newsize;
    const int mask = newsize - 1;
    _mask = mask;
    free(_arr);
    _arr = (struct elt**)calloc(_size, sizeof(struct elt*));

    /* 3. rehash all elements */
    _nactive = 0;
    struct elt** s = saved;
    struct elt** end = saved + _n;
    for(; s != end; ++s) {
      struct elt* const p = *s;
      const int idx = (_hash(p->k) & mask);
      if(_arr[idx] == NULL) { ++_nactive; }
      /* since we are rehashing elements that are already in the
	 table, we can assume that all keys are unique, and we don't
	 need to check keys with KeyEq() */
      p->next = _arr[idx];
      _arr[idx] = p;
    }

    free(saved);
  }



  /**********************************************************************
   * things to try: relink retrieved elements to head of bucket
   **********************************************************************/


  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val
  simple_hasht<Key,Val,KeyHash,KeyEq>::get(const Key& k,
					   const Val nullvalue) const {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);

    /* search in the bucket */
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    if(p == NULL) { return nullvalue; }
    else          { return p->v; }
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val
  simple_hasht<Key,Val,KeyHash,KeyEq>::getput(const Key& k,
					      const Val defaultvalue) {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);

    /* search in the bucket */
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    if(p == NULL) {
      /* create a new element */
      struct elt* const e = (struct elt*)malloc(sizeof(struct elt));
      e->k = k;
      e->v = defaultvalue;
      /* link it to the head of the bucket */
      if((e->next = _arr[idx]) == NULL) { ++_nactive; }
      _arr[idx] = e;
      ++_n;
      /* double the size of the array if it is full */
      if(_n >= _size && _size < EGSTRA_SIMPLE_HASHT_MAXSIZE) { enlarge(); }
      return defaultvalue;
    } else {
      return p->v;
    }
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val
  simple_hasht<Key,Val,KeyHash,KeyEq>::inc(const Key& k,
					   const Val initvalue) {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);

    /* search in the bucket */
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    if(p == NULL) {
      /* create a new element */
      struct elt* const e = (struct elt*)malloc(sizeof(struct elt));
      e->k = k;
      e->v = initvalue;
      /* link it to the head of the bucket */
      if((e->next = _arr[idx]) == NULL) { ++_nactive; }
      _arr[idx] = e;
      ++_n;
      /* double the size of the array if it is full */
      if(_n >= _size && _size < EGSTRA_SIMPLE_HASHT_MAXSIZE) { enlarge(); }
      return initvalue;
    } else {
      return ++(p->v);
    }
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val*
  simple_hasht<Key,Val,KeyHash,KeyEq>::pget(const Key& k) {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);

    /* search in the bucket */
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    if(p == NULL) { return NULL; }
    else          { return &(p->v); }
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val*
  simple_hasht<Key,Val,KeyHash,KeyEq>::pgetput(const Key& k,
					       const Val defaultvalue) {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);
    
    /* search in the bucket */
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    if(p == NULL) {
      /* create a new element */
      
      /* XC, Sept 2012, changed this line using malloc ... */
      //      struct elt* const e = (struct elt*)malloc(sizeof(struct elt));
      /* for the following line using new */
      struct elt* const e = new elt;
      /* malloc's allocation was giving a segfault when assinging the
	 field k in case where Key is something more complex than a
	 long integer, such as a FIdxChars or FIdxPair */

      e->k = k;
      e->v = defaultvalue;

      /* link it to the head of the bucket */
      if((e->next = _arr[idx]) == NULL) { ++_nactive; }
      _arr[idx] = e;
      ++_n;
      /* double the size of the array if it is full */
      if(_n >= _size && _size < EGSTRA_SIMPLE_HASHT_MAXSIZE) { enlarge(); }
      return &(e->v);
    } else {
      return &(p->v);
    }
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline void
  simple_hasht<Key,Val,KeyHash,KeyEq>::del(const Key& k) {
    /* find the bucket index */
    const int idx = (_hash(k) & _mask);

    /* search in the bucket */
    struct elt* prev = NULL;
    struct elt* p;
    for(p = _arr[idx];
	p != NULL && !_cmp(k, p->k);
	prev = p, p = p->next) {}

    /* at this point, p contains either a pointer to an equal-key elt,
       or NULL if the key does not exist in the hashtable */
    assert(p != NULL);
    /* unlink the element from the table */
    if(prev == NULL) { /* p is the head of the list */
      if((_arr[idx] = p->next) == NULL) { --_nactive; }
    } else { /* prev is the parent of p */
      prev->next = p->next;
    }
    /* deallocate the element */
    free(p);
    --_n;
    /*** shrink the hashtable array? ***/
  }



  /* enumerator implementation */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline
  simple_hasht<Key,Val,KeyHash,KeyEq>::
  enumerator::enumerator(struct elt** const p, const int size)
    : _p(p), _q(*_p), _end(_p + size) {
    assert(size > 0);
  }

  /* return a (key,value) pair */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline bool
  simple_hasht<Key,Val,KeyHash,KeyEq>::enumerator::next(Key& k, Val& v) {
    if(_q == NULL) {
      ++_p;
      /* search forward for next non-null entry */
      for(; _p != _end && *_p == NULL; ++_p) {}
      /* check end of array */
      if(_p == _end) { return false; }
      _q = *_p;
    }
    assert(_q != NULL);
    k = _q->k;
    v = _q->v;
    _q = _q->next;
    return true;
  }

  /* return a key and a pointer to its associated value */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val*
  simple_hasht<Key,Val,KeyHash,KeyEq>::enumerator::next(Key& k) {
    if(_q == NULL) {
      ++_p;
      /* search forward for next non-null entry */
      for(; _p != _end && *_p == NULL; ++_p) {}
      /* check end of array */
      if(_p == _end) { return NULL; }
      _q = *_p;
    }
    assert(_q != NULL);
    k = _q->k;
    Val* const v = &(_q->v);
    _q = _q->next;
    return v;
  }

  /* return a pointer to the next value */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline Val*
  simple_hasht<Key,Val,KeyHash,KeyEq>::enumerator::next() {
    if(_q == NULL) {
      ++_p;
      /* search forward for next non-null entry */
      for(; _p != _end && *_p == NULL; ++_p) {}
      /* check end of array */
      if(_p == _end) { return NULL; }
      _q = *_p;
    }
    assert(_q != NULL);
    Val* const v = &(_q->v);
    _q = _q->next;
    return v;
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline typename simple_hasht<Key,Val,KeyHash,KeyEq>::enumerator
  simple_hasht<Key,Val,KeyHash,KeyEq>::mbegin() {
    return enumerator(_arr, _size);
  }



  /* const enumerator implementation */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline
  simple_hasht<Key,Val,KeyHash,KeyEq>::
  const_enumerator::const_enumerator(const struct elt** const p,
				     const int size)
    : _p(p), _q(*_p), _end(_p + size) {
    assert(size > 0);
  }

  /* return a (key,value) pair */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline bool
  simple_hasht<Key,Val,KeyHash,KeyEq>::const_enumerator::
  next(Key& k, Val& v) {
    if(_q == NULL) {
      ++_p;
      /* search forward for next non-null entry */
      for(; _p != _end && *_p == NULL; ++_p) {}
      /* check end of array */
      if(_p == _end) { return false; }
      _q = *_p;
    }
    assert(_q != NULL);
    k = _q->k;
    v = _q->v;
    _q = _q->next;
    return true;
  }

  /* return a value */
  template <class Key, class Val, class KeyHash, class KeyEq>
  inline bool
  simple_hasht<Key,Val,KeyHash,KeyEq>::const_enumerator::
  next(Val& v) {
    if(_q == NULL) {
      ++_p;
      /* search forward for next non-null entry */
      for(; _p != _end && *_p == NULL; ++_p) {}
      /* check end of array */
      if(_p == _end) { return false; }
      _q = *_p;
    }
    assert(_q != NULL);
    v = _q->v;
    _q = _q->next;
    return true;
  }

  template <class Key, class Val, class KeyHash, class KeyEq>
  inline typename simple_hasht<Key,Val,KeyHash,KeyEq>::const_enumerator
  simple_hasht<Key,Val,KeyHash,KeyEq>::begin() const {
    return const_enumerator((const struct elt**)_arr, _size);
  }
}
