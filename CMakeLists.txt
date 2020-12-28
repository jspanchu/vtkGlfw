cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project (vtkGlfw)

find_package (glfw3 REQUIRED)
find_package (VTK COMPONENTS
  RenderingCore
)

add_library (vtkGlfwOpenGLRenderWindow "${PROJECT_SOURCE_DIR}/src/vtkGlfwOpenGLRenderWindow.cxx")
target_include_directories (vtkGlfwOpenGLRenderWindow PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries (vtkGlfwOpenGLRenderWindow
  PUBLIC
    VTK::RenderingCore
    glfw
)
add_library (vtkGlfwRenderWindowInteractor "${PROJECT_SOURCE_DIR}/src/vtkGlfwRenderWindowInteractor.cxx")
target_include_directories (vtkGlfwRenderWindowInteractor PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries (vtkGlfwRenderWindowInteractor
  PUBLIC
    VTK::RenderingCore
  PRIVATE
    glfw
)

option (BUILD_DEMO "Build demo VTK+GLFW+OpenGL" ON)
if (BUILD_DEMO)
  find_package(VTK COMPONENTS 
    CommonColor
    CommonCore
    FiltersSources
    InteractionStyle
    RenderingCore
    RenderingFreeType
    RenderingOpenGL2 QUIET)
  if (VTK_VERSION VERSION_LESS "8.90.0")
    # old system
    include(${VTK_USE_FILE})
    add_executable(vtkGlfwDemo MACOSX_BUNDLE "${PROJECT_SOURCE_DIR}/src/vtkGlfwDemo.cpp" )
    target_link_libraries(vtkGlfwDemo PRIVATE ${VTK_LIBRARIES} vtkGlfwOpenGLRenderWindow vtkGlfwRenderWindowInteractor)
  else ()
    # include all components
    add_executable(vtkGlfwDemo MACOSX_BUNDLE "${PROJECT_SOURCE_DIR}/src/vtkGlfwDemo.cpp" )
    target_link_libraries(vtkGlfwDemo PRIVATE ${VTK_LIBRARIES} vtkGlfwOpenGLRenderWindow vtkGlfwRenderWindowInteractor)
    # vtk_module_autoinit is needed
    vtk_module_autoinit(
      TARGETS vtkGlfwDemo
      MODULES ${VTK_LIBRARIES}
      )
  endif ()
endif ()