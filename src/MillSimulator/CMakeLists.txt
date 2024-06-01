cmake_minimum_required(VERSION 3.10)
project(glMillSimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

set(QT_SOURCES
    DlgCAMSimulator.cpp
)


# Include directories
#include_directories(${CMAKE_SOURCE_DIR}/include)


# Option to select whether to use Qt or GLEW
option(USE_QT "Use Qt for the project" OFF)
if (USE_QT)

# Add executable
add_executable(glMillSimulator ${SOURCES} ${QT_SOURCES})

find_package(Qt5 COMPONENTS Gui REQUIRED)
message(${Qt5Gui_INCLUDE_DIRS})
target_include_directories(glMillSimulator PRIVATE ${Qt5Gui_INCLUDE_DIRS})
target_link_libraries(glMillSimulator Qt5::Gui)

else()

# Add executable
add_executable(glMillSimulator ${SOURCES})

add_definitions(-DCAM_SIM_USE_GLEW)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Link libraries
target_link_libraries(glMillSimulator glfw ${GLEW_LIBRARIES} GL)
endif()

#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES} GL)

