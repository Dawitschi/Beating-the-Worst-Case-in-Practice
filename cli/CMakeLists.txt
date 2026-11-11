
set(EXECUTABLE_FILES
    run.cpp
    convert_to_edgelist.cpp
    graph_generator.cpp
    run_dominance.cpp
    run_dominance_with_diff_var.cpp
    bbfs.cpp
)

foreach (file ${EXECUTABLE_FILES})
    get_filename_component(executable ${file} NAME_WE)
    add_executable(${executable} ${file})
    target_link_libraries(${executable} PRIVATE framework cli11)
endforeach ()

# Graph-only CSV generation for the R analysis.
add_executable(graph_properties ../evaluation/graph_properties.cpp)
target_link_libraries(graph_properties PRIVATE framework)

