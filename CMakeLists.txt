cmake_minimum_required(VERSION 3.21)

project(UntitledI18N VERSION 1.0.0.0)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(.)

set(UI18N_LIBRARY_TYPE "SHARED")
if (BUILD_VARIANT_STATIC)
    set(UI18N_LIBRARY_TYPE "STATIC")
endif()

if (BUILD_VARIANT_VENDOR OR WIN32)
    if (USE_PRECONFIGURED_YAML)
        include_directories(${YAML_CPP_INCLUDE_DIRS_T})
    else ()
        add_subdirectory(yaml-cpp)
        include_directories("yaml-cpp/include/yaml-cpp" "yaml-cpp/include")
    endif()
else()
    include(FindYamlCpp)
    find_package(YamlCpp REQUIRED)
    include_directories(${YamlCpp_INCLUDE_PATH})
endif()

set(UI18N_HEADERS "UI18N.hpp" "Common.h")

add_library(UntitledI18N ${UI18N_LIBRARY_TYPE} ${UI18N_HEADERS} "C/cui18n.h" "C/cui18n.cpp" "UI18N.cpp")
add_executable(test main.cpp)

target_compile_definitions(UntitledI18N PRIVATE "UVK_LOG_EXPORT_FROM_LIBRARY" "UVK_LIB_COMPILE")
target_compile_definitions(UntitledI18N PUBLIC UIMGUI_I18N_MODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_I18N_MODULE_ENABLED")

target_link_libraries(UntitledI18N yaml-cpp)
target_link_libraries(test UntitledI18N)

configure_file(UntitledI18N.pc.in UntitledI18N.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledI18N
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/)
    install(FILES ${UI18N_HEADERS} DESTINATION include/UntitledI18N)
    install(FILES "C/cui18n.h" DESTINATION include/UntitledI18N/C)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledI18N.pc
            DESTINATION lib/pkgconfig/)
endif()