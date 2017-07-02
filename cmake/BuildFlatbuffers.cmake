function(compile_flatbuffers_schema SRC_FBS)
    get_filename_component(SRC_FBS_DIR ${SRC_FBS} PATH)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${SRC_FBS})
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${GEN_HEADER}
            COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c --no-includes --gen-mutable
            --gen-object-api -o "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS_DIR}"
            -I "${FLATBUFFERS_INCLUDE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}"
            DEPENDS flatc ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FBS}
    )
endfunction()

set(FLATBUFFERS_FLATC_EXECUTABLE lib/flatbuffers/flatc)
set(FLATBUFFERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deviceLib/buffers)
add_subdirectory(lib/flatbuffers)

compile_flatbuffers_schema(deviceLib/buffers/monster.fbs)
add_custom_target(flatbuffer_headers DEPENDS deviceLib/buffers/monster_generated.h)