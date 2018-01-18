//////////////////////////////////////////////////////////////////
//
//    STL-like n-ary tree template 
//
//    Copyright (C) 2006   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This program is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _TREE_TEMPLATE
#define _TREE_TEMPLATE

#include "freeling/windll.h"

namespace freeling {

  // predeclaration
  template<class T, class N> class basic_tree_iterator;
  template<class T> class basic_const_tree_iterator;
  template<class T> class basic_nonconst_tree_iterator;
  template<class T, class X> class basic_preorder_iterator;
  template<class T, class X> class basic_sibling_iterator;
  template<class T> class tree_preorder_iterator;
  template<class T> class const_tree_preorder_iterator;
  template<class T> class tree_sibling_iterator;
  template<class T> class const_tree_sibling_iterator;
   
  //////////////////////////////////////////////////////
  ///  STL-like container implementing a n-ary tree.
  //////////////////////////////////////////////////////

  template <class T> class tree  {
    /// iterators are friends of the tree, so they can access the structure

    friend class basic_nonconst_tree_iterator<T>;
    friend class basic_tree_iterator<T, tree<T> >;
    friend class basic_tree_iterator<T, const tree<T> >;
    friend class basic_sibling_iterator<T,basic_nonconst_tree_iterator<T> >;
    friend class basic_sibling_iterator<T,basic_const_tree_iterator<T> >;
    friend class basic_preorder_iterator<T,basic_nonconst_tree_iterator<T> >;
    friend class basic_preorder_iterator<T,basic_const_tree_iterator<T> >;

    private:
      /// auxiliary to copy, assignment, and destructor
      void clone(const tree<T>&);
    
    protected:
      /// information contained in the root node
      T *pinfo;
      /// parent node
      tree<T> *parent;        
      /// first/last child
      tree<T> *first, *last;
      /// prev/next sibling
      tree<T> *prev, *next;   
      /// number of children
      unsigned int nchildren;
   
    public:
      /// iterator types for tree<T>
      typedef tree_preorder_iterator<T> preorder_iterator;
      typedef const_tree_preorder_iterator<T> const_preorder_iterator;
      typedef tree_sibling_iterator<T> sibling_iterator;
      typedef const_tree_sibling_iterator<T> const_sibling_iterator;

      /// default tree iterator is preorder
      typedef preorder_iterator iterator;
      typedef const_preorder_iterator const_iterator;
      
      /// constuctor
      tree();
      tree(const T&);
      tree(const const_iterator&);
      /// copy
      tree(const tree<T>&);
      /// assignment
      tree<T>& operator=(const tree<T>&);
      /// destructor
      ~tree();
      
      /// clear tree content
      void clear();
      /// check whether the node is the root of the tree 
      /// (that is, it has no further parent above)
      bool is_root() const;
      /// check whether the tree is empty (no nodes)
      bool empty() const;
      /// number of children of the tree
      unsigned int num_children() const;
      /// see if the tree is somewhere under given tree
      bool has_ancestor(const tree<T> &) const;

      sibling_iterator nth_child(unsigned int);
      const_sibling_iterator nth_child(unsigned int) const;
      tree<T> & nth_child_ref(unsigned int);
      const tree<T> & nth_child_ref(unsigned int) const;

      static const_iterator get_leftmost_leaf(const_iterator);
      static const_iterator get_rightmost_leaf(const_iterator);

      /// copy given tree and add it as a child
      void add_child(const tree<T>& t, bool back=true);
      void add_child(const const_iterator &p, bool back=true);
      /// add given tree and as a child reordering structure. NO COPIES MADE.
      void hang_child(tree<T>& t, tree_sibling_iterator<T> where=tree_sibling_iterator<T>(NULL));
      void hang_child(preorder_iterator &p, tree_sibling_iterator<T> where=tree_sibling_iterator<T>(NULL));
      void hang_child(sibling_iterator &p, tree_sibling_iterator<T> where=tree_sibling_iterator<T>(NULL));

      /// get iterator to parent node
      preorder_iterator get_parent();
      const_preorder_iterator get_parent() const;
      
      preorder_iterator begin();
      preorder_iterator end();
      const_preorder_iterator begin() const;
      const_preorder_iterator end() const;
      
      sibling_iterator sibling_begin();
      sibling_iterator sibling_end();
      const_sibling_iterator sibling_begin() const;
      const_sibling_iterator sibling_end() const;
      sibling_iterator sibling_rbegin();
      sibling_iterator sibling_rend();
      const_sibling_iterator sibling_rbegin() const;
      const_sibling_iterator sibling_rend() const;
  };
 
  /////////////////////////////////////////////
  /// Constructor. Empty tree (pinfo==NULL)

