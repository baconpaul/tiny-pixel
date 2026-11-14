# tpx_embed(target symbol file): compiles file into target as
#   extern const unsigned char symbol[]; extern const unsigned int symbol_size;
function(tpx_embed target symbol file)
    get_filename_component(file ${file} ABSOLUTE)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${file})
    file(READ ${file} hex HEX)
    string(LENGTH "${hex}" hexLen)
    math(EXPR size "${hexLen} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    set(out ${CMAKE_CURRENT_BINARY_DIR}/embed/${symbol}.cpp)
    file(WRITE ${out}.tmp
        "extern const unsigned char ${symbol}[] = {${bytes}};\n"
        "extern const unsigned int ${symbol}_size = ${size};\n")
    # leave the file alone when unchanged so it does not recompile every configure
    file(COPY_FILE ${out}.tmp ${out} ONLY_IF_DIFFERENT)
    target_sources(${target} PRIVATE ${out})
endfunction()
