# Create a variable with all source files
set(SOURCES
    EndMill.cpp
    EndMillBall.cpp
    EndMillFlat.cpp
    EndMillTaper.cpp
    GCodeParser.cpp
    GlUtils.cpp
    GuiDisplay.cpp
    main.cpp
    MillPathSegment.cpp
    MillSimulation.cpp
    Shader.cpp
    SimShapes.cpp
    StockObject.cpp
    Texture.cpp
    TextureLoader.cpp
)

add_definitions(-DCAM_SIM_USE_GLEW)

# Include directories
#include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(glMillSimulator ${SOURCES})

find_package(GLEW REQUIRED)
#find_package(GLFW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES} GL)

# Link libraries
target_link_libraries(glMillSimulator glfw ${GLEW_LIBRARIES} GL)