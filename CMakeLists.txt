cmake_minimum_required (VERSION 3.28)
project(raytracer C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(thirdparty/raylib)
# include_directories(thirdparty/include) # raygui
add_subdirectory(thirdparty/googletest)
# add_subdirectory(thirdparty/assimp)     # a model loading framework

add_compile_definitions(CENG477_RESOURCE="${CMAKE_CURRENT_SOURCE_DIR}/resource")

enable_testing()
include(GoogleTest)
add_executable(mixed_test test_everything.cpp)
target_link_libraries(mixed_test GTest::gtest_main)
gtest_discover_tests(mixed_test)

# TODO this time it's a bit much -  reorganize project
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(rasterizer
	main_rasterizer.cpp 
	Camera.cpp
	Matrix4.cpp
	Scene.cpp
	Triangle.cpp
	Color.cpp
	Mesh.cpp
	Vec3.cpp
	Helpers.cpp
	Rotation.cpp
	tinyxml2.cpp
	Vec4.cpp
	Scaling.cpp
	Translation.cpp
)
