#pragma once
#include <iostream>
#include <utility>
#include <iterator>
#include <vector>
#include "stack_iterator.hpp"
#include "ap_error.hpp"


/**
*	@file stack_pool.hpp
*	@brief Header file: implementation of class stack_pool, our pool of blazingly fast stacks
*/


/**
* Class \p stack_pool: pool of stacks, data structures compliant with the LastInFirstOut rule.
*
* A stack of nodes is a data structure implementing the LastInFirstOut rule,
* stating that the last added element (node) will also be the first one removed.
* The only allowed insertions/removals occur in fact in the front of the stack,
* and are implemented through the \p push() and \p pop() methods. \n
* The proposed implementation employs an std::vector as support of the pool,
* exploiting its indexing to provide a simple yet effective identification method for
* nodes and stacks: each node will be identified by its index on the vector + 1,
* each stack by its first node's index, referred to as head. \n The nodes themselves
* are objects of type node: nested in the \p stack_pool it's in fact implemented the
* class representing the concept of node, \p node_t, a simple structure carrying a value and the
* index of the next node. The nested implementation of the class \p node_t was the most sensible
* choice due to its templates being the same of \p stack_pool, along with its reduced size
* and close connection with the latter. \n Going back to templates, \p stack_pool  has two
* templates, allowing the user to choose the desired type for both
* the values carried by the nodes and their indexes. \n Also, the aim is building a
* blazingly fast data structure and to this end the implementations tries to
* mitigate two common bottlenecks caused by the slow, slow memory: the allocation
* of the elements one by one and the distance between them. The first issue
* is mitigated by the employment of methods allowing to reserve a certain
* memory region upfront, the second by the very use of \p std::vector<node_t>,
* allowing to keep the nodes organized and close to each other. \n But why,
* as I spoiled before, the nodes are indexes as their real index on the node + 1?
* The answer is simple: convenience. This indexing system in fact allows to use
* index 0 (not a real index in the vector, would be -1) as proxy for the end of
* the stack: if the next index is 0 the current node is the last
* one, if the head is 0 the stack is empty. \n How does the implementation
* deal with stacks resizing? When the size increases, hence
* when nodes are added, the std::vector takes care of the possible need
* to increase its capacity; when the size decreases, so when nodes are removed,
* the std::vector slots previously owned by the shrinked stack are not left
* unused: they are added to a stack of free nodes (free indexes) which will
* to be occupied by the next newly added nodes. Free nodes have the
* priority over never used std::vector slots, which will begin to be filled
* only when no more free nodes result available. \n
* \n
* Templates guidelines: the class has been designed with N being an unsigned integral type in mind,
* since indicating indexes this makes the most sense (there is no type check: be kind to yourself, don't use clearly unsuitable types!). \n
* Notice: the choice of different types will impact the class in the following ways: \n
*   - small types: better performances; many methods are implemented by passing arguments by value, which is cheaper then passing references if the ints are small.
*   - large types: larger pool, since the correct implementation of the pool is possible as long as there are enough indexes to represent the nodes
* @tparam T type of the values carried by each node
* @tparam N stack/index type
*/

template <typename T, typename N = std::size_t>
class stack_pool{

  /** Class \p node_t, implementing the concept of node of a stack*/
  struct node_t{
    /** value of type T carried by the node */
    T value;

    /** index to the next node, type N */
    N next;


    /** Custom constructor taking l-value: initializes \p value and \p index with the passed values.
    * Does not throw: upstream checks done by \p stack_pool<T,N>::_push(), which is the only one able to call it
    * @param v const reference to the value the node will carry
    * @param index index of the next node
    */
    node_t(const T& v, N index) noexcept
      :value{v},
       next{std::move(index)}
       {}


    /** Custom constructor taking r-value: initializes \p value and \p index with the passed values.
    * Does not throw: upstream checks done by \p stack_pool<T,N>::_push(), which is the only one able to call it
    * @param v r-value, indicating the value the node will carry
    * @param index index of the next node
    */
    node_t(T&& v, N index) noexcept
      :value{std::move(v)},
       next{std::move(index)}
       {}


    /**Default destructor, explicitly = default*/
    ~node_t()noexcept = default;

  };

