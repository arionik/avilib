cmake_minimum_required (VERSION 3.22.0)
project (avilib)

add_library (avi STATIC avilib.cpp reader.cpp writer.cpp)
target_compile_definitions(avi PRIVATE _CRT_SECURE_NO_WARNINGS)
if (NOT WIN32)
	add_compile_options(-Wno-multichar)
endif()
if(CMAKE_GENERATOR MATCHES "Xcode")
    set_target_properties(avi PROPERTIES
        "XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug]"   "YES"
        "XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release]" "NO"
        "XCODE_ATTRIBUTE_ARCHS[variant=Release]"            "arm64 x86_64"
        XCODE_ATTRIBUTE_SKIP_INSTALL                        "YES"
    )
endif()
install(TARGETS avi DESTINATION lib)
install(FILES avilib.h DESTINATION include)

add_executable (avilib_test main.cpp)
target_link_libraries(avilib_test avi)
