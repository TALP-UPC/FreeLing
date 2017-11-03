/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_list.i
 * ----------------------------------------------------------------------------- */

%include <std_common.i>
//%include test.i

%{
#include <list>
#include <stdexcept>

// Iterator class to allow stl list iterators to java, getting references to contained objects
  template<class T> class ListRefIterator  {
  private:
    list<T>* _list;
    typename std::list<T>::iterator _iter;
    
  public:
    ListRefIterator(std::list<T>* original) {
      this->_list = original;
      this->_iter = this->_list->begin();
    }
    bool hasNext() const {
      return this->_iter != this->_list->end();
    }

    T& next() {
      T& ret = (T&) *this->_iter;
      this->_iter++;
      return ret;
    }
  };

// Iterator class to allow stl list iterators to java, getting copies of contained objects
  template<class T> class ListCopyIterator  {
  private:
    list<T>* _list;
    typename std::list<T>::iterator _iter;
    
  public:
    ListCopyIterator(std::list<T>* original) {
      this->_list = original;
      this->_iter = this->_list->begin();
    }
    bool hasNext() const {
      return this->_iter != this->_list->end();
    }

    T next() {
      T ret = *this->_iter;
      this->_iter++;
      return ret;
    }
  };
 
%}


namespace std {    
  template<class T> class list {
  public:
    typedef size_t size_type;
    typedef T value_type;
    typedef const value_type& const_reference;
    list();
    list(size_type n);
    size_type size() const;
    %rename(isEmpty) empty;
    bool empty() const;
    void clear();
    void reverse();
    %rename(addFirst) push_front;
    void push_front(const value_type& x);
    %rename(addLast) push_back;
    void push_back(const value_type& x);
    %rename(getFirst) front;
    const_reference front();
    %rename(getLast) back;
    const_reference back();
    %rename(clearLast) pop_back;
    void pop_back();
    %rename(clearFirst) pop_front;
    void pop_front();
  };
}

template<class T> class ListRefIterator  {
 public:
  ListRefIterator(std::list<T>* original);   
  bool hasNext();   
  T& next();
};

template <class T> class ListCopyIterator  {
 public:
  ListCopyIterator(std::list<T>* original);   
  bool hasNext();   
  T next();
};


%define specialize_std_list(T)
#warning "specialize_std_list - specialization for type T no longer needed"
%enddef

