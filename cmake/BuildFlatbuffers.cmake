# Function to generate the filename for the resulting header
function(get_compiled_fbs_filename SRC_FBS VAR)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" COMPILED_NAME ${SRC_FBS})
    set(${VAR} ${COMPILED_NAME} PARENT_SCOPE)
endfunction()
# Function to compile a flatbuffer scheme into a c++ header in-place
function(compile_flatbuffers_schema SRC_FBS)
    get_filename_component(SRC_FBS_DIR ${SRC_FBS} PATH)
    get_compiled_fbs_filename(${SRC_FBS} GEN_HEADER)
    add_custom_command(
            OUTPUT ${GEN_HEADER}
            COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c --no-includes --gen-mutable
            --gen-object-api -o "${SRC_FBS_DIR}"
            -I "${FLATBUFFERS_INCLUDE_DIR}"
            "${SRC_FBS}"
            DEPENDS flatc ${SRC_FBS}
    )
endfunction()

# Set the location of flatc and the buffers
set(FLATBUFFERS_FLATC_EXECUTABLE lib/flatbuffers/flatc)
set(FLATBUFFERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deviceLib/buffers)

# Include flatbuffer CMakeLists.txt for compilation of flatc and include the headers directory
add_subdirectory(lib/flatbuffers)
include_directories(lib/flatbuffers/include)

# Search for .fbs files recursively in FLATBUFFERS_DIR
file(GLOB_RECURSE BUFFERS "${FLATBUFFERS_DIR}/*.fbs")
# Iterate source buffers
foreach(SRC_FBS ${BUFFERS})
    # Compile buffers
    compile_flatbuffers_schema(${SRC_FBS})
    # Add resulting headers to the list
    get_compiled_fbs_filename(${SRC_FBS} HEADER_NAME)
    set(FBS_HEADERS ${FBS_HEADERS} ${HEADER_NAME})
endforeach()

# Create a custom target for the headers
add_custom_target(flatbuffer_headers DEPENDS ${FBS_HEADERS})