/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_fs.h"
#include "brw_fs_live_variables.h"
#include "brw_cfg.h"

/** @file brw_fs_saturate_propagation.cpp
 *
 * Implements a pass that propagates the SAT modifier from a MOV.SAT into the
 * instruction that produced the source of the MOV.SAT, thereby allowing the
 * MOV's src and dst to be coalesced and the MOV removed.
 *
 * For instance,
 *
 *    ADD     tmp, src0, src1
 *    MOV.SAT dst, tmp
 *
 * would be transformed into
 *
 *    ADD.SAT tmp, src0, src1
 *    MOV     dst, tmp
 */

static bool
opt_saturate_propagation_local(fs_visitor *v, bblock_t *block)
{
   bool progress = false;
   int ip = block->end_ip + 1;

   foreach_inst_in_block_reverse(fs_inst, inst, block) {
      ip--;

      if (inst->opcode != BRW_OPCODE_MOV ||
          inst->dst.file != GRF ||
          inst->src[0].file != GRF ||
          inst->src[0].abs ||
          inst->src[0].negate ||
          !inst->saturate)
         continue;

      int src_var = v->live_intervals->var_from_reg(inst->src[0]);
      int src_end_ip = v->live_intervals->end[src_var];

      bool interfered = false;
      foreach_inst_in_block_reverse_starting_from(fs_inst, scan_inst, inst, block) {
         if (scan_inst->overwrites_reg(inst->src[0])) {
            if (scan_inst->is_partial_write())
               break;

            if (scan_inst->saturate) {
               inst->saturate = false;
               progress = true;
            } else if (src_end_ip <= ip || inst->dst.equals(inst->src[0])) {
               if (scan_inst->can_do_saturate()) {
                  scan_inst->saturate = true;
                  inst->saturate = false;
                  progress = true;
               }
            }
            break;
         }
         for (int i = 0; i < scan_inst->sources; i++) {
            if (scan_inst->src[i].file == GRF &&
                scan_inst->src[i].reg == inst->src[0].reg &&
                scan_inst->src[i].reg_offset == inst->src[0].reg_offset) {
               if (scan_inst->opcode != BRW_OPCODE_MOV ||
                   !scan_inst->saturate ||
                   scan_inst->src[0].abs ||
                   scan_inst->src[0].negate) {
                  interfered = true;
                  break;
               }
            }
         }

         if (interfered)
            break;
      }
   }

   return progress;
}

bool
fs_visitor::opt_saturate_propagation()
{
   bool progress = false;

   calculate_live_intervals();

   foreach_block (block, cfg) {
      progress = opt_saturate_propagation_local(this, block) || progress;
   }

   /* Live intervals are still valid. */

   return progress;
}
