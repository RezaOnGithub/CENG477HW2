cmake_minimum_required(VERSION 3.28)
project(rasterizer CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(thirdparty/raylib)
target_compile_options(raylib PRIVATE "-w") # disable warnings
include_directories(thirdparty/include) # raygui, stb_image_write
add_subdirectory(thirdparty/googletest)
# add_subdirectory(thirdparty/assimp)     # a model loading framework
#add_subdirectory(thirdparty/glm) # to check my math!
find_package(glm CONFIG REQUIRED)

add_compile_definitions(CENG477_RESOURCE="${CMAKE_CURRENT_SOURCE_DIR}/resource")

add_executable(imgdiff
        main_imgdiff.cpp
)
target_link_libraries(imgdiff raylib)

enable_testing()
include(GoogleTest)
add_executable(mixed_test 
        test_everything.cpp
        Base.cpp
        tinyxml2.cpp
        CENG477.cpp
        World.cpp
)
target_link_libraries(mixed_test PRIVATE
        GTest::gtest_main
        glm::glm
)
gtest_discover_tests(mixed_test)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(rasterizer
        main_rasterizer.cpp
        tinyxml2.cpp
        CENG477.cpp
        Base.cpp
        World.cpp
        Renderer.cpp
)
add_executable(quickrizer
        main_quickrizer.cpp
        tinyxml2.cpp
        CENG477.cpp
        Base.cpp
        World.cpp
        Renderer.cpp
)

# add_executable(visual_test
#         main_visualtest.cpp
#         Base.cpp
# )
# target_link_libraries(visual_test raylib)