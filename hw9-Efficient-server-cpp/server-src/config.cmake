# Option for the resource pool size
add_definitions(-DRESOURCE_POOL_TWO_SIZE=3)
# Option for enabling logging
option(ENABLE_LOGGER_FILE "Enable logger file" OFF)
option(ENABLE_LOGGER_THREAD "Enable logger thread" ON)

option(ENABLE_DEBUG_LOG "Enable debug logging level" ON)
option(ENABLE_INFO_LOG "Enable info logging level" ON)
option(ENABLE_WARN_LOG "Enable warn logging level" ON)
option(ENABLE_ERROR_LOG "Enable error logging level" ON)

# Option for enabling locking
option(ENABLE_LOCKING "Enable grid locking" OFF)

# Conditionally add definitions based on the configuration option
if (ENABLE_LOGGER_THREAD)
    add_definitions(-DENABLE_LOGGER_THREAD)
endif ()
if (ENABLE_LOGGER_FILE)
    add_definitions(-DENABLE_LOGGER_FILE)
endif ()
if (ENABLE_DEBUG_LOG)
    add_definitions(-DENABLE_DEBUG)
endif ()
if (ENABLE_INFO_LOG)
    add_definitions(-DENABLE_INFO)
endif ()
if (ENABLE_WARN_LOG)
    add_definitions(-DENABLE_WARN)
endif ()
if (ENABLE_ERROR_LOG)
    add_definitions(-DENABLE_ERROR)
endif ()
if (ENABLE_LOCKING)
    add_definitions(-DENABLE_LOCKING)
endif ()