  using value_type = T; // type of the values
  using stack_type = N; // type of the stacks indexes
  using size_type = typename std::vector<node_t>::size_type; //"type suitable for holding the size of the vector"
  using pool_type = stack_pool<value_type, stack_type>;


  /**std::vector of nodes, the support of the pool.
   * Initialized by the default constructor of vector */
  std::vector<node_t> pool;


  /**Stack of free nodes.
   * At the beginning it's empty. The nodes previously belonging
   * to a stack will be added to this stack, which needs to be
   * emptied before new nodes are added increasing the size of the vector*/
  stack_type free_nodes;



 public:

  /**Default constructor, sets free_nodes as empty. */
  stack_pool() noexcept
    : free_nodes{end()}{}


  /** Custom constructor, reserves n nodes in the pool, sets free_nodes as empty.
  * Notice, the nodes are reserved but not constructed. Reserving nodes allows
  * to avoid reallocation each time the capacity of the vector is reached. \n
  * \p std::vector<T>::reserve(...) throws in case insufficient memory is available
  * @param n number of nodes to reserve
  */
  explicit stack_pool(size_type n)
    : stack_pool()
    {pool.reserve(n);}




  //____________Iterators_Domain___________________________________________//

  using iterator = _stack_iterator<value_type, stack_type, pool_type>;
  using const_iterator = _stack_iterator<const value_type, stack_type, const pool_type>;


  /** Function providing the iterator to the first element of the stack.
  * Easy way to obtain the iterator to the first element, without the need of coding its instantiation. \n
  * Keyword this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * Throws through the constructor of _stack_iterator<> if the given index is larger than \p psize(), hence not a plausible index for any stack
  * @param x head of the stack
  * @return iterator to the first element
  */
  iterator begin(stack_type x){
    return iterator{x, this};
    }

  /** Function providing the iterator to the last element of the stack.
  * Since the proxy end of the stack is simply the zero element, the function returns an iterator to \p end();
  * no parameter is passed since any passed index would remain unused, in this way the warnings are avoided and there's no throwing risk. \n
  * Keyword \p this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * @return iterator to the proxy last element
  */
  iterator end(stack_type )noexcept{
    return iterator{end(), this};
    }


  /** Overloaded begin function providing a const iterator to the first element of the stack.
  * Easy way to obtain the iterator to the first element, without the need of coding its instantiation. \n
  * Keyword \p this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * Throws through the constructor of _stack_iterator<> if the given index is larger than \p psize(), hence not a plausible index for any stack
  * @param x head of the stack
  * @return const iterator to the first element
  */
  const_iterator begin(stack_type x) const{
    return const_iterator{x, this};
    }

  /** Overloaded end function providing the iterator to the last element of the stack.
  * Since the proxy end of the stack is simply the zero element, the function returns an iterator to \p end();
  * no parameter is passed since any passed index would remain unused, in this way the warnings are avoided and there's no throwing risk. \n
  * Keyword \p this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * @return const iterator to the proxy last element
  */
  const_iterator end(stack_type ) const noexcept{
    return const_iterator{end(), this};
    }


  /** Constant begin function providing a const iterator to the first element of the stack.
  * Easy way to obtain the iterator to the first element, without the need of coding its instantiation. \n
  * Keyword \p this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * Throws through the constructor of _stack_iterator<> if the given index is larger than \p psize(), hence not a plausible index for any stack
  * @param x head of the stack
  * @return const iterator to the first element
  */
  const_iterator cbegin(stack_type x) const{
    return const_iterator{x, this};
    }

  /** Constant end function providing the iterator to the last element of the stack.
  * Since the proxy end of the stack is simply the zero element, the function returns an iterator to \p end();
  * no parameter is passed since any passed index would remain unused, in this way the warnings are avoided and there's no throwing risk. \n
  * Keyword \p this is used to point at the object, in order to maintain the connection between the pool and the iterator. \n
  * @return const iterator to the proxy last element
  */
  const_iterator cend(stack_type ) const noexcept{
    return const_iterator{end(), this};
    }



  //____________Get_To_Know_The_Pool______________________________________//


