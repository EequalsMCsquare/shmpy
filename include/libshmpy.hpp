#pragma once

#include <stdexcept>
#include <cstdlib>
#include <mutex>
#include <cstddef>
#include <string_view>
#include <cstdint>
#include <climits>
#include <thread>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "dtype.hpp"
#include "exception.hpp"
