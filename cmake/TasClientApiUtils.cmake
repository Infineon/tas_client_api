
function(tac_add_target_sources target srcs)
    if(TARGET ${target})
        target_sources(${target} 
            PRIVATE ${srcs}
        )

        target_include_directories(${target} 
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    else()
        message(STATUS "Target ${target} not defined, sources not added")
    endif()
endfunction()