  template<class T> tree<T>::tree() {
    pinfo = NULL;
    parent = first = last = prev = next = NULL;
    nchildren = 0;
  }

  /////////////////////////////////////////////
  /// Constructor, given one element

  template<class T> tree<T>::tree(const T &x) {
    pinfo = new T(x); 
    parent = first = last = prev = next = NULL;
    nchildren = 0;
  }
 
  /////////////////////////////////////////////
  /// Constructor, given one iterator

  template<class T> tree<T>::tree(const const_iterator &p) {
    clone(*(p.tr));
  }


  /////////////////////////////////////////////
  /// Copy constructor

  template<class T> tree<T>::tree(const tree<T>& t) {
    clone(t);
  }

  /////////////////////////////////////////////
  /// Assignment

  template<class T> tree<T>& tree<T>::operator=(const tree<T>& t) {
    if (this!=&t) {
      clear();
      clone(t);
    }
    return (*this);
  }
  
  /////////////////////////////////////////////
  /// Destructor

  template<class T> tree<T>::~tree() {
    clear();
  }

  /////////////////////////////////////////////
  /// Auxiliary for destructor and asignment

  template<class T> void tree<T>::clear() {
    tree<T> *p = first;
    while (p!=NULL) {
      tree<T> *q = p->next;
      delete p;
      p=q;
    }
    delete pinfo;
  }

  /////////////////////////////////////////////
  /// Auxiliary for copy and assignment
  
  template<class T> void tree<T>::clone(const tree<T>& t) {
    pinfo = (not t.empty() ? new T(*t.pinfo) : NULL);
    parent = first = last = prev = next = NULL;
    nchildren = 0;
    for (tree<T>* p = t.first; p!=NULL; p=p->next)  {
      this->add_child(*p);
    }
  }

  /////////////////////////////////////////////
  /// Check whether the tree top has no parent (is the root

  template<class T> bool tree<T>::is_root() const {
    return (parent==NULL);
  }
    
  /////////////////////////////////////////////
  /// Check whether the tree is empty

  template<class T> bool tree<T>::empty() const {
    return (pinfo==NULL);
  }


  /////////////////////////////////////////////
  /// get number of children of the tree

  template<class T> unsigned int tree<T>::num_children() const {
    return nchildren;
  }


  /////////////////////////////////////////////
  /// see if the tree is somewhere under given tree

  template<class T> bool tree<T>::has_ancestor(const tree<T> &p) const {
    const tree<T> *t = this;
    while (not t->is_root() and t!=&p) t=t->parent;
    return t==&p;
  }

  /////////////////////////////////////////////
  /// get iterator to n-th children (or end() if not there)
  
  template<class T> typename tree<T>::sibling_iterator tree<T>::nth_child(unsigned int n) {
    sibling_iterator i = this->sibling_begin();    
    while (n>0 && i!=this->sibling_end()) {
      i++;
      n--;
    }
    return i;
  }


  /////////////////////////////////////////////
  /// const version of nth_child
  
  template<class T> typename tree<T>::const_sibling_iterator tree<T>::nth_child(unsigned int n) const {
    const_sibling_iterator i = this->sibling_begin();    
    while (n>0 && i!=this->sibling_end()) {
      i++;
      n--;
    }
    return i;
  }

  /////////////////////////////////////////////
  /// get reference to n-th child (or end() if not there)
  
  template<class T> tree<T>& tree<T>::nth_child_ref(unsigned int n) {
    sibling_iterator i = this->sibling_begin();    
    while (n>0 && i!=this->sibling_end()) {
      i++;
      n--;
    }

    tree<T> *t;
    if (i!=this->sibling_end()) 
      return *(i.tr);
    else {
      // no such child, return undefined tree.
      t = new tree<T>();
      return *t;
    }
  }


  /////////////////////////////////////////////
  /// const version of nth_child_ref
  
  template<class T> const tree<T>& tree<T>::nth_child_ref(unsigned int n) const {
    const_sibling_iterator i = this->sibling_begin();
    while (n>0 && i!=this->sibling_end()) {
      i++;
      n--;
    }

    tree<T> *t;
    if (i!=this->sibling_end()) 
      return *(i.tr);
    else {
      // no such child, return undefined tree.
      t = new tree<T>();
      return *t;
    }
  }

  /////////////////////////////////////////////
  /// get iterator to leftmost leaf subsumed by this tree

  template<class T> typename tree<T>::const_iterator tree<T>::get_leftmost_leaf(typename tree<T>::const_iterator t) {
    if (t.num_children()==0) 
      return t;
    else 
      return get_leftmost_leaf(t.sibling_begin());
  }

