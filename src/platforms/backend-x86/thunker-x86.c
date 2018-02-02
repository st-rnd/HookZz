/**
 *    Copyright 2017 jmpews
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "thunker-x86.h"
#include "zzinfo.h"
#include <string.h>

// just like pre_call, wow!
void function_context_begin_invocation(ZzHookFunctionEntry *entry, zz_ptr_t next_hop, RegState *rs,
                                       zz_ptr_t caller_ret_addr) {
    ZZ_DEBUG_LOG("target %p call begin-invocation", entry->target_ptr);

    ZzThreadStack *stack = ZzGetCurrentThreadStack(entry->thread_local_key);
    if (!stack) {
        stack = ZzNewThreadStack(entry->thread_local_key);
    }
    ZzCallStack *callstack = ZzNewCallStack();
    ZzPushCallStack(stack, callstack);

    /* call pre_call */
    if (entry->pre_call) {
        PRECALL pre_call;
        pre_call = entry->pre_call;
        (*pre_call)(rs, (ThreadStack *)stack, (CallStack *)callstack);
    }

    /* set next hop */
    if (entry->replace_call) {
        *(zz_ptr_t *)next_hop = entry->replace_call;
    } else {
        *(zz_ptr_t *)next_hop = entry->on_invoke_trampoline;
    }

    if (entry->hook_type == HOOK_TYPE_FUNCTION_PRE_POST) {
        callstack->caller_ret_addr   = *(zz_ptr_t *)caller_ret_addr;
        *(zz_ptr_t *)caller_ret_addr = entry->on_leave_trampoline;
    }
}

// just like post_call, wow!
void function_context_half_invocation(ZzHookFunctionEntry *entry, zz_ptr_t next_hop, RegState *rs,
                                      zz_ptr_t caller_ret_addr) {
    ZZ_DEBUG_LOG("target %p call half-invocation", entry->target_ptr);

    ZzThreadStack *stack = ZzGetCurrentThreadStack(entry->thread_local_key);
    if (!stack) {
#if defined(DEBUG_MODE)
        debug_break();
#endif
    }
    ZzCallStack *callstack = ZzPopCallStack(stack);

    /* call half_call */
    if (entry->half_call) {
        HALFCALL half_call;
        half_call = entry->half_call;
        (*half_call)(rs, (ThreadStack *)stack, (CallStack *)callstack);
    }

    /*  set next hop */
    *(zz_ptr_t *)next_hop = (zz_ptr_t)entry->target_half_ret_addr;

    ZzFreeCallStack(callstack);
}

// just like post_call, wow!
void function_context_end_invocation(ZzHookFunctionEntry *entry, zz_ptr_t next_hop, RegState *rs) {
    ZZ_DEBUG_LOG("%p call end-invocation", entry->target_ptr);

    ZzThreadStack *stack = ZzGetCurrentThreadStack(entry->thread_local_key);
    if (!stack) {
#if defined(DEBUG_MODE)
        debug_break();
#endif
    }
    ZzCallStack *callstack = ZzPopCallStack(stack);

    /* call post_call */
    if (entry->post_call) {
        POSTCALL post_call;
        post_call = entry->post_call;
        (*post_call)(rs, (ThreadStack *)stack, (CallStack *)callstack);
    }

    /* set next hop */
    *(zz_ptr_t *)next_hop = callstack->caller_ret_addr;
    ZzFreeCallStack(callstack);
}

void zz_x86_thunker_build_enter_thunk(ZzAssemblerWriter *writer) {}

void zz_x86_thunker_build_half_thunk(ZzAssemblerWriter *writer) {}

void zz_x86_thunker_build_leave_thunk(ZzAssemblerWriter *writer) {}

ZZSTATUS ZzThunkerBuildThunk(ZzInterceptorBackend *self) { return ZZ_FAILED; }
