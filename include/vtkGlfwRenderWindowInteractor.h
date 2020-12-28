#ifndef vtkGlfwRenderWindowInteractor_h
#define vtkGlfwRenderWindowInteractor_h

#include "vtkRenderWindowInteractor.h"
#include <GLFW/glfw3.h>

class vtkGlfwRenderWindowInteractor
  : public vtkRenderWindowInteractor
{
public:
  /**
   * Construct object so that light follows camera motion.
   */
  static vtkGlfwRenderWindowInteractor* New();

  vtkTypeMacro(vtkGlfwRenderWindowInteractor, vtkRenderWindowInteractor);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Initialize the event handler
   */
  virtual void Initialize() override;
  
  /**
   * Enable/Disable interactions.  By default interactors are enabled when
   * initialized.  Initialize() must be called prior to enabling/disabling
   * interaction. These methods are used when a window/widget is being
   * shared by multiple renderers and interactors.  This allows a "modal"
   * display where one interactor is active when its data is to be displayed
   * and all other interactors associated with the widget are disabled
   * when their data is not displayed.
   */
  virtual void Enable() override;
  virtual void Disable() override;

  /**
   * Run the event loop and return. This is provided so that you can
   * implement your own event loop but yet use the vtk event handling as
   * well.
   */
  void ProcessEvents() override;

  /**
   * SDL2 specific application terminate, calls ClassExitMethod then
   * calls PostQuitMessage(0) to terminate the application. An application can
   * Specify ExitMethod for alternative behavior (i.e. suppression of keyboard
   * exit)
   */
  void TerminateApp(void) override;

  /**
   * These methods correspond to the Exit, User and Pick
   * callbacks. They allow for the Style to invoke them.
   */
  void ExitCallback() override;

  virtual int OnChar(GLFWwindow* wnd, unsigned int codepoint);
  virtual int OnDrop(GLFWwindow* wnd, int count, const char** paths);
  virtual int OnEnter(GLFWwindow* wnd, int entered);
  virtual int OnMouseMove(GLFWwindow* wnd, double x, double y);
  virtual int OnMouseBtn(GLFWwindow* wnd, int button, int action, int mods);
  virtual int OnMouseWhl(GLFWwindow* wnd, double x, double y);
  virtual int OnKey(GLFWwindow* wnd,
                     int key,
                     int scancode,
                     int action,
                     int mods);
  virtual int OnSize(GLFWwindow* wnd, int w, int h);

protected:
  vtkGlfwRenderWindowInteractor();
  ~vtkGlfwRenderWindowInteractor() override;

  bool InstallCallbacks;
  bool MouseInWindow;
  
  //@{
  /**
   * SDL2-specific internal timer methods. See the superclass for detailed
   * documentation.
   */
  int InternalCreateTimer(int timerId,
                          int timerType,
                          unsigned long duration) override;
  int InternalDestroyTimer(int platformTimerId) override;
  //@}

  /**
   * This will start up the event loop and never return. If you
   * call this method it will loop processing events until the
   * application is exited.
   */
  void StartEventLoop() override;

private:
  vtkGlfwRenderWindowInteractor(const vtkGlfwRenderWindowInteractor&) = delete;
  void operator=(const vtkGlfwRenderWindowInteractor&) = delete;
};

#endif