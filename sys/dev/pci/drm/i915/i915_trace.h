/* Public domain. */
#ifndef _I915_TRACE_H_
#define _I915_TRACE_H_

#include "i915_drv.h"

#define trace_g4x_wm(a, b)
#define trace_i915_context_create(a)
#define trace_i915_context_free(a)
#define trace_i915_gem_evict(a, b, c, d)
#define trace_i915_gem_evict_node(a, b, c)
#define trace_i915_gem_evict_vm(a)
#define trace_i915_gem_object_clflush(a)
#define trace_i915_gem_object_create(a)
#define trace_i915_gem_object_destroy(a)
#define trace_i915_gem_object_fault(a, b, c, d)
#define trace_i915_gem_object_pread(a, b, c)
#define trace_i915_gem_object_pwrite(a, b, c)
#define trace_i915_gem_shrink(a, b, c)
#define trace_i915_ppgtt_create(a)
#define trace_i915_ppgtt_release(a)
#define trace_i915_reg_rw(a, b, c, d, e)
#define trace_i915_request_add(a)
#define trace_i915_request_execute(a)
#define trace_i915_request_guc_submit(a)
#define trace_i915_request_in(a, b)
#define trace_i915_request_out(a)
#define trace_i915_request_queue(a, b)
#define trace_i915_request_retire(a)
#define trace_i915_request_submit(a)
#define trace_i915_request_wait_begin(a, b)
#define trace_i915_request_wait_end(a)
#define trace_i915_vma_bind(a, b)
#define trace_i915_vma_unbind(a)
#define trace_intel_context_ban(a)
#define trace_intel_context_create(a)
#define trace_intel_context_deregister(a)
#define trace_intel_context_deregister_done(a)
#define trace_intel_context_do_pin(a)
#define trace_intel_context_do_unpin(a)
#define trace_intel_context_fence_release(a)
#define trace_intel_context_free(a)
#define trace_intel_context_register(a)
#define trace_intel_context_reset(a)
#define trace_intel_context_sched_disable(a)
#define trace_intel_context_sched_done(a)
#define trace_intel_context_sched_enable(a)
#define trace_intel_context_set_prio(a)
#define trace_intel_context_steal_guc_id(a)
#define trace_intel_cpu_fifo_underrun(a, b)
#define trace_intel_disable_plane(a, b)
#define trace_intel_fbc_activate(a)
#define trace_intel_fbc_deactivate(a)
#define trace_intel_fbc_nuke(a)
#define trace_intel_frontbuffer_flush(a, b)
#define trace_intel_frontbuffer_invalidate(a, b)
#define trace_intel_gpu_freq_change(a)
#define trace_intel_memory_cxsr(a, b, c)
#define trace_intel_pch_fifo_underrun(a, b)
#define trace_intel_pipe_crc(a, b)
#define trace_intel_pipe_disable(a)
#define trace_intel_pipe_enable(a)
#define trace_intel_pipe_update_end(a, b, c)
#define trace_intel_pipe_update_start(a)
#define trace_intel_pipe_update_vblank_evaded(a)
#define trace_intel_update_plane(a, b)
#define trace_vlv_fifo_size(a, b, c, d)
#define trace_vlv_wm(a, b)

#endif
