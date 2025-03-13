cmake_minimum_required(VERSION 3.21)

project(UntitledI18N VERSION 1.5.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)

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
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
    include(FindYamlCpp)
    find_package(YamlCpp REQUIRED)
    include_directories(${YamlCpp_INCLUDE_PATH})
endif()

set(UI18N_HEADERS "UI18N.hpp" "Common.h")
file(GLOB_RECURSE HASH_MAP "parallel-hashmap/*.h")

add_library(UntitledI18N ${UI18N_LIBRARY_TYPE} ${UI18N_HEADERS} "C/cui18n.h" "C/cui18n.cpp" "UI18N.cpp" ${HASH_MAP})

target_compile_definitions(UntitledI18N PRIVATE "MLS_EXPORT_LIBRARY" "MLS_LIB_COMPILE")
target_compile_definitions(UntitledI18N PUBLIC UIMGUI_I18N_MODULE_ENABLED)
list(APPEND compile_defs "UIMGUI_I18N_MODULE_ENABLED")

target_link_libraries(UntitledI18N yaml-cpp)

configure_file(UntitledI18N.pc.in UntitledI18N.pc @ONLY)

if (UIMGUI_INSTALL)
    install(TARGETS UntitledI18N
            LIBRARY DESTINATION lib64/
            ARCHIVE DESTINATION lib64/)
    install(FILES ${UI18N_HEADERS} DESTINATION include/UntitledI18N)
    install(FILES ${HASH_MAP} DESTINATION include/UntitledI18N/parallel-hashmap/parallel_hashmap)
    install(FILES "C/cui18n.h" DESTINATION include/UntitledI18N/C)
    install(FILES ${PROJECT_BINARY_DIR}/UntitledI18N.pc
            DESTINATION lib/pkgconfig/)
endif()
