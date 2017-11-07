# Set the location of flatc and the buffers
set(FLATBUFFERS_FLATC_EXECUTABLE ${CMAKE_BINARY_DIR}/lib/flatbuffers/flatc)
set(FLATBUFFERS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/schemes)
include_directories(${CMAKE_BINARY_DIR}/schemes)

# Include flatbuffer CMakeLists.txt for compilation of flatc and include the headers directory
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/flatbuffers)
include_directories(lib/flatbuffers/include)

function(create_flatbuffer_target TARGET_NAME FBS_DIR)
    # Variables
    set(INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${FBS_DIR})

    # Collect all .fbs files
    file(GLOB_RECURSE BUFFERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${INPUT_DIR}/*.fbs")

    # Iterate all flatbuffers
    foreach(FBS_PATH ${BUFFERS})
        # Variables
        set(INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FBS_PATH})
        string(REGEX REPLACE "\\.fbs$" "_generated.h" OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${FBS_PATH})
        get_filename_component(FILE_OUTPUT_DIR ${OUTPUT_FILE} DIRECTORY)

        # Compile the buffer
        add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}"
                -c --no-includes --gen-mutable --gen-object-api
                -o "${FILE_OUTPUT_DIR}"
                -I "${FLATBUFFERS_INCLUDE_DIR}"
                "${INPUT_FILE}"
                DEPENDS flatc ${INPUT_FILE}
        )

        # Add the output to the list of buffers
        set(FBS_HEADERS ${FBS_HEADERS} ${OUTPUT_FILE})
    endforeach()

    # Create the custom target for the headers
    add_custom_target(${TARGET_NAME} DEPENDS ${FBS_HEADERS})
endfunction()