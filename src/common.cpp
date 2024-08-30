#include "src/common.h"
#include <stdexcept>

namespace Alf
{

void fatal_error(const char *const msg)
{
    throw std::runtime_error{std::string{"Fatal error occurred: "} + msg};
}

} // namespace Alf
