/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      bool SupermajorityCheckerImpl::hasSupermajority(
          const shared_model::interface::types::SignatureRangeType &signatures,
          const std::vector<std::shared_ptr<shared_model::interface::Peer>>
              &peers) const {
        return checkSize(boost::size(signatures), peers.size())
            and peersSubset(signatures, peers);
      }

      bool SupermajorityCheckerImpl::checkSize(uint64_t current,
                                               uint64_t all) const {
        if (current > all) {
          return false;
        }
        auto f = (all - 1) / 3.0;
        return current >= 2 * f + 1;
      }

      bool SupermajorityCheckerImpl::peersSubset(
          const shared_model::interface::types::SignatureRangeType &signatures,
          const std::vector<std::shared_ptr<shared_model::interface::Peer>>
              &peers) const {
        return std::all_of(
            signatures.begin(),
            signatures.end(),
            [&peers](const auto &signature) {
              return std::find_if(
                         peers.begin(),
                         peers.end(),
                         [&signature](const std::shared_ptr<
                                      shared_model::interface::Peer> &peer) {
                           return signature.publicKey() == peer->pubkey();
                         })
                  != peers.end();
            });
      }

      bool SupermajorityCheckerImpl::hasReject(uint64_t frequent,
                                               uint64_t voted,
                                               uint64_t all) const {
        auto not_voted = all - voted;
        return not checkSize(frequent + not_voted, all);
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
