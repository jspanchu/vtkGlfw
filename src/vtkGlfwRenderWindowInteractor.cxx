#include "vtkGlfwRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include <GLFW/glfw3.h>

#include "vtkStringArray.h"

#include "vtkCommand.h"
#include "vtkObjectFactory.h"

namespace vtkGlfwRenderWindowInteractor_detail {
void
charCallback(GLFWwindow* wnd, unsigned int codepoint)
{}
void
dropCallback(GLFWwindow* wnd, int count, const char** paths)
{}
void
cursorPosCallback(GLFWwindow* wnd, double x, double y)
{}
void
mouseBtnCallback(GLFWwindow* wnd, int button, int action, int mods)
{}
void
mouseWhlCallback(GLFWwindow* wnd, double x, double y)
{}
void
keyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{}
void
wnSizeCallback(GLFWwindow* wnd, int w, int h)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnSize(wnd, w, h);
}
}

vtkStandardNewMacro(vtkGlfwRenderWindowInteractor);

//------------------------------------------------------------------------------
// Construct object so that light follows camera motion.
vtkGlfwRenderWindowInteractor::vtkGlfwRenderWindowInteractor()
  : InstallCallbacks(true)
{}

//------------------------------------------------------------------------------
vtkGlfwRenderWindowInteractor::~vtkGlfwRenderWindowInteractor() {}

//------------------------------------------------------------------------------
void
vtkGlfwRenderWindowInteractor::ProcessEvents()
{
  // No need to do anything if this is a 'mapped' interactor
  if (!this->Enabled) {
    return;
  }
  glfwPollEvents();
}

//------------------------------------------------------------------------------
void
vtkGlfwRenderWindowInteractor::StartEventLoop()
{
  // No need to do anything if this is a 'mapped' interactor
  if (!this->Enabled) {
    return;
  }

  // make sure we have a RenderWindow and camera
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "No renderer defined!");
    return;
  }
  vtkRenderWindow* ren = this->RenderWindow;
  GLFWwindow* wnd = static_cast<GLFWwindow*>(ren->GetGenericWindowId());

  while (!this->Done) {
    this->ProcessEvents();
    this->Done = glfwWindowShouldClose(wnd);
  }
}

//------------------------------------------------------------------------------
// Begin processing keyboard strokes.
void
vtkGlfwRenderWindowInteractor::Initialize()
{
  int* size;

  // make sure we have a RenderWindow and camera
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "No renderer defined!");
    return;
  }
  if (this->Initialized) {
    return;
  }
  this->Initialized = 1;
  // get the info we need from the RenderingWindow
  vtkRenderWindow* ren = this->RenderWindow;
  GLFWwindow* wnd = static_cast<GLFWwindow*>(ren->GetGenericWindowId());
  glfwSetWindowUserPointer(wnd, this);
  ren->Start();
  ren->End();
  size = ren->GetSize();
  ren->GetPosition();

  this->Enable();
  this->Size[0] = size[0];
  this->Size[1] = size[1];
}

void
vtkGlfwRenderWindowInteractor::Enable()
{
  if (!this->Initialized)
    return;

  // make sure we have a RenderWindow and camera
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "No renderer defined!");
    return;
  }

  if (!this->RenderWindow)
    return;

  if (this->InstallCallbacks) {
    using namespace vtkGlfwRenderWindowInteractor_detail;
    vtkRenderWindow* ren = this->RenderWindow;
    auto wnd = static_cast<GLFWwindow*>(ren->GetGenericWindowId());
    glfwSetCharCallback(wnd, charCallback);
    glfwSetDropCallback(wnd, dropCallback);
    glfwSetCursorPosCallback(wnd, cursorPosCallback);
    glfwSetMouseButtonCallback(wnd, mouseBtnCallback);
    glfwSetScrollCallback(wnd, mouseWhlCallback);
    glfwSetKeyCallback(wnd, keyCallback);
    glfwSetWindowSizeCallback(wnd, wnSizeCallback);
  }
  this->Enabled = 1;
  this->Modified();
}

void
vtkGlfwRenderWindowInteractor::Disable()
{
  if (!this->Initialized)
    return;

  // make sure we have a RenderWindow and camera
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "No renderer defined!");
    return;
  }

  if (!this->RenderWindow)
    return;

  if (this->InstallCallbacks) {
    using namespace vtkGlfwRenderWindowInteractor_detail;
    vtkRenderWindow* ren = this->RenderWindow;
    auto wnd = static_cast<GLFWwindow*>(ren->GetGenericWindowId());
    glfwSetCharCallback(wnd, NULL);
    glfwSetDropCallback(wnd, NULL);
    glfwSetCursorPosCallback(wnd, NULL);
    glfwSetMouseButtonCallback(wnd, NULL);
    glfwSetScrollCallback(wnd, NULL);
    glfwSetKeyCallback(wnd, NULL);
    glfwSetWindowSizeCallback(wnd, NULL);
  }
  this->Enabled = 0;
  this->Modified();
}

//------------------------------------------------------------------------------
void
vtkGlfwRenderWindowInteractor::TerminateApp(void)
{
  this->Done = true;
}

//------------------------------------------------------------------------------
int
vtkGlfwRenderWindowInteractor::InternalCreateTimer(int timerId,
                                                   int vtkNotUsed(timerType),
                                                   unsigned long duration)
{
  return 1;
}

//------------------------------------------------------------------------------
int
vtkGlfwRenderWindowInteractor::InternalDestroyTimer(int platformTimerId)
{
  return 1;
}

//------------------------------------------------------------------------------
void
vtkGlfwRenderWindowInteractor::PrintSelf(ostream& os, vtkIndent indent)
{}

//------------------------------------------------------------------------------
void
vtkGlfwRenderWindowInteractor::ExitCallback()
{
  if (this->HasObserver(vtkCommand::ExitEvent)) {
    this->InvokeEvent(vtkCommand::ExitEvent, nullptr);
  }

  this->TerminateApp();
}

void
vtkGlfwRenderWindowInteractor::OnChar(GLFWwindow* wnd, unsigned int codepoint)
{}

void
vtkGlfwRenderWindowInteractor::OnDrop(GLFWwindow* wnd,
                                      int count,
                                      const char** paths)
{}

void
vtkGlfwRenderWindowInteractor::OnMouseMove(GLFWwindow* wnd, double x, double y)
{}

void
vtkGlfwRenderWindowInteractor::OnMouseBtn(GLFWwindow* wnd,
                                          int button,
                                          int action,
                                          int mods)
{}

void
vtkGlfwRenderWindowInteractor::OnMouseWhl(GLFWwindow* wnd, double x, double y)
{}

void
vtkGlfwRenderWindowInteractor::OnKey(GLFWwindow* wnd,
                                     int key,
                                     int scancode,
                                     int action,
                                     int mods)
{}

void
vtkGlfwRenderWindowInteractor::OnSize(GLFWwindow* wnd, int w, int h)
{
  this->UpdateSize(w, h);
  if (this->Enabled) {
    this->InvokeEvent(vtkCommand::ConfigureEvent, nullptr);
  }
}
