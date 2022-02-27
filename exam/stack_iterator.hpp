#pragma once
#include <iostream>
#include <utility>
#include <iterator>
#include "ap_error.hpp"


/**
*	@file stack_iterator.hpp
*	@brief Header file: implementation of class _stack_iterator, the iterator for the class stack_pool
*/


/**
* Class \p _stack_iterator: iterator allowing to navigate stacks in the stack_pool data structure.
* Notice: the class allows to iterate through a single stack at a time!
*
* @tparam T type of the values carried by each node.
* @tparam N stack/index type
* @tparam S_P stack_pool type, templating the function on the type it's supposed to work on
*/
template <typename T, typename N, typename S_P >
class _stack_iterator {

  using pool_type = S_P;
  using stack_type = N;

  /** Pointer to stack_pool, will store the passed pool address.*/
  pool_type* pool_ptr;
  /** Variable of type stack_type that will store the passed index.*/
  stack_type index;


 public:
  using value_type = T;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;


  /** Custom constructor: initializes \p pool_ptr and \p index with the passed values.
  * Throws if my_pool is of type pool_type but points to nothing. Don't do that! \n
  * Given the previous does not happen, throws if the index is not in the pool, hence
  * larger than \p pool_ptr -> psize() since up to size() all nodes belong to a stack or to free_nodes,
  * hence could be successfully used to build an iterator
  * @param x index value
  * @param my_pool pointer to the stack_pool, it's a constant pointer
  */
  _stack_iterator(stack_type x, pool_type* const my_pool) :
    pool_ptr{my_pool},
    index{std::move(x)}{
      AP_ERROR(pool_ptr!=nullptr) << "The pointer to stack_pool points to no pool :( why did you do that? "<< std::endl;
      AP_ERROR_IN_RANGE(x, (*pool_ptr).end(), (*pool_ptr).psize());
    }


  /** Default destructor.*/
		~_stack_iterator() noexcept = default;


  /** Dereference operator.
  * Given that the object already passed from the constructor so initial index and pool_ptr are fine,
  * there's a problem if the index is end() or reaches it due to the increment of the iterator.
  * See \p stack_pool<T,N>::value().
  * @return value of the node at index
  */
  reference operator*() const {
    return pool_ptr -> value(index);
    }


  /** Reference operator.
  * Throws if problems from \p operator*
  * @return address of the value of the node at index (pointer)
  */
  pointer operator ->() const {
    return &**this;
    }


  /** PreIncrement: increments.
  * Given that the object already passed from the constructor so initial index and pool_ptr are fine,
  * there's a problem if the index is end() or reaches it due to the increment of the iterator.
  * See \p stack_pool<T,N>::next().
  * @return the incremented iterator
  */
  _stack_iterator& operator++() {
    index = pool_ptr -> next(index);
    return *this;
  }


  /** PostIncrement: increments.
  * Exceptions due to \p operator++().
  * @return the iterator
  */
  _stack_iterator& operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }


  /** Equality operator overload.
  * @param x reference to an iterator
  * @param y reference to another iterator
  * @return a boolean: true (iterators at the same node) or false (iterators at different nodes)
  */
  friend bool operator==(const _stack_iterator& x, const _stack_iterator& y) {
    return x.index == y.index;
  }


  /** Inequality operator overload.
  * @param x reference to an iterator
  * @param y reference to another iterator
  * @return a boolean: false (iterators at the same node) or true (iterators at different nodes)
  */
  friend bool operator!=(const _stack_iterator& x, const _stack_iterator& y) {
    return !(x == y);
  }


  /** Put-to operator overload.
  * @param os reference to output stream
  * @param si reference to iterator
  * @return output stream
  */
  friend std::ostream& operator<<(std::ostream& os, const _stack_iterator& si) {
    os <<"Pointed pool: " <<si.pool_ptr << "Index: "<< si.index<< std::endl;
    return os;
  }

};

