#ifndef UNET_OPERATINGSYSTEM_OPERATINGSYSTEM_H
#define UNET_OPERATINGSYSTEM_OPERATINGSYSTEM_H

#include "../weos/chrono.hpp"
#include "../weos/mutex.hpp"
#include "../weos/objectpool.hpp"
#include "../weos/semaphore.hpp"
#include "../weos/thread.hpp"

namespace OperatingSystem
{
// chrono.hpp
namespace chrono = weos::chrono;

// mutex.hpp
using weos::lock_guard;
using weos::mutex;

// objectpool.hpp
using weos::object_pool;
using weos::counting_object_pool;

// semaphore.hpp
using weos::semaphore;

// thread.hpp
using weos::thread;

} // namespace OperatingSystem

#endif // UNET_OPERATINGSYSTEM_OPERATINGSYSTEM_H
