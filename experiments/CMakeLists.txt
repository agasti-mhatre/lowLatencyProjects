add_executable(atomics atomics_barriers.cpp)

foreach(t atomics
)
    target_compile_options(${t} PRIVATE
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )
endforeach()