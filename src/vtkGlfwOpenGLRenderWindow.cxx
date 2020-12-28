#include <string>

#include "vtkCommand.h"
#include "vtkIdList.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkOpenGLVertexBufferObjectCache.h"
#include "vtkRendererCollection.h"

// clang-format off
#include "vtkGlfwOpenGLRenderWindow.h"
#include "vtkGlfwRenderWindowInteractor.h"
// clang-format on

vtkStandardNewMacro(vtkGlfwOpenGLRenderWindow);

const std::string vtkGlfwOpenGLRenderWindow::DEFAULT_BASE_WINDOW_NAME =
  "Visualization Toolkit - SDL2OpenGL #";

vtkGlfwOpenGLRenderWindow::vtkGlfwOpenGLRenderWindow()
  : WindowId(nullptr)
  , ContextId(nullptr)
{
  this->SetWindowName(DEFAULT_BASE_WINDOW_NAME.c_str());
  this->SetStencilCapable(1);

  // set position to -1 to let SDL place the window
  // SetPosition will still work. Defaults of 0,0 result
  // in the window title bar being off screen.
  this->Position[0] = -1;
  this->Position[1] = -1;
}

vtkGlfwOpenGLRenderWindow::~vtkGlfwOpenGLRenderWindow()
{
  this->Finalize();

  vtkRenderer* ren;
  vtkCollectionSimpleIterator rit;
  this->Renderers->InitTraversal(rit);
  while ((ren = this->Renderers->GetNextRenderer(rit))) {
    ren->SetRenderWindow(nullptr);
  }
}

void
vtkGlfwOpenGLRenderWindow::Clean()
{
  /* finish OpenGL rendering */
  if (this->OwnContext && this->ContextId) {
    this->MakeCurrent();
    this->CleanUpRenderers();
  }
  this->ContextId = nullptr;
}

void
vtkGlfwOpenGLRenderWindow::CleanUpRenderers()
{
  // tell each of the renderers that this render window/graphics context
  // is being removed (the RendererCollection is removed by vtkRenderWindow's
  // destructor)
  this->ReleaseGraphicsResources(this);
}

void
vtkGlfwOpenGLRenderWindow::SetWindowName(const char* title)
{
  this->Superclass::SetWindowName(title);
  if (this->WindowId)
    glfwSetWindowTitle(this->WindowId, title);
}

//------------------------------------------------------------------------------
void
vtkGlfwOpenGLRenderWindow::MakeCurrent()
{
  if (this->WindowId)
  {
    glfwMakeContextCurrent(this->WindowId);
    this->ContextId = this->WindowId;
  }
}

void
vtkGlfwOpenGLRenderWindow::PushContext()
{
  auto current = glfwGetCurrentContext();
  this->ContextStack.push(current);
  this->WindowStack.push(current);
  if (current != this->ContextId)
    this->MakeCurrent();
}

void
vtkGlfwOpenGLRenderWindow::PopContext()
{
  auto current = glfwGetCurrentContext();
  auto target = this->ContextStack.top();
  auto wind = this->WindowStack.top();
  this->ContextStack.pop();
  this->WindowStack.pop();
  if (target != current)
    glfwMakeContextCurrent(wind);
}

//------------------------------------------------------------------------------
// Description:
// Tells if this window is the current OpenGL context for the calling thread.
bool
vtkGlfwOpenGLRenderWindow::IsCurrent()
{
  return this->WindowId == glfwGetCurrentContext();
}

bool
vtkGlfwOpenGLRenderWindow::SetSwapControl(int i)
{
  glfwSwapInterval(i);
  return true;
}

//------------------------------------------------------------------------------
void
vtkGlfwOpenGLRenderWindow::SetSize(int x, int y)
{
  if ((this->Size[0] != x) || (this->Size[1] != y)) {
    this->Superclass::SetSize(x, y);

    if (this->Interactor) {
      this->Interactor->SetSize(x, y);
    }
    if (this->WindowId) {
      glfwSetWindowSize(this->WindowId, x, y);
    }
    this->Render();
  }
}

void
vtkGlfwOpenGLRenderWindow::SetPosition(int x, int y)
{
  if ((this->Position[0] != x) || (this->Position[1] != y)) {
    this->Modified();
    this->Position[0] = x;
    this->Position[1] = y;
    if (this->Mapped) {
      glfwSetWindowPos(this->WindowId, x, y);
    }
  }
}

void
vtkGlfwOpenGLRenderWindow::Frame()
{
  this->Superclass::Frame();
  if (!this->AbortRender && this->DoubleBuffer && this->SwapBuffers) {
    glfwSwapBuffers(this->WindowId);
  }
}

int
vtkGlfwOpenGLRenderWindow::GetColorBufferSizes(int* rgba)
{
  if (rgba == nullptr) {
    return 0;
  }
  rgba[0] = 8;
  rgba[1] = 8;
  rgba[2] = 8;
  rgba[3] = 8;
  return 1;
}

void
vtkGlfwOpenGLRenderWindow::SetShowWindow(bool val)
{
  if (val == this->ShowWindow) {
    return;
  }

  if (this->WindowId) {
    if (val) {
      glfwShowWindow(this->WindowId);
    } else {
      glfwHideWindow(this->WindowId);
    }
    this->Mapped = val;
  }
  this->Superclass::SetShowWindow(val);
}

