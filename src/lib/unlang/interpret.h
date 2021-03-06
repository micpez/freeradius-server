#pragma once
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * $Id$
 *
 * @file unlang/interpret.h
 * @brief Declarations for the unlang interpreter.
 *
 * @copyright 2019 The FreeRADIUS server project
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <freeradius-devel/unlang/action.h>

#define UNLANG_TOP_FRAME (true)
#define UNLANG_SUB_FRAME (false)

/** Function to call when interpreting a frame
 *
 * @param[in] request		The current request.
 * @param[in,out] presult	Pointer to the current rcode, may be modified by the function.
 * @return an action for the interpreter to perform.
 */
typedef unlang_action_t (*unlang_op_interpret_t)(REQUEST *request, rlm_rcode_t *presult);

/** Function to call if the initial function yielded and the request was signalled
 *
 * This is the operation specific cancellation function.  This function will usually
 * either call a more specialised cancellation function set when something like a module yielded,
 * or just cleanup the state of the original #unlang_op_interpret_t.
 *
 * @param[in] request		The current request.
 * @param[in] action		We're being signalled with.
 */
typedef void (*unlang_op_signal_t)(REQUEST *request, fr_state_signal_t action);

/** A generic function pushed by a module or xlat to functions deeper in the C call stack to create resumption points
 *
 * @param[in] request		The current request.
 * @param[in,out] uctx		Provided by whatever pushed the function.  Is opaque to the
 *				interpreter, but should be usable by the function.
 *				All input (args) and output will be done using this structure.
 * @return an #unlang_action_t.
 */
typedef unlang_action_t (*unlang_function_t)(REQUEST *request, rlm_rcode_t *presult, int *priority, void *uctx);

/** An unlang operation
 *
 * These are like the opcodes in other interpreters.  Each operation, when executed
 * will return an #unlang_action_t, which determines what the interpreter does next.
 */
typedef struct {
	char const		*name;				//!< Name of the operation.

	unlang_op_interpret_t	interpret;     			//!< Function to interpret the keyword

	unlang_op_signal_t	signal;				//!< Function to signal stop / dup / whatever

	bool			debug_braces;			//!< Whether the operation needs to print braces
								///< in debug mode.

	size_t			frame_state_size;       	//!< size of instance data in the stack frame

	char const		*frame_state_name;		//!< talloc name of the frame instance data

	size_t			frame_state_pool_objects;	//!< How many sub-allocations we expect.

	size_t			frame_state_pool_size;		//!< The total size of the pool to alloc.
} unlang_op_t;

void		unlang_interpret_push_function(REQUEST *request,
					       unlang_function_t func, unlang_function_t repeat, void *uctx);

void		unlang_interpret_push_section(REQUEST *request, CONF_SECTION *cs,
					      rlm_rcode_t default_action, bool top_frame);

void		unlang_interpret_push_instruction(REQUEST *request, void *instruction,
						  rlm_rcode_t default_rcode, bool top_frame);

rlm_rcode_t	unlang_interpret(REQUEST *request);

rlm_rcode_t	unlang_interpret_section(REQUEST *request, CONF_SECTION *cs, rlm_rcode_t default_action);

rlm_rcode_t	unlang_interpret_synchronous(REQUEST *request, CONF_SECTION *cs, rlm_rcode_t action);

void		*unlang_interpret_stack_alloc(TALLOC_CTX *ctx);

void		unlang_interpret_resumable(REQUEST *request);

void		unlang_interpret_signal(REQUEST *request, fr_state_signal_t action);

int		unlang_interpret_stack_depth(REQUEST *request);

rlm_rcode_t	unlang_interpret_stack_result(REQUEST *request);

void		unlang_interpret_init(void);
#ifdef __cplusplus
}
#endif
