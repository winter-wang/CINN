// Copyright (c) 2021 CINN Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cinn/common/macros.h"
#include "cinn/frontend/op_mapper_registry.h"
#include "cinn/frontend/op_mappers/common_utils.h"

namespace cinn {
namespace frontend {
namespace op_mappers {

void FetchOpMapper(const paddle::cpp::OpDesc& op_desc, const OpMapperContext& ctx) {
  CHECK_EQ(op_desc.Input("X").size(), 1UL);
  auto output_name = op_desc.Input("X").front();
  LOG(INFO) << "detect model output: [" << output_name << "]";
}

void FeedOpMapper(const paddle::cpp::OpDesc& op_desc, const OpMapperContext& ctx) {
  CHECK_EQ(op_desc.Output("Out").size(), 1UL);
  auto feed_name = op_desc.Output("Out").front();
  VLOG(4) << "Model get feed [" << feed_name << "]";

  const auto& feed_info = ctx.GetFeedInfo(feed_name);
  auto cinn_id          = cinn::utils::TransValidVarName(feed_name);
  auto input            = ctx.Builder()->CreateInput(feed_info.type, feed_info.shape, cinn_id);
  ctx.AddVar(feed_name, input);
  ctx.AddVarModelToProgram(feed_name, input.id().data());
}

}  // namespace op_mappers
}  // namespace frontend
}  // namespace cinn

CINN_REGISTER_HELPER(fetch_feed) {
  CINN_REGISTER_OP_MAPPER(fetch, cinn::frontend::op_mappers::FetchOpMapper)
  CINN_REGISTER_OP_MAPPER(feed, cinn::frontend::op_mappers::FeedOpMapper)
  return true;
}