void
vtkGlfwOpenGLRenderWindow::CreateAWindow()
{
  int height = ((this->Size[1] > 0) ? this->Size[1] : 300);
  int width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  this->SetSize(width, height);

  this->WindowId = glfwCreateWindow(width,
                                    height,
                                    this->WindowName,
                                    NULL,
                                    (GLFWwindow*)this->GetGenericParentId());
  this->MakeCurrent();

  if (this->WindowId) {
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    int wmm(0), hmm(0);
    glfwGetMonitorPhysicalSize(mon, &wmm, &hmm);
    float dpi = (float)mode->width * (float)mode->height /
                ((float)wmm * (float)hmm * 0.0393701f * 0.0393701f);
    float xs(0), ys(0);
    glfwGetMonitorContentScale(mon, &xs, &ys);
    vtkDebugMacro(<< "Pixels " << mode->width << "x" << mode->height);
    vtkDebugMacro(<< "Screen " << wmm << "x" << hmm << "(mm)");
    vtkDebugMacro(<< "Red" << mode->redBits);
    vtkDebugMacro(<< "Blue" << mode->blueBits);
    vtkDebugMacro(<< "Green" << mode->greenBits);
    vtkDebugMacro(<< "Refresh-Rate" << mode->refreshRate);
    vtkDebugMacro(<< "Native" << dpi << "DPI");
    vtkDebugMacro(<< "Current" << xs * dpi << "DPI");
    vtkDebugMacro(<< "x-scale" << xs);
    vtkDebugMacro(<< "y-scale" << ys);
    this->SetDPI(xs * dpi);
  }
}

// Initialize the rendering window.
void
vtkGlfwOpenGLRenderWindow::Initialize()
{
  if (!glfwInit())
    vtkErrorMacro(<< "Error initializing GLFW3 ");

  glfwWindowHint(GLFW_SAMPLES, 0);
#ifdef GL_ES_VERSION_3_0
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  if (!this->WindowId) {
    this->CreateAWindow();
  }

  if (!this->ContextId) {
    this->ContextId = glfwGetCurrentContext();
  }
  if (!this->ContextId) {
    vtkErrorMacro("Unable to create GLFW3 opengl context");
  }
  this->OpenGLInit();
}

void
vtkGlfwOpenGLRenderWindow::Finalize()
{
  this->DestroyWindow();
}

void
vtkGlfwOpenGLRenderWindow::DestroyWindow()
{
  this->Clean();
  if (this->WindowId) {
    auto wnd = static_cast<GLFWwindow*>(this->WindowId);
    glfwDestroyWindow(this->WindowId);
    this->WindowId = nullptr;
  }
}

// Get the current size of the window.
int*
vtkGlfwOpenGLRenderWindow::GetSize(void)
{
  // if we aren't mapped then just return the ivar
  if (this->WindowId && this->Mapped) {
    auto wnd = static_cast<GLFWwindow*>(this->WindowId);
    glfwGetWindowSize(wnd, this->Size, this->Size + 1);
  }

  return this->vtkOpenGLRenderWindow::GetSize();
}

// Get the size of the whole screen.
int*
vtkGlfwOpenGLRenderWindow::GetScreenSize(void)
{
  auto wnd = static_cast<GLFWwindow*>(this->WindowId);
  GLFWmonitor* mon = glfwGetWindowMonitor(wnd);
  const GLFWvidmode* mode = glfwGetVideoMode(mon);
  this->Size[0] = mode->width;
  this->Size[1] = mode->height;
  return this->Size;
}

// Get the position in screen coordinates of the window.
int*
vtkGlfwOpenGLRenderWindow::GetPosition(void)
{
  // if we aren't mapped then just return the ivar
  if (!this->Mapped) {
    return this->Position;
  }

  auto wnd = static_cast<GLFWwindow*>(this->WindowId);
  //  Find the current window position
  glfwGetWindowPos(wnd, this->Position, this->Position + 1);
  return this->Position;
}

// Change the window to fill the entire screen.
void
vtkGlfwOpenGLRenderWindow::SetFullScreen(vtkTypeBool arg)
{
  if (this->FullScreen == arg) {
    return;
  }

  if (!this->Mapped) {
    return;
  }

  // set the mode
  this->FullScreen = arg;
  auto wnd = static_cast<GLFWwindow*>(this->WindowId);
  GLFWmonitor* mon = glfwGetWindowMonitor(wnd);
  const GLFWvidmode* mode = glfwGetVideoMode(mon);
  glfwSetWindowMonitor(
    wnd, (arg ? mon : NULL), 0, 0, mode->width, mode->height, mode->refreshRate);

  this->Modified();
}

void
vtkGlfwOpenGLRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ContextId: " << this->ContextId << "\n";
  os << indent << "Window Id: " << this->WindowId << "\n";
}

//------------------------------------------------------------------------------
void
vtkGlfwOpenGLRenderWindow::HideCursor()
{
  auto wnd = static_cast<GLFWwindow*>(this->WindowId);
  glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

//------------------------------------------------------------------------------
void
vtkGlfwOpenGLRenderWindow::ShowCursor()
{
  auto wnd = static_cast<GLFWwindow*>(this->WindowId);
  glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}