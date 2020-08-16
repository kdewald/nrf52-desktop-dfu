
message("-- [INFO] Building DFU Library")
file(GLOB_RECURSE SRC_DFU_FILES "src-dfu/*.cpp" "src-dfu/*.c")
add_library(dfu SHARED ${SRC_DFU_FILES})
add_library(dfu-static STATIC ${SRC_DFU_FILES})
file(COPY "src-dfu/NrfDfuServer.h" "src-dfu/NrfDfuServerTypes.h" DESTINATION ${OUTPUT_DIR})

message("-- [INFO] Building DFU Library Test Application")
# BLE Platform Dependant Library Configuration
include_directories(${PROJECT_DIR_PATH}/src-dfu-app/ble)
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LIB_BLE ${PROJECT_DIR_PATH}/src-dfu-app/ble/windows-${WINDOWS_TARGET_ARCH}/nativeble-static.lib )
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LIB_BLE ${PROJECT_DIR_PATH}/src-dfu-app/ble/darwin/libnativeble-static.a "-framework Foundation" "-framework CoreBluetooth" objc )
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_search_module(DBUS REQUIRED dbus-1)
    if(DBUS_FOUND)
        include_directories(${DBUS_INCLUDE_DIRS})
        message(STATUS "Using DBUS from path: ${DBUS_INCLUDE_DIRS}")
    else()
        message(ERROR "Unable to find DBUS")
    endif()
    set(LIB_BLE ${PROJECT_DIR_PATH}/src-dfu-app/ble/linux/libnativeble-static.a ${DBUS_LIBRARIES} pthread)
ENDIF()

#Building DFU Applications
include_directories(${PROJECT_DIR_PATH}/src-dfu)
file(GLOB_RECURSE SRC_DFU_TEST_FILES "src-dfu-app/*.cpp" "src-dfu-app/*.cc")
file(GLOB_RECURSE SRC_MINIZ_FILES "src-dfu-app/miniz/*.c" )

add_executable(dfu_app ${SRC_DFU_TEST_FILES} ${SRC_MINIZ_FILES})
target_link_libraries(dfu_app ${LIB_BLE} dfu-static)
target_compile_definitions(dfu_app PUBLIC MINIZ_STATIC_DEFINE)
