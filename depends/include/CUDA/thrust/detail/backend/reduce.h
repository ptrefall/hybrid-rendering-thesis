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


/*! \file reduce.h
 *  \brief Entry points for reduce backend.
 */

#pragma once

#include <thrust/pair.h>

namespace thrust
{
namespace detail
{
namespace backend
{

template<typename InputIterator, 
         typename OutputType,
         typename BinaryFunction>
  OutputType reduce(InputIterator first,
                    InputIterator last,
                    OutputType init,
                    BinaryFunction binary_op);

template <typename InputIterator1,
          typename InputIterator2,
          typename OutputIterator1,
          typename OutputIterator2,
          typename BinaryPredicate,
          typename BinaryFunction>
  thrust::pair<OutputIterator1,OutputIterator2>
  reduce_by_key(InputIterator1 keys_first, 
                     InputIterator1 keys_last,
                     InputIterator2 values_first,
                     OutputIterator1 keys_output,
                     OutputIterator2 values_output,
                     BinaryPredicate binary_pred,
                     BinaryFunction binary_op);

} // end namespace backend
} // end namespace detail
} // end namespace thrust

#include <thrust/detail/backend/reduce.inl>

