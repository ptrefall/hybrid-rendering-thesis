/*
 *  Copyright 2008-2012 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <thrust/detail/type_traits.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/iterator/iterator_adaptor.h>
#include <thrust/iterator/iterator_facade.h>

namespace thrust
{

// forward declaration of use_default
struct use_default;

namespace experimental
{

namespace detail
{

// If T is use_default, return the result of invoking
// DefaultNullaryFn, otherwise return T.
// XXX rename to dflt_help
template <class T, class DefaultNullaryFn>
struct ia_dflt_help
  : thrust::detail::eval_if<
        thrust::detail::is_same<T, thrust::use_default>::value
      , DefaultNullaryFn
      , thrust::detail::identity_<T>
    >
{
}; // end ia_dflt_help


// A metafunction which computes an iterator_adaptor's base class,
// a specialization of iterator_facade.
template <
    typename Derived
  , typename Base
  , typename Pointer
  , typename Value
  , typename Space
  , typename Traversal
  , typename Reference
  , typename Difference
>
  struct iterator_adaptor_base
{
  typedef typename ia_dflt_help<
    Value,
    iterator_value<Base>
  >::type value;

  typedef typename ia_dflt_help<
    Space,
    thrust::iterator_space<Base>
  >::type space;

  typedef typename ia_dflt_help<
    Traversal,
    thrust::iterator_traversal<Base>
  >::type traversal;

  typedef typename ia_dflt_help<
    Reference,
    thrust::detail::eval_if<
      thrust::detail::is_same<Value,use_default>::value,
      thrust::iterator_reference<Base>,
      thrust::detail::add_reference<Value>
    >
  >::type reference;

  typedef typename ia_dflt_help<
    Difference,
    iterator_difference<Base>
  >::type difference;

  typedef iterator_facade<
    Derived,
    Pointer,
    value,
    space,
    traversal,
    reference,
    difference
  > type;
}; // end iterator_adaptor_base

} // end detail

} // end experimental

} // end thrust