  /** Function providing the proxy index of the end of the stacks.
  * @return element 0 casted in the correct way to \p stack_type
  */
  stack_type end() const noexcept {
     return stack_type(0);
     }


  /** Function providing the head of a new empty stack.
  * @return head of the empty new stack which is always \p end()
  */
  stack_type new_stack() noexcept{
    return end();
  }


  /** Function providing access to the node value at the given index.
  * The function throws if the given index is equal to \p end() or larger than \p psize(), since at these indexes there is no value at all
  * @param x index of a node
  * @return reference to the value of the node identified by \p x
  */
  T& value(stack_type x){
    AP_ERROR_IN_RANGE(x, end(), psize());
    return node(x).value;
  }

  /** Constant function providing access to the node value at the given index.
  * The function throws if the given index is equal to \p end() or larger than \p psize(), since at these indexes there is no value at all
  * @param x index of a node
  * @return constant reference to the value of the node identified by \p x
  */
  const T& value(stack_type x) const{
    AP_ERROR_IN_RANGE(x, end(), psize());
    return node(x).value;
  }


  /** Function providing access to the node next element at the given index.
  * The function throws if the given index is equal to \p end() or larger than \p psize(), since at these indexes there is no next at all
  * @param x index of a node
  * @return reference to the next index of the node identified by \p x
  */
  stack_type& next(stack_type x){
    AP_ERROR_IN_RANGE(x, end(), psize());
    return node(x).next;
  }

  /** Constant function providing access to the node next element at the given index.
  * The function throws if the given index is equal to \p end() or larger than \p psize(), since at these indexes there is no next at all
  * @param x index of a node
  * @return constant reference to the next index of the node identified by \p x
  */
  const stack_type& next(stack_type x) const{
    AP_ERROR_IN_RANGE(x, end(), psize());
    return node(x).next;
  }


  /** Function allowing to reserve n nodes in an already present pool.
  * It behaves as \p std::vector<T>::reserve(...), requesting that the vectory capacity should be at least equal to n. \n
  * \p std::vector<T>::reserve(...) throws in case insufficient memory is available
  * @param n number of required nodes
  */
  void reserve(size_type n){
    pool.reserve(n);
  }


  /** Function allowing to assess the current capacity of the pool.
  * It does not throw since \p std::vector<T>::capacity() is no-throw guaranteed. \n
  * @return the pool capacity
  */
  size_type capacity() const noexcept{
    return pool.capacity();
  }


  /** Function allowing to assess the current size of the pool.
  * Notice that the size of the pool is the sum of the nodes in the stacks and in free_nodes. \n
  * It does not throw since \p std::vector<T>::size() is no-throw guaranteed. \n
  * @return the pool capacity
  */
  size_type psize() const noexcept{
    return pool.size();
  }


  /** Function allowing to assess whether the given stack is empty or not.
  * In order to check if a stack is empty or not it's enough to check
  * if the head is equal to \p end()
  * @return true if the stack is empty, false if it's not
  */
  bool empty(stack_type x) const noexcept{
    return (x==end());
  }



  //___________________FInally_Use_The_Pool_______________________________________//



  /** Function taking l-value references able to add a node to the stack.
  * Calls the auxiliary function \p _push(), and throws through it.
  * @param val constant reference to the value of the new node
  * @param head current head of the stack, will be the next of the new node
  * @return the new head of the stack after adding the new node
  */
  stack_type push(const T& val, stack_type head){
    return _push(val, head);
  }

  /** Function taking r-value references able to add a node to the stack.
  * Calls the auxiliary function \p _push(), and throws through it.
  * @param val r-value reference to the value of the new node
  * @param head current head of the stack, will be the next of the new node
  * @return the new head of the stack after adding the new node
  */
  stack_type push(T&& val, stack_type head){
    return _push(std::move(val), head);
  }


  /** Function that deleted the first node of the given stack.
  * The function takes the head of the stack as a parameter and pops the first element. \b Be \b careful,
  * if an index different from a head is supplied to the function a node imbetween the stack will be deleted! \n
  * The removed node is added to free_nodes. \n
  * The function works by assigning to free_nodes the original head, to the original head the original next element of the first
  * node, and to the latter the original free_nodes.
  * See \p supplementary_materials \n
  * Calls the auxiliary function \p _new_first and throws through it.
  * @param head of the stack, index of the node to be removed
  * @return the new head of the stack after removing the first node
  */
  stack_type pop(stack_type x){
    _new_first(free_nodes, x);
    return x;
  }


