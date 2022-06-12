#include <crasy/utils.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace crasy {

#ifdef _WIN32

result<std::size_t> available_cpu_cores() {
    SYSTEM_INFO sysinfo{};
    GetSystemInfo(&sysinfo);
    return ok(static_cast<std::size_t>(sysinfo.dwNumberOfProcessors));
}

#else

result<std::size_t> available_cpu_cores() {
    auto ret = ::sysconf(_SC_NPROCESSORS_ONLN);
    if (ret < 0) { return err(std::error_code(errno, std::system_category())); }
    return ok(static_cast<std::size_t>(ret));
}

#endif

} // namespace crasy
