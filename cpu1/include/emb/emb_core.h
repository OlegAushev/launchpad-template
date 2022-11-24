///
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "emb_core/emb_c28x.h"
#include "emb_core/emb_monostate.h"
#include "emb_core/emb_scopedenum.h"


/**
 * @brief
 */
#define EMB_UNUSED(arg) (void)arg;


/**
 * @brief UNIQ_ID implementation
 */
#define EMB_UNIQ_ID_IMPL(line) _a_local_var_##line
#define EMB_UNIQ_ID(line) EMB_UNIQ_ID_IMPL(line)


/**
 * @brief simple static_assert implementation
 */
#define EMB_CAT_(a, b) a ## b
#define EMB_CAT(a, b) EMB_CAT_(a, b)
#define EMB_STATIC_ASSERT(cond) typedef int EMB_CAT(assert, __LINE__)[(cond) ? 1 : -1]


namespace emb {


/**
 * @brief
 */
enum MasterSlaveMode
{
	ModeMaster,
	ModeSlave
};


/**
 * @brief
 */
enum ProcessStatus
{
	ProcessSuccess = 0,
	ProcessFail = 1,
	ProcessInProgress = 2,
};


} // namespace emb