  /** Function that deletes an entire stack.
  * The function takes the head of the stack as a parameter and sets the stack to \n end(). \b Be \b careful,
  * if an index different from a head is supplied to the function the portion of stack up to the pointed node will be deleted! \n
  * The removed stack is added to free_nodes. \n
  * - If free_nodes is empty, the function simply works by assigning it to head before setting the latter to \p end()
  * - If free_nodes is not empty, the auxiliary function \p _last_jump() is called, returning the next element of the last
  * node and assigning it to head, before setting the latter to \p end(); the choice of reaching the end of free_nodes
  * instead of the one of the given stack and do the opposite procedure is due to the fact that free_nodes is bound to
  * shrink, while stacks to increase, so in the majority of the cases free_nodes should be shorter than the stacks,
  * which is relevant since we have to travel up to its last node.
//  * See \p supplementary_materials \n
  * The function throws if \p x is larger than \p psize() since assigning to free nodes an index not pointing to nodes
  * would not makes sense and would cause troubles with free_nodes.
  * @param head of the stack, index of the stack to remove
  * @return the new head of the freed stack, always \p end()
  */
  stack_type free_stack(stack_type x){
    AP_ERROR_IN_RANGE(x, end(), psize());
    if (!empty(free_nodes)){
      _last_jump(free_nodes)=std::move(x);
    }else{
      free_nodes = std::move(x);
    }
    return end();
  }




  //___________________Explore_Your_Stacks_______________________________________//


  /** Function allowing to assess the size of the given stack.
  * \b Be \b careful, if an intermediate index is passed instead of
  * a head the function only provides a partial size, not the entire size of the stack. \n
  * The function throws through \p next(): if \p x is larger than \p psize() the
  * operator++ of the iterator calls \p next() which fails if there is no next element. \n
  * If \p x is equal to \p end() the function returns size zero
  * @param x stack index
  * @return the size of the stack
  */
  size_type ssize(stack_type x) const{
    size_type i=0;
    auto c_end{cend(x)};
    for (auto this_node=cbegin(x); this_node!=c_end; ++this_node){
      ++i;
    }
    return i;
  }


  /** Function allowing to reach the value of the \b mth node in the stack.
  * The function allows to access the value of the mth node in the stack, where the
  * first node: \p m=1 and the last node: \p m=ssize(x) \n
  * The type of m is \p stack_type to be coherent with the order of magnitude of the stack size. \n
  * \b Be \b careful, if an intermediate index is passed instead of
  * a head the function considers m=1 the node at the current index. \n
  * - Throws through \p next() or \p value() if x is equal to \p end() or larger than \p psize()
  * - Throws if the passed m is larger than \p ssize() since this would lead to the previous exception
  * If m is equal to end() hence 0 in stack_type type, the function returns the value of the first node
  * @param x stack index
  * @param m the hierarchical number of a node, going from the first (1) to the last ( \p ssize(x) )
  * @return reference to the value of the reached node
  */
  value_type& reach(stack_type x, stack_type m) {
    AP_ERROR_IN_RANGE(m, end(), ssize(x));
    for (stack_type i=1; i<m; ++i){
      x=next(x);
    }
    return value(x);
  }

  /** Constant function allowing to reach the value of the \b mth node in the stack.
  * The function allows to access the value of the mth node in the stack, where the
  * first node: \p m=1 and the last node: \p m=ssize(x) \n
  * The type of m is \p stack_type to be coherent with the order of magnitude of the stack size. \n
  * \b Be \b careful, if an intermediate index is passed instead of
  * a head the function considers m=1 the node at the current index. \n
  * - Throws through \p next() or \p value() if x is equal to \p end() or larger than \p psize()
  * - Throws if the passed m is larger than \p ssize() since this would lead to the previous exception
  * If m is equal to end() hence 0 in stack_type type, the function returns the value of the first node
  * @param x stack index
  * @param m the hierarchical number of a node, going from the first (1) to the last ( \p ssize(x) )
  * @return const reference to the value of the reached node
  */
  const value_type& reach(stack_type x, stack_type m) const {
    AP_ERROR_IN_RANGE(m, end(), ssize(x));
    for (stack_type i=1; i<m; ++i){
      x=next(x);
    }
    return value(x);
  }