  /////////////////////////////////////////////
  /// get iterator to rightmost leaf subsumed by this tree

  template<class T> typename tree<T>::const_iterator tree<T>::get_rightmost_leaf(typename tree<T>::const_iterator t) {
    if (t.num_children()==0) 
      return t;
    else 
      return get_rightmost_leaf(t.sibling_rbegin());
  }


  /////////////////////////////////////////////
  /// get iterator to parent node

  template<class T> typename tree<T>::preorder_iterator tree<T>::get_parent() {
    return tree<T>::preorder_iterator(parent);
  }

  /////////////////////////////////////////////
  /// get iterator to parent node

  template<class T> typename tree<T>::const_preorder_iterator tree<T>::get_parent() const {
    return tree<T>::const_preorder_iterator(parent);
  }
 

  /////////////////////////////////////////////
  /// add given tree and as a child reordering structure. NO COPIES MADE.

  template<class T> void tree<T>::hang_child(tree<T>& t, typename tree<T>::sibling_iterator where) {

    // remove child from its current location, preserving structure
    // 1- remove it from siblings chain
    if (t.prev!=NULL) t.prev->next = t.next;
    if (t.next!=NULL) t.next->prev = t.prev;  
    // 2- adujst parent pointers if first or last child
    if (t.parent!=NULL) {
      if (t.prev==NULL) t.parent->first=t.next;
      if (t.next==NULL) t.parent->last=t.prev;
      t.parent->nchildren--;
    }

    // hang child on new location under 'this'.
    t.parent = this;
    
    // locate 'where' position and insert node 
    t.prev = NULL; 
    t.next = this->first;
    while (t.next != where.tr) {
      t.prev = t.next;
      t.next = t.next->next;
    }

    // fix pointers in surrounding nodes
    if (t.next!=NULL) t.next->prev = &t;
    else this->last = &t;
    if (t.prev!=NULL) t.prev->next = &t;
    else this->first = &t;

    // update count
    this->nchildren++;
  }

  /////////////////////////////////////////////
  /// add given tree and as a child reordering structure. NO COPIES MADE.

  template<class T> void tree<T>::hang_child(typename tree<T>::preorder_iterator &p, typename tree<T>::sibling_iterator where) {
    this->hang_child(*(p.tr),where);
  }

  /////////////////////////////////////////////
  /// add given tree and as a child reordering structure. NO COPIES MADE.

  template<class T> void tree<T>::hang_child(typename tree<T>::sibling_iterator &p, typename tree<T>::sibling_iterator where) {
    this->hang_child(*(p.tr),where);
  }


  /////////////////////////////////////////////
  /// copy given tree and add it as a child

  template<class T> void tree<T>::add_child(const tree<T>& t, bool back) {
    tree<T> *nt = new tree<T>(t);  // make a copy of given tree
    // hang the copy under 'this'
    if (back) this->hang_child(*nt,this->sibling_end());  
    else this->hang_child(*nt,this->sibling_begin()); 
  }

  /////////////////////////////////////////////
  /// copy tree under given iterator and add it as a child

  template<class T> void tree<T>::add_child(const typename tree<T>::const_iterator &p, bool back) {
    tree<T> *nt = new tree<T>(p);  // make a copy of given tree
    // hang the copy under 'this'
    if (back) this->hang_child(*nt,this->sibling_end());  
    else this->hang_child(*nt,this->sibling_begin()); 
  }

  /////////////////////////////////////////////
  /// Get iterator to first node in the tree

