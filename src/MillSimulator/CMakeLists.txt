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

# Include directories
#include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(glMillSimulator ${SOURCES})

# Option to select whether to use Qt or GLEW
option(USE_QT "Use Qt for the project" OFF)
if (USE_QT)

find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries(helloworld Qt5::Core)

else()

add_definitions(-DCAM_SIM_USE_GLEW)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Link libraries
target_link_libraries(glMillSimulator glfw ${GLEW_LIBRARIES} GL)
endif()

#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES} GL)

