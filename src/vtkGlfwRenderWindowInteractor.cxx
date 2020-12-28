#include "vtkCommand.h"
#include "vtkGlfwRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkStringArray.h"

namespace vtkGlfwRenderWindowInteractor_detail {
void
charCallback(GLFWwindow* wnd, unsigned int codepoint)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnChar(wnd, codepoint);
}
void
dropCallback(GLFWwindow* wnd, int count, const char** paths)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnDrop(wnd, count, paths);
}
void
enterCallback(GLFWwindow* wnd, int entered)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnEnter(wnd, entered);
}
void
cursorPosCallback(GLFWwindow* wnd, double x, double y)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnMouseMove(wnd, x, y);
}
void
mouseBtnCallback(GLFWwindow* wnd, int button, int action, int mods)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnMouseBtn(wnd, button, action, mods);
}
void
mouseWhlCallback(GLFWwindow* wnd, double x, double y)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnMouseWhl(wnd, x, y);
}
void
keyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
  auto inst = reinterpret_cast<vtkGlfwRenderWindowInteractor*>(
    glfwGetWindowUserPointer(wnd));
  inst->OnKey(wnd, key, scancode, action, mods);
}
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
  , MouseInWindow(true)
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

  while (!(this->Done || glfwWindowShouldClose(wnd))) {
    this->ProcessEvents();
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

int
vtkGlfwRenderWindowInteractor::OnChar(GLFWwindow* wnd, unsigned int codepoint)
{
  if (!this->Enabled)
    return 0;

  int alt = glfwGetKey(wnd, GLFW_MOD_ALT);
  int ctrl = glfwGetKey(wnd, GLFW_MOD_CONTROL);
  int shift = glfwGetKey(wnd, GLFW_MOD_SHIFT);
  this->SetAltKey(alt);

  this->SetKeyEventInformation(ctrl, shift, codepoint);
  return this->InvokeEvent(vtkCommand::CharEvent, nullptr);
}

int
vtkGlfwRenderWindowInteractor::OnDrop(GLFWwindow* wnd,
                                      int count,
                                      const char** paths)
{
  if (!this->Enabled)
    return 0;

  double location[2] = {};
  glfwGetCursorPos(wnd, location, location + 1);
  this->InvokeEvent(vtkCommand::UpdateDropLocationEvent, location);

  if (count < 0)
    return 0;

  vtkNew<vtkStringArray> filePaths;
  filePaths->Allocate(count);
  for (int i = 0; i < count; ++i)
    filePaths->InsertNextValue(paths[i]);

  return this->InvokeEvent(vtkCommand::DropFilesEvent, filePaths);
}

int
vtkGlfwRenderWindowInteractor::OnEnter(GLFWwindow* wnd, int entered)
{
  if (!this->Enabled)
    return 0;

  this->MouseInWindow = entered;
  if (entered)
    return this->InvokeEvent(vtkCommand::EnterEvent, nullptr);
  else
    return this->InvokeEvent(vtkCommand::LeaveEvent, nullptr);
}

int
vtkGlfwRenderWindowInteractor::OnMouseMove(GLFWwindow* wnd, double x, double y)
{
  if (!this->Enabled)
    return 0;
;
  if (!this->MouseInWindow)
    return 0;

  int alt = glfwGetKey(wnd, GLFW_MOD_ALT);
  int ctrl = glfwGetKey(wnd, GLFW_MOD_CONTROL);
  int shift = glfwGetKey(wnd, GLFW_MOD_SHIFT);
  this->SetAltKey(alt);
  this->SetEventInformationFlipY(x, y, ctrl, shift);
  return this->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
}

int
vtkGlfwRenderWindowInteractor::OnMouseBtn(GLFWwindow* wnd,
                                          int button,
                                          int action,
                                          int mods)
{
  if (!this->Enabled)
    return 0;

  int alt = mods & GLFW_MOD_ALT;
  int ctrl = mods & GLFW_MOD_CONTROL;
  int shift = mods & GLFW_MOD_SHIFT;
  this->SetAltKey(alt);

  double x(0), y(0);
  glfwGetCursorPos(wnd, &x, &y);
  this->SetEventInformationFlipY(x, y, ctrl, shift);

  int retval(0);
  switch (action) {
    case GLFW_PRESS:
      switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
          retval = this->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
        case GLFW_MOUSE_BUTTON_MIDDLE:
          retval =
            this->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
        case GLFW_MOUSE_BUTTON_RIGHT:
          retval =
            this->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
        default:
          break;
      }
      break;
    case GLFW_RELEASE:
      switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
          retval =
            this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
        case GLFW_MOUSE_BUTTON_MIDDLE:
          retval =
            this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
        case GLFW_MOUSE_BUTTON_RIGHT:
          retval =
            this->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
        default:
          break;
      }
      break;
    default:
      break;
  }

  return retval;
}

int
vtkGlfwRenderWindowInteractor::OnMouseWhl(GLFWwindow* wnd, double x, double y)
{
  if (!this->Enabled)
    return 0;

  int alt = glfwGetKey(wnd, GLFW_MOD_ALT);
  int ctrl = glfwGetKey(wnd, GLFW_MOD_CONTROL);
  int shift = glfwGetKey(wnd, GLFW_MOD_SHIFT);
  this->SetAltKey(alt);
  this->SetControlKey(ctrl);
  this->SetShiftKey(shift);

  if (y > 0)
    return this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
  else
    return this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
}

int
vtkGlfwRenderWindowInteractor::OnKey(GLFWwindow* wnd,
                                     int key,
                                     int scancode,
                                     int action,
                                     int mods)
{
  if (!this->Enabled)
    return 0;

  int alt = mods & GLFW_MOD_ALT;
  int ctrl = mods & GLFW_MOD_CONTROL;
  int shift = mods & GLFW_MOD_SHIFT;

  const char* keysym = glfwGetKeyName(key, scancode);
  int repeat = (action == GLFW_REPEAT);
  this->SetKeyEventInformation(ctrl, shift, scancode, repeat, keysym);

  if (action == GLFW_RELEASE)
    return this->InvokeEvent(vtkCommand::KeyReleaseEvent, nullptr);
  else
    return this->InvokeEvent(vtkCommand::KeyPressEvent, nullptr);
}

int
vtkGlfwRenderWindowInteractor::OnSize(GLFWwindow* wnd, int w, int h)
{
  if (!this->Enabled)
    return 0;

  this->UpdateSize(w, h);

  if (this->Enabled)
    return this->InvokeEvent(vtkCommand::ConfigureEvent, nullptr);

  return 0;
}
