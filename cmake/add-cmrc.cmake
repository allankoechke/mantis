include(FetchContent)
include(ExternalProject)

set(MANTIS_ADMIN_DOWNLOAD_URL "https://github.com/allankoechke/mantis-admin/releases/download/${MANTIS_ADMIN_VERSION}/mantis-admin-static.zip")
set(MANTIS_ADMIN_BASE_DIR "${CMAKE_BINARY_DIR}/3rdParty/mantis-admin/${MANTIS_ADMIN_VERSION}")
set(MANTIS_ADMIN_ZIP_PATH "${MANTIS_ADMIN_BASE_DIR}/admin-build.zip")   # Admin ZIP file
set(MANTIS_ADMIN_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qrc")            # Admin UNZIP dir


# Step 1: Download the zip
file(DOWNLOAD
        ${MANTIS_ADMIN_DOWNLOAD_URL}
        ${MANTIS_ADMIN_ZIP_PATH}
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
        EXPECTED_HASH SHA256=123aaca425c139155fcc8b9f7468a6fff6cb2b14e6c2d889fd7a9878cd29a3ac
)

list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
if(NOT DOWNLOAD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to download React admin build zip")
endif()

if(NOT EXISTS ${MANTIS_ADMIN_EXTRACT_DIR}/index.html)
    file(MAKE_DIRECTORY ${MANTIS_ADMIN_EXTRACT_DIR})

    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xvf ${MANTIS_ADMIN_ZIP_PATH}
            WORKING_DIRECTORY ${MANTIS_ADMIN_EXTRACT_DIR}
            RESULT_VARIABLE UNZIP_RESULT
    )

    if(NOT UNZIP_RESULT EQUAL 0)
        # Try fallback unzip
        message(WARNING "CMake failed to extract ZIP; trying system unzip...")
        execute_process(
                COMMAND unzip -o ${MANTIS_ADMIN_ZIP_PATH} -d ${MANTIS_ADMIN_EXTRACT_DIR}
                RESULT_VARIABLE UNZIP_RESULT
        )

        if(NOT UNZIP_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to unzip React admin build using both CMake and unzip.")
        endif()
    endif()
endif()

# Step 3: Use cmrc to embed the files
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cmrc)

file(GLOB_RECURSE REACT_FILES LIST_DIRECTORIES false "${MANTIS_ADMIN_EXTRACT_DIR}/*")
message(STATUS "React files to embed: ${REACT_FILES}")

cmrc_add_resource_library(mantis-rc NAMESPACE mantis ${REACT_FILES})
target_link_libraries(mantis PRIVATE mantis-rc)
target_compile_definitions(mantis PRIVATE CMRC_ENABLE)
