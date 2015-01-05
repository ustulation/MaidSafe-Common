/*  Copyright 2014 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */


/*
 * There two flavours of serialization and de-serialization functions here. One that works on
 * strings and the other that works on streams. This is because the string ones create
 * stringstreams as local variables and stringstreams being locale aware have poor contruction
 * speed. Thus if the client code wants to provide streams himself so that he can cache and reuse
 * the streams, the functions which directly operate on streams can be used.
 */

#ifndef MAIDSAFE_COMMON_SERIALISATION_SERIALISATION_H_
#define MAIDSAFE_COMMON_SERIALISATION_SERIALISATION_H_

#include <utility>

#include "maidsafe/common/serialisation/stream_policies/string_stream_policy.h"
#include "maidsafe/common/serialisation/stream_policies/boost_interprocess_stream_policy.h"

namespace maidsafe {

template<template<typename> class SerialisationPolicy, typename StreamPolicy>
struct Serialisation : public SerialisationPolicy<StreamPolicy> {
  // ---------------------------------------
  //                Typedefs
  // ---------------------------------------

  using OutputStream_t = typename StreamPolicy::OutputStreamType;
  using InputStream_t  = typename StreamPolicy::InputStreamType;
  using Underlying_t   = typename StreamPolicy::UnderlyingType;

  // ---------------------------------------
  //                Serialise
  // ---------------------------------------

  template <typename... TypesToSerialise>
  static OutputStream_t& SerialiseToStream(OutputStream_t& ref_dest_stream,
                                           TypesToSerialise&&... ref_source_objs) {
    return SerialisationPolicy::ConvertToStream(
          ref_dest_stream, std::forward<TypesToSerialise>(ref_source_objs)...);
  }

  template <typename... TypesToSerialise>
  static Underlying_t Serialise(OutputStream_t& ref_dest_stream,
                                TypesToSerialise&&... ref_source_objs) {
    return StreamPolicy::GetUnderlyingType(SerialiseToStream(ref_dest_stream,
                            std::forward<TypesToSerialise>(ref_source_objs)...));
  }

  template <typename... TypesToSerialise>
  static Underlying_t Serialise(TypesToSerialise&&... ref_source_objs) {
    OutputStream_t output_stream;
    return Serialise(output_stream, std::forward<TypesToSerialise>(ref_source_objs)...);
  }

  // ---------------------------------------
  //                 Parse
  // ---------------------------------------

  template <typename... TypesToParse>
  static void Parse(InputStream_t& ref_source_stream, TypesToParse&... ref_dest_objs) {
    SerialisationPolicy::ConvertFromStream(ref_source_stream, ref_dest_objs...);
  }

  template <typename TypeToParse>
  static TypeToParse& Parse(InputStream_t& ref_source_stream, TypeToParse& ref_dest_obj) {
    SerialisationPolicy::ConvertFromStream(ref_source_stream, ref_dest_obj);
    return ref_dest_obj;
  }

  template <typename TypeToParse>
  static TypeToParse Parse(InputStream_t& ref_source_stream) {
    TypeToParse dest_obj;
    return Parse(ref_source_stream, dest_obj);
  }

  template <typename... TypesToParse>
  static void Parse(const Underlying_t& ref_source, TypesToParse&... ref_dest_objs) {
    InputStream_t input_stream{ref_source};
    Parse(input_stream, ref_dest_objs...);
  }

  template <typename TypeToParse>
  static TypeToParse& Parse(const Underlying_t& ref_source, TypeToParse& ref_dest_obj) {
    InputStream_t input_stream{ref_source};
    return Parse(input_stream, ref_dest_obj);
  }

  template <typename TypeToParse>
  static TypeToParse Parse(const Underlying_t& ref_source) {
    InputStream_t input_stream{ref_source};
    return Parse<TypeToParse>(input_stream);
  }

  Serialisation() = delete;
};

using CerealStringSerialisation = Serialisation<CerealPolicy, StringStreamPolicy>;
using CerealBoostInterprocessSerialisation = Serialisation<CerealPolicy, BoostInterprocessStreamPolicy>;

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SERIALISATION_SERIALISATION_H_
