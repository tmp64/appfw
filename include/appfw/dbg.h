#ifndef APPFW_DBG_H
#define APPFW_DBG_H
#include <cassert>
#include <appfw/compiler.h>

namespace appfw {

// TODO: Replace with custom
#define AFW_ASSERT(cond) assert(cond)
#define AFW_ASSERT_MSG(cond, msg) assert(cond)

} // namespace appfw

#endif
