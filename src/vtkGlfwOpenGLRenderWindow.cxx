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
  if (this->WindowId) {
    glfwSetWindowTitle(this->WindowId, title);
    // SDL_SetWindowTitle(this->WindowId, title);
  }
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
  // SDL_GLContext current = SDL_GL_GetCurrentContext();
  // this->ContextStack.push(current);
  // this->WindowStack.push(SDL_GL_GetCurrentWindow());
  // if (current != this->ContextId) {
  // this->MakeCurrent();
  //}
}

void
vtkGlfwOpenGLRenderWindow::PopContext()
{
  auto current = glfwGetCurrentContext();
  auto target = this->ContextStack.top();
  auto wind = this->WindowStack.top();
  // SDL_GLContext current = SDL_GL_GetCurrentContext();
  // SDL_GLContext target = this->ContextStack.top();
  // SDL_Window* win = this->WindowStack.top();
  this->ContextStack.pop();
  this->WindowStack.pop();
  if (target != current)
    glfwMakeContextCurrent(wind);
  // if (target != current) {
  // SDL_GL_MakeCurrent(win, target);
  //}
}

//------------------------------------------------------------------------------
// Description:
// Tells if this window is the current OpenGL context for the calling thread.
bool
vtkGlfwOpenGLRenderWindow::IsCurrent()
{
  return this->WindowId == glfwGetCurrentContext();
  // return this->ContextId != 0 && this->ContextId ==
  // SDL_GL_GetCurrentContext();
}

bool
vtkGlfwOpenGLRenderWindow::SetSwapControl(int i)
{
  glfwSwapInterval(i);
  // SDL_GL_SetSwapInterval(i);
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
      // SDL_SetWindowSize(this->WindowId, x, y);
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
      // SDL_SetWindowPosition(this->WindowId, x, y);
    }
  }
}

void
vtkGlfwOpenGLRenderWindow::Frame()
{
  this->Superclass::Frame();
  if (!this->AbortRender && this->DoubleBuffer && this->SwapBuffers) {
    glfwSwapBuffers(this->WindowId);
    // SDL_GL_SwapWindow(this->WindowId);
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
      // SDL_ShowWindow(this->WindowId);
    } else {
      glfwHideWindow(this->WindowId);
      // SDL_HideWindow(this->WindowId);
    }
    this->Mapped = val;
  }
  this->Superclass::SetShowWindow(val);
}

void
vtkGlfwOpenGLRenderWindow::CreateAWindow()
{
  // int x =
  //((this->Position[0] >= 0) ? this->Position[0] : SDL_WINDOWPOS_UNDEFINED);
  // int y =
  //((this->Position[1] >= 0) ? this->Position[1] : SDL_WINDOWPOS_UNDEFINED);
  int height = ((this->Size[1] > 0) ? this->Size[1] : 300);
  int width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  this->SetSize(width, height);

  this->WindowId = glfwCreateWindow(width,
                                    height,
                                    this->WindowName,
                                    NULL,
                                    (GLFWwindow*)this->GetGenericParentId());
  this->MakeCurrent();

  // this->WindowId = SDL_CreateWindow(this->WindowName,
  //                                  x,
  //                                  y,
  //                                  width,
  //                                  height,
  //                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  /* SDL_SetWindowResizable(this->WindowId, SDL_TRUE);*/
  // if (this->WindowId) {
  //  int idx = SDL_GetWindowDisplayIndex(this->WindowId);
  //  float hdpi = 72.0;
  //  SDL_GetDisplayDPI(idx, nullptr, &hdpi, nullptr);
  //  this->SetDPI(hdpi);
  //}
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
  // int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  // if (res) {
  // vtkErrorMacro("Error initializing SDL " << SDL_GetError());
  //}

  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

#ifdef GL_ES_VERSION_3_0
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
  // SDL_GL_CONTEXT_PROFILE_ES);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
  // SDL_GL_CONTEXT_PROFILE_CORE);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

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
    // SDL_DestroyWindow(this->WindowId);
    this->WindowId = nullptr;
  }
}

// Get the current size of the window.
int*
vtkGlfwOpenGLRenderWindow::GetSize(void)
{
  // if we aren't mapped then just return the ivar
  if (this->WindowId && this->Mapped) {
    int w = 0;
    int h = 0;

    // SDL_GetWindowSize(this->WindowId, &w, &h);
    this->Size[0] = w;
    this->Size[1] = h;
  }

  return this->vtkOpenGLRenderWindow::GetSize();
}

// Get the size of the whole screen.
int*
vtkGlfwOpenGLRenderWindow::GetScreenSize(void)
{
  // SDL_Rect rect;
  // SDL_GetDisplayBounds(0, &rect);
  // this->Size[0] = rect.w;
  // this->Size[1] = rect.h;

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

  //  Find the current window position
  //  x,y,&this->Position[0],&this->Position[1],&child);

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
  // SDL_SetWindowFullscreen(this->WindowId,
  //                        arg ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
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
  // SDL_ShowCursor(SDL_DISABLE);
}

//------------------------------------------------------------------------------
void
vtkGlfwOpenGLRenderWindow::ShowCursor()
{
  // SDL_ShowCursor(SDL_ENABLE);
}