  /** Constant function printing the value of each node in the stack.
  * \b Be \b careful, if an intermediate index is passed instead of
  * The function throws through \p next(): if \p x is larger than \p psize() the
  * operator++ of the iterator calls \p next() which fails if there is no next element. \n
  * @param x stack index
  */
  void print_stack(stack_type x) const{
    auto c_end{cend(x)};
    for (auto this_node{cbegin(x)}; this_node!=c_end; ++this_node){
      std::cout<<*this_node<<"\n";
    }
  }


 private:

  /**Function providing the node at a given index.
   * @param x "stack index" of a node, which is real index + 1
   * @return the node at the correct index
   */
  node_t& node(stack_type x) noexcept {
    return pool[x-1]; }

  /**Constant function providing the node at a given index.
   * @param x "stack index" of a node, which is real index + 1
   * @return the node at the correct index
   */
  const node_t& node(stack_type x) const noexcept {
    return pool[x-1]; }


  /**Templated auxiliary function allowing to add 1 node using the public method push.
   * The function can take both r-values and r-value while being able to correctly
   * identify and treat them. \n
   * If free_nodes it's empty the function proceeds to add the node at the end of the pool. \n
   * If free_nodes it's not empty: the first node of free_nodes becomes the new node and free_nodes
   * shrinks, while the next element of the already gone first element becomes its new head.
   * See \p supplementary_materials. \n
   * The function throws through push_back and through _new_first (in case \n head is equal to \p end() or larger than \p psize())
   * @param V deduced from the arguments passed to the function
   * @param val universal reference to the value of the new node
   * @param head current head of the stack, index that will become the next of the new node
   * @return the new head of the stack hence index of the new node
   */
  template <typename V>
  stack_type _push(V&& val, stack_type head) {
    if (empty(free_nodes)){
      pool.push_back(node_t{std::forward<V>(val), head});
      return static_cast<stack_type>(pool.size());
    }
    _new_first(head, free_nodes);
    value(head)=std::forward<V>(val);
    return static_cast<stack_type>(head);
  }


  /**Auxiliary function allowing to transfer the ownership of the first node from a stack to another.
   * The first node of \p stack2 (head \p s2) becomes the new first node (head \p s1) of \p stack1; to do that
   * the following movements occur "in parallel":
   * - s1 becomes s2
   * - s2 becomes next(s2)
   * - next(s2) becomes s1
   * See \p supplementary_materials \n
   * The function throws through \p next() if s2 is equal to \p end() or larger than \p psize() since there are no nodes there. \n
   * Throws "de novo" if s1 is larger than \p psize(); no problem if s1 is equal to \p end() \n
   * When the function is accessed through _push, we can be sure that at least the first condition is respected since it's accessed if !(empty(free_nodes))
   * @tparam V deduced from the arguments passed to the function
   * @param val universal reference to the value of the new node
   * @param head index that will become the next of the new node
   * @return the new head of the stack hence index of the new node
   */
  void _new_first(stack_type& s1, stack_type& s2){
    AP_ERROR_IN_RANGE(s1, end(), psize());
    auto tmp=s2;
    s2=next(s2);
    next(tmp)= s1;
    s1=tmp;
  }


  /**Auxiliary function allowing to easily reach the \p next element of the last node of the passed stack.
   * See \p supplementary_materials \n
   * The function does not throw since it's only passed the head of non-empty free_nodes:
   * - \p x is not equal to \p end()
   * - \p x is not larger than \p psize()
   * Notice, the function does not modify the object itself, but it's tailored to return a value so that it can be modified,
   * hence the const qualification would not suit the intents of the function.
   * @param x head of a stack, index
   * @return reference to the last node's \p next() element, always equal to \p end()
   */
  stack_type& _last_jump(stack_type x) noexcept{
      while (!empty(next(x))) {
        x=next(x);     //hope in NRVO
      }
      return next(x);
    }


};










