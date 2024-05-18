# Option for the resource pool size
add_definitions(-DRESOURCE_POOL_SIZE=2)
# Option for enabling logging
option(ENABLE_DEBUG_LOG "Enable debug logging level" OFF)
option(ENABLE_INFO_LOG "Enable info logging level" ON)
option(ENABLE_WARN_LOG "Enable warn logging level" ON)
option(ENABLE_ERROR_LOG "Enable error logging level" ON)
# Option for enabling locking
option(ENABLE_LOCKING "Enable grid locking" ON)

# Conditionally add definitions based on the configuration option
if(ENABLE_DEBUG_LOG)
    add_definitions(-DENABLE_DEBUG)
endif()
if(ENABLE_INFO_LOG)
    add_definitions(-DENABLE_INFO)
endif()
if(ENABLE_WARN_LOG)
    add_definitions(-DENABLE_WARN)
endif()
if(ENABLE_ERROR_LOG)
    add_definitions(-DENABLE_ERROR)
endif()
if(ENABLE_LOCKING)
    add_definitions(-DENABLE_LOCKING)
endif()
