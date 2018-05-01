/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_SIGNABLE_HPP
#define IROHA_SIGNABLE_HPP

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <unordered_set>

#include "cryptography/default_hash_provider.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {

  namespace crypto {
    class Signed;
    class PublicKey;
  }  // namespace crypto

  namespace interface {

#ifdef DISABLE_BACKWARD
#define SIGNABLE(Model) Signable<Model>
#else
#define SIGNABLE(Model) Signable<Model, iroha::model::Model>
#endif

/**
 * Interface provides signatures and adds them to model object
 * @tparam Model - your new style model
 */
#ifndef DISABLE_BACKWARD
    template <typename Model,
              typename OldModel,
              typename HashProvider = crypto::DefaultHashProvider>
    class Signable : public Primitive<Model, OldModel> {
#else
    template <typename Model,
              typename HashProvider = shared_model::crypto::Sha3_256>
    class Signable : public ModelPrimitive<Model> {
#endif

     public:
      /**
       * @return attached signatures
       */
      virtual types::SignatureRangeType signatures() const = 0;

      /**
       * Attach signature to object
       * @param signature - signature object for insertion
       * @return true, if signature was added
       */
      virtual bool addSignature(const crypto::Signed &signed_blob,
                                const crypto::PublicKey &public_key) = 0;

      /**
       * Clear object's signatures
       * @return true, if signatures were cleared
       */
      virtual bool clearSignatures() = 0;

      /**
       * @return time of creation
       */
      virtual types::TimestampType createdTime() const = 0;

      /**
       * @return object payload (everything except signatures)
       */
      virtual const types::BlobType &payload() const = 0;

      /**
       * @return blob representation of object include signatures
       */
      virtual const types::BlobType &blob() const = 0;

      /**
       * Provides comparison based on equality of objects and signatures.
       * @param rhs - another model object
       * @return true, if objects totally equal
       */
      bool operator==(const Model &rhs) const override {
        return this->hash() == rhs.hash()
            and boost::equal(this->signatures(), rhs.signatures())
            and this->createdTime() == rhs.createdTime();
      }

      const types::HashType &hash() const {
        if (hash_ == boost::none) {
          hash_.emplace(HashProvider::makeHash(payload()));
        }
        return *hash_;
      }

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Signable")
            .append("created_time", std::to_string(createdTime()))
            .appendAll(signatures(),
                       [](auto &signature) { return signature.toString(); })
            .finalize();
      }

     protected:
      /**
       * Hash class for SigWrapper type. It's required since std::unordered_set
       * uses hash inside and it should be declared explicitly for user-defined
       * types.
       */
      class SignableHash {
       public:
        /**
         * Operator which actually calculates hash. Uses boost::hash_combine to
         * calculate hash from several fields.
         * @param sig - item to find hash from
         * @return calculated hash
         */
        size_t operator()(const types::SignatureType &sig) const {
          std::size_t seed = 0;
          boost::hash_combine(seed, sig->publicKey().blob());
          boost::hash_combine(seed, sig->signedData().blob());
          return seed;
        }
      };

      using SignatureSetType =
          std::unordered_set<types::SignatureType, SignableHash>;

     private:
      mutable boost::optional<types::HashType> hash_;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SIGNABLE_HPP
