// Copyright (c) 2016 Google Inc.
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

#include "strip_debug_info_pass.h"
#include "ir_context.h"

namespace spvtools {
namespace opt {

Pass::Status StripDebugInfoPass::Process() {
  bool modified = !context()->debugs1().empty() ||
                  !context()->debugs2().empty() ||
                  !context()->debugs3().empty();
  context()->debug_clear();

  context()->module()->ForEachInst([&modified](Instruction* inst) {
    modified |= !inst->dbg_line_insts().empty();
    inst->dbg_line_insts().clear();
  });

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