  template<class T> typename tree<T>::preorder_iterator tree<T>::begin() {
    return tree<T>::preorder_iterator(this);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::preorder_iterator tree<T>::end() {
    return tree<T>::preorder_iterator((tree<T>*)NULL);
  }

  /////////////////////////////////////////////
  /// Get iterator to first node in the tree

  template<class T> typename tree<T>::const_preorder_iterator tree<T>::begin() const {
    return tree<T>::const_preorder_iterator(this);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::const_preorder_iterator tree<T>::end() const {
    return tree<T>::const_preorder_iterator((tree<T>*)NULL);
  }

  /////////////////////////////////////////////
  /// Get iterator to first node in the tree

  template<class T> typename tree<T>::sibling_iterator tree<T>::sibling_begin() {
    return tree<T>::sibling_iterator(this->first);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::sibling_iterator tree<T>::sibling_end() {
    return tree<T>::sibling_iterator((tree<T>*)NULL);
  }

  /////////////////////////////////////////////
  /// Get iterator to first node in the tree

  template<class T> typename tree<T>::const_sibling_iterator tree<T>::sibling_begin() const {
    return tree<T>::const_sibling_iterator(this->first);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::const_sibling_iterator tree<T>::sibling_end() const {
    return tree<T>::const_sibling_iterator((tree<T>*)NULL);
  }

  /////////////////////////////////////////////
  /// Get iterator to last node in the tree

  template<class T> typename tree<T>::sibling_iterator tree<T>::sibling_rbegin() {
    return tree<T>::sibling_iterator(this->last);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::sibling_iterator tree<T>::sibling_rend() {
    return tree<T>::sibling_iterator((tree<T>*)NULL);
  }

  /////////////////////////////////////////////
  /// Get iterator to first last in the tree

  template<class T> typename tree<T>::const_sibling_iterator tree<T>::sibling_rbegin() const {
    return tree<T>::const_sibling_iterator(this->last);
  }

  /////////////////////////////////////////////
  /// Get end of iterator chain

  template<class T> typename tree<T>::const_sibling_iterator tree<T>::sibling_rend() const {
    return tree<T>::const_sibling_iterator((tree<T>*)NULL);
  }



  /// ############################################################# ///
  ///
  ///                    TREE  ITERATORS
  ///
  /// ############################################################# ///

  ////////////////////////////////////////////////////////////////////////////    
  /// Generic const iterator, to derive all the others.
  /// It has what is common to all iterators (the pointer, the const operations,
  /// and access to const operations of the underlying tree
  
  template<class T, class N> class basic_tree_iterator {
    friend class tree<T>;
    
    protected:
      N *tr;
      
    public:
      basic_tree_iterator();
      ~basic_tree_iterator();

      /// basic const iterator operations
      const T& operator*() const;
      const T* operator->() const;
      /// emulate operator -> for java/python/perl APIS
      const T* get_info() const;
      bool operator==(const basic_tree_iterator &t) const;
      bool operator!=(const basic_tree_iterator &t) const;      

      /// check whether the iterator points somewhere.
      bool is_defined() const;
      /// access to tree operations via iterator
      bool is_root() const;
      bool empty() const;
      bool has_ancestor(const tree<T> &p) const;
      unsigned int num_children() const;

      /// get iterator to parent node
      const_tree_preorder_iterator<T> get_parent() const;
      /// get nth child
      const_tree_sibling_iterator<T> nth_child(unsigned int n) const;
      const tree<T> & nth_child_ref(unsigned int n) const;
      /// iterators
      const_tree_preorder_iterator<T> begin() const;
      const_tree_preorder_iterator<T> end() const;
      const_tree_sibling_iterator<T> sibling_begin() const;
      const_tree_sibling_iterator<T> sibling_end() const;
      const_tree_sibling_iterator<T> sibling_rbegin() const;
      const_tree_sibling_iterator<T> sibling_rend() const;
  };

  ////////////////////////////////////////////////////////////////////////////    
  ///  const iterator.  Just for convenience and homogeneity

  template<class T> class basic_const_tree_iterator : public basic_tree_iterator<T, const tree<T> > {
    friend class tree_preorder_iterator<T>;
    friend class tree_sibling_iterator<T>;
    friend class const_tree_preorder_iterator<T>;
    friend class const_tree_sibling_iterator<T>;
  };

  ////////////////////////////////////////////////////////////////////////////    
  ///  nonconst iterator.  A const iterator X (preorder or sibling)
  /// plus non-const * and ->, plus access to nonconst operations of the
  /// underlying tree
  
  template<class T> class basic_nonconst_tree_iterator : public basic_tree_iterator<T, tree<T> > {
    friend class tree_preorder_iterator<T>;
    friend class tree_sibling_iterator<T>;
    friend class const_tree_preorder_iterator<T>;
    friend class const_tree_sibling_iterator<T>;

    public:
      basic_nonconst_tree_iterator();
      ~basic_nonconst_tree_iterator();
      T& operator*() const;
      T* operator->() const;
      /// emulate operator -> for java/python/perl APIS
      T* get_info() const;

      /// get iterator to parent node
      tree_preorder_iterator<T> get_parent();
      // get iterator to nth child
      tree_sibling_iterator<T> nth_child(unsigned int);
      // get reference to nth child (Useful for Java API)
      tree<T>& nth_child_ref(unsigned int);

      /// iterators begin/end
      tree_preorder_iterator<T> begin();
      tree_preorder_iterator<T> end();
      tree_sibling_iterator<T> sibling_begin();
      tree_sibling_iterator<T> sibling_end();
      tree_sibling_iterator<T> sibling_rbegin();
      tree_sibling_iterator<T> sibling_rend();

      /// copy given tree and add it as a child
      void add_child(const tree<T>& t, bool back=true);
      /// add given tree and as a child reordering structure. NO COPIES MADE.      
      void hang_child(tree<T>& t, tree_sibling_iterator<T> where=tree_sibling_iterator<T>(NULL));
      void hang_child(basic_nonconst_tree_iterator &p, tree_sibling_iterator<T> where=tree_sibling_iterator<T>(NULL));
  };



  ////////////////////////////////////////////////////////////////////////////    
  /// basic preorder iterator. It is a basic_iterator (const or nonconst, 
  /// depending on X) plus preorder decrement and increment operations
  
  template<class T, class X> class basic_preorder_iterator : public X {
    public: 
      basic_preorder_iterator();
      basic_preorder_iterator(const basic_preorder_iterator &p);
      ~basic_preorder_iterator();

      basic_preorder_iterator<T,X> operator++(int);
      basic_preorder_iterator<T,X>& operator++();
      basic_preorder_iterator<T,X> operator--(int);
      basic_preorder_iterator<T,X>& operator--();

      // for python and other APIs
      void incr();
      void decr();
  };


  ////////////////////////////////////////////////////////////////////////////    
  /// basic sibling iterator. It is a basic_iterator (const or nonconst, 
  /// depending on X) plus sibling decrement and increment operations
  
  template<class T, class X> class basic_sibling_iterator : public X {

    public: 
      basic_sibling_iterator();
      basic_sibling_iterator(const basic_sibling_iterator &p);
      ~basic_sibling_iterator();

      basic_sibling_iterator<T,X> operator++(int);
      basic_sibling_iterator<T,X>& operator++();
      basic_sibling_iterator<T,X> operator--(int);
      basic_sibling_iterator<T,X>& operator--();

      // for python and other APIs
      void incr();
      void decr();
  };


  ////////////////////////////////////////////////////////////////////////////    
  /// preorder iterator. Inherits form basic_preorder and from basic_nonconst

  template<class T> class tree_preorder_iterator : public basic_preorder_iterator<T,basic_nonconst_tree_iterator<T> > {
    friend class tree_sibling_iterator<T>;
    friend class const_tree_preorder_iterator<T>;
    friend class const_tree_sibling_iterator<T>;
    public: 
      tree_preorder_iterator();
      tree_preorder_iterator(tree<T> *p);
      tree_preorder_iterator<T>& operator=(const tree_preorder_iterator<T> &p);
      tree_preorder_iterator(const basic_preorder_iterator<T,basic_nonconst_tree_iterator<T> > &p);
      tree_preorder_iterator(const tree_preorder_iterator<T> &p);
      tree_preorder_iterator(const tree_sibling_iterator<T> &p);
      ~tree_preorder_iterator();

  };

  ////////////////////////////////////////////////////////////////////////////    
  /// const tree_preorder iterator. Inherits form basic_preorder and from basic_const

  template<class T> class const_tree_preorder_iterator : public basic_preorder_iterator<T,basic_const_tree_iterator<T> > {
    friend class const_tree_sibling_iterator<T>;
    public: 
      const_tree_preorder_iterator();
      const_tree_preorder_iterator(tree<T> *p);
      const_tree_preorder_iterator<T>& operator=(const const_tree_preorder_iterator<T> &p);
      const_tree_preorder_iterator(const basic_preorder_iterator<T,basic_const_tree_iterator<T> > &p);
      const_tree_preorder_iterator(const basic_preorder_iterator<T,basic_nonconst_tree_iterator<T> > &p);
      const_tree_preorder_iterator(const tree<T> *p);
      const_tree_preorder_iterator(const tree_preorder_iterator<T> &p);
      const_tree_preorder_iterator(const tree_sibling_iterator<T> &p);
      const_tree_preorder_iterator(const const_tree_preorder_iterator<T> &p);
      const_tree_preorder_iterator(const const_tree_sibling_iterator<T> &p);
      ~const_tree_preorder_iterator();
  };

  ////////////////////////////////////////////////////////////////////////////    
  /// sibling iterator. Inherits form basic_sibling and from basic_nonconst

  template<class T> class tree_sibling_iterator : public basic_sibling_iterator<T,basic_nonconst_tree_iterator<T> > {
    friend class tree_preorder_iterator<T>;
    friend class const_tree_preorder_iterator<T>;
    friend class const_tree_sibling_iterator<T>;
    public: 
      tree_sibling_iterator();
      tree_sibling_iterator(tree<T> *p);
      tree_sibling_iterator<T>& operator=(const tree_sibling_iterator<T> &p);
      tree_sibling_iterator(const basic_sibling_iterator<T,basic_nonconst_tree_iterator<T> > &p);
      tree_sibling_iterator(const tree_sibling_iterator<T> &p);
      tree_sibling_iterator(const tree_preorder_iterator<T> &p);
      ~tree_sibling_iterator();
  };

  ////////////////////////////////////////////////////////////////////////////    
  /// const sibling iterator. Inherits form basic_sibling and from basic_const

  template<class T> class const_tree_sibling_iterator : public basic_sibling_iterator<T,basic_const_tree_iterator<T> > {
    friend class tree_preorder_iterator<T>;
    friend class const_tree_preorder_iterator<T>;
    public: 
      const_tree_sibling_iterator();
      const_tree_sibling_iterator(tree<T> *p);
      const_tree_sibling_iterator<T>& operator=(const const_tree_sibling_iterator<T> &p);
      const_tree_sibling_iterator(const basic_sibling_iterator<T,basic_const_tree_iterator<T> > &p);
      const_tree_sibling_iterator(const basic_sibling_iterator<T,basic_nonconst_tree_iterator<T> > &p);
      const_tree_sibling_iterator(const tree<T> *p);
      const_tree_sibling_iterator(const tree_sibling_iterator<T> &p);
      const_tree_sibling_iterator(const tree_preorder_iterator<T> &p);
      const_tree_sibling_iterator(const const_tree_sibling_iterator<T> &p);
      const_tree_sibling_iterator(const const_tree_preorder_iterator<T> &p);
      ~const_tree_sibling_iterator();
  };



  /// ===================================================
  ///  Iterator operations implementations
  /// ===================================================
  
  ///   ----------------------------------------------------------------------------------
  /// basic_tree_iterator  -----------------------------------------------------------
  template<class T, class N> basic_tree_iterator<T,N>::basic_tree_iterator() {tr=NULL;}
  template<class T, class N> basic_tree_iterator<T,N>::~basic_tree_iterator() {}  
  template<class T, class N> const T& basic_tree_iterator<T,N>::operator*() const {return *(tr->pinfo);}
  template<class T, class N> const T* basic_tree_iterator<T,N>::operator->() const {return tr->pinfo;}
  template<class T, class N> const T* basic_tree_iterator<T,N>::get_info() const {return tr->pinfo;}
  template<class T, class N> bool basic_tree_iterator<T,N>::operator==(const basic_tree_iterator<T,N> &t) const {return tr==t.tr;}
  template<class T, class N> bool basic_tree_iterator<T,N>::operator!=(const basic_tree_iterator<T,N> &t) const {return tr!=t.tr;}
  template<class T, class N> bool basic_tree_iterator<T,N>::is_defined() const {return tr!=NULL;}

  ///   ----------------------------------------------------------------------------------
  /// const tree operations accessed via the iterator   ------------------------------------------
  template<class T, class N> bool basic_tree_iterator<T,N>::is_root() const {return tr->is_root();}
  template<class T, class N> bool basic_tree_iterator<T,N>::empty() const {return tr->empty();}
  template<class T, class N> bool basic_tree_iterator<T,N>::has_ancestor(const tree<T> &p) const {return tr->has_ancestor(p);}
  template<class T, class N> unsigned int basic_tree_iterator<T,N>::num_children() const {return tr->num_children();}      
  template<class T, class N> const_tree_sibling_iterator<T> basic_tree_iterator<T,N>::nth_child(unsigned int n) const { return tr->nth_child(n);}
  template<class T, class N> const tree<T>& basic_tree_iterator<T,N>::nth_child_ref(unsigned int n) const { return tr->nth_child_ref(n);}
  template<class T, class N> const_tree_preorder_iterator<T> basic_tree_iterator<T,N>::get_parent() const {return tr->get_parent();}
  template<class T, class N> const_tree_preorder_iterator<T> basic_tree_iterator<T,N>::begin() const {return tr->begin();}
  template<class T, class N> const_tree_preorder_iterator<T> basic_tree_iterator<T,N>::end() const {return tr->end();}
  template<class T, class N> const_tree_sibling_iterator<T> basic_tree_iterator<T,N>::sibling_begin() const {return tr->sibling_begin();}
  template<class T, class N> const_tree_sibling_iterator<T> basic_tree_iterator<T,N>::sibling_end() const {return tr->sibling_end();}
  template<class T, class N> const_tree_sibling_iterator<T> basic_tree_iterator<T,N>::sibling_rbegin() const {return tr->sibling_rbegin();}
  template<class T, class N> const_tree_sibling_iterator<T> basic_tree_iterator<T,N>::sibling_rend() const {return tr->sibling_rend();}
  
  ///  ----------------------------------------------------------------------------------
  ///  basic_nonconst_tree_iterator ---------------------------------------------------------
  template<class T> basic_nonconst_tree_iterator<T>::basic_nonconst_tree_iterator() {}
  template<class T> basic_nonconst_tree_iterator<T>::~basic_nonconst_tree_iterator() {}
  template<class T> T& basic_nonconst_tree_iterator<T>::operator*() const {return *(this->tr->pinfo);}
  template<class T> T* basic_nonconst_tree_iterator<T>::operator->() const {return this->tr->pinfo;}
  template<class T> T* basic_nonconst_tree_iterator<T>::get_info() const {return this->tr->pinfo;}

  /// ----------------------------------------------------------------------------------
  /// non-const tree operations accessed via the iterator   ------------------------------------------
  template<class T> tree_sibling_iterator<T> basic_nonconst_tree_iterator<T>::nth_child(unsigned int n) { return this->tr->nth_child(n); }
  template<class T> tree<T>& basic_nonconst_tree_iterator<T>::nth_child_ref(unsigned int n) { return this->tr->nth_child_ref(n); }
  template<class T> tree_preorder_iterator<T> basic_nonconst_tree_iterator<T>::get_parent() {return this->tr->get_parent();}
  template<class T> tree_preorder_iterator<T> basic_nonconst_tree_iterator<T>::begin() {return this->tr->begin();}
  template<class T> tree_preorder_iterator<T> basic_nonconst_tree_iterator<T>::end() {return this->tr->end();}
  template<class T> tree_sibling_iterator<T> basic_nonconst_tree_iterator<T>::sibling_begin() {return this->tr->sibling_begin();}
  template<class T> tree_sibling_iterator<T> basic_nonconst_tree_iterator<T>::sibling_end() {return this->tr->sibling_end();}
  template<class T> tree_sibling_iterator<T> basic_nonconst_tree_iterator<T>::sibling_rbegin() {return this->tr->sibling_rbegin();}
  template<class T> tree_sibling_iterator<T> basic_nonconst_tree_iterator<T>::sibling_rend() {return this->tr->sibling_rend();}
  template<class T> void basic_nonconst_tree_iterator<T>::add_child(const tree<T>& t,bool back) {this->tr->add_child(t,back);}
  template<class T> void basic_nonconst_tree_iterator<T>::hang_child(tree<T>& t, tree_sibling_iterator<T> where) {this->tr->hang_child(t,where);}
  template<class T> void basic_nonconst_tree_iterator<T>::hang_child(basic_nonconst_tree_iterator<T>& p, tree_sibling_iterator<T> where) {this->tr->hang_child(*p.tr,where);}


  ///   ----------------------------------------------------------------------------------
  /// basic_preorder_iterator   -----------------------------------------------------------
  template<class T, class X> basic_preorder_iterator<T,X>::basic_preorder_iterator() {}
  template<class T, class X> basic_preorder_iterator<T,X>::basic_preorder_iterator(const basic_preorder_iterator<T,X> &p) {this->tr=p.tr;}
  template<class T, class X> basic_preorder_iterator<T,X>::~basic_preorder_iterator() {}  

  /// postincrement
  template<class T, class X> basic_preorder_iterator<T,X> basic_preorder_iterator<T,X>::operator++(int) {
    basic_preorder_iterator<T,X> b = (*this);
    ++(*this);
    return b;
  }
  /// preincrement
  template<class T, class X> basic_preorder_iterator<T,X>& basic_preorder_iterator<T,X>::operator++() {
    if (this->tr->first != NULL) 
      this->tr = this->tr->first;
    else {
      while (this->tr!=NULL && this->tr->next==NULL) 
        this->tr = this->tr->parent;
      if (this->tr!=NULL) this->tr = this->tr->next;
    }
    return *this;
  }

  /// postdecrement
  template<class T, class X> basic_preorder_iterator<T,X> basic_preorder_iterator<T,X>::operator--(int) {
    basic_preorder_iterator<T,X> b = (*this);
    --(*this);
    return b;
  }
  /// predecrement
  template<class T, class X> basic_preorder_iterator<T,X>& basic_preorder_iterator<T,X>::operator--() {
    if (this->tr->prev == NULL) 
      this->tr = this->tr->parent;
    else {
      this->tr = this->tr->prev;
      while (this->tr->nchildren>0)
        this->tr = this->tr->last;
    }
    return *this;
  }

  // increment/decrement in APIs for  python et al.
  template<class T, class X> void basic_preorder_iterator<T,X>::incr() { ++(*this); }
  template<class T, class X> void basic_preorder_iterator<T,X>::decr() { --(*this); }


  ///   ----------------------------------------------------------------------------------
  /// basic_sibling_iterator   -----------------------------------------------------------
  template<class T, class X> basic_sibling_iterator<T,X>::basic_sibling_iterator() {}
  template<class T, class X> basic_sibling_iterator<T,X>::basic_sibling_iterator(const basic_sibling_iterator<T,X> &p) {this->tr=p.tr;}
  template<class T, class X> basic_sibling_iterator<T,X>::~basic_sibling_iterator() {}  

  // postincrement
  template<class T, class X> basic_sibling_iterator<T,X> basic_sibling_iterator<T,X>::operator++(int) {
    basic_sibling_iterator<T,X> b = (*this);
    ++(*this);
    return b;
  }
  // preincrement
  template<class T, class X> basic_sibling_iterator<T,X>& basic_sibling_iterator<T,X>::operator++() {
    if (this->tr!=NULL) this->tr = this->tr->next;
    return *this;
  }
  // postdecrement
  template<class T, class X> basic_sibling_iterator<T,X> basic_sibling_iterator<T,X>::operator--(int) {
    basic_sibling_iterator<T,X> b = (*this);
    --(*this);
    return b;
  }
  // predecrement
  template<class T, class X> basic_sibling_iterator<T,X>& basic_sibling_iterator<T,X>::operator--() {
    if (this->tr!=NULL) this->tr = this->tr->prev;
    return *this;
  }
  // increment/decrement in APIs for  python et al.
  template<class T, class X> void basic_sibling_iterator<T,X>::incr() { ++(*this); }
  template<class T, class X> void basic_sibling_iterator<T,X>::decr() { --(*this); }


  ///   ----------------------------------------------------------------------------------
  /// tree_preorder_iterator   -----------------------------------------------------------
  template<class T> tree_preorder_iterator<T>::tree_preorder_iterator() {}
  template<class T> tree_preorder_iterator<T>::tree_preorder_iterator(tree<T> *p) {this->tr = p;}
  template<class T> tree_preorder_iterator<T>& tree_preorder_iterator<T>::operator=(const tree_preorder_iterator<T> &p) {
    if (this!=&p) this->tr = p.tr;
    return (*this);
  }
  template<class T> tree_preorder_iterator<T>::tree_preorder_iterator(const basic_preorder_iterator<T,basic_nonconst_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> tree_preorder_iterator<T>::tree_preorder_iterator(const tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> tree_preorder_iterator<T>::tree_preorder_iterator(const tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> tree_preorder_iterator<T>::~tree_preorder_iterator() {}

  ///   ----------------------------------------------------------------------------------
  /// const_tree_preorder_iterator   -----------------------------------------------------------
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator() {}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(tree<T> *p) {this->tr = p;}
  template<class T> const_tree_preorder_iterator<T>& const_tree_preorder_iterator<T>::operator=(const const_tree_preorder_iterator<T> &p) {
    if (this!=&p) this->tr = p.tr;
    return (*this);
  }
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const basic_preorder_iterator<T,basic_const_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const basic_preorder_iterator<T,basic_nonconst_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const tree<T> *p) {this->tr = p;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const const_tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::const_tree_preorder_iterator(const const_tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_preorder_iterator<T>::~const_tree_preorder_iterator() {}


  ///   ----------------------------------------------------------------------------------
  /// tree_sibling_iterator   -----------------------------------------------------------
  template<class T> tree_sibling_iterator<T>::tree_sibling_iterator() {}
  template<class T> tree_sibling_iterator<T>::tree_sibling_iterator(tree<T> *p) {this->tr = p;}
  template<class T> tree_sibling_iterator<T>& tree_sibling_iterator<T>::operator=(const tree_sibling_iterator<T> &p) {
    if (this!=&p) this->tr = p.tr;
    return (*this);
  }
  template<class T> tree_sibling_iterator<T>::tree_sibling_iterator(const basic_sibling_iterator<T,basic_nonconst_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> tree_sibling_iterator<T>::tree_sibling_iterator(const tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> tree_sibling_iterator<T>::tree_sibling_iterator(const tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> tree_sibling_iterator<T>::~tree_sibling_iterator() {}


  ///   ----------------------------------------------------------------------------------
  /// const_tree_sibling_iterator   -----------------------------------------------------------
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator() {}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(tree<T> *p) {this->tr = p;}
  template<class T> const_tree_sibling_iterator<T>& const_tree_sibling_iterator<T>::operator=(const const_tree_sibling_iterator<T> &p) {
    if (this!=&p) this->tr = p.tr;
    return (*this);
  }
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const basic_sibling_iterator<T,basic_const_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const basic_sibling_iterator<T,basic_nonconst_tree_iterator<T>> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const tree<T> *p) {this->tr = p;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const const_tree_sibling_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::const_tree_sibling_iterator(const const_tree_preorder_iterator<T> &p) {this->tr = p.tr;}
  template<class T> const_tree_sibling_iterator<T>::~const_tree_sibling_iterator() {}


} // namespace

#endif
