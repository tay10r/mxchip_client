#include <mxchip_client/client.h>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <implot.h>

#include <GLFW/glfw3.h>

#include <stdlib.h>

namespace {

class program final
{
public:
  program(const char* title)
    : window_(glfwCreateWindow(640, 480, title, nullptr, nullptr))
  {
    glfwMakeContextCurrent(window_);

    glfwSwapInterval(1);

    gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glClearColor(0, 0, 0, 1);

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);

    ImGui_ImplOpenGL3_Init("#version 100");

    ImPlot::CreateContext();
  }

  ~program()
  {
    ImPlot::DestroyContext();

    ImGui_ImplOpenGL3_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
  }

  bool run()
  {
    while (!glfwWindowShouldClose(window_)) {

      glfwPollEvents();

      if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_, GLFW_TRUE);

      ImGui_ImplOpenGL3_NewFrame();

      ImGui_ImplGlfw_NewFrame();

      ImGui::NewFrame();

      render_main_window();

      ImGui::Render();

      int w, h;
      glfwGetFramebufferSize(window_, &w, &h);

      glViewport(0, 0, w, h);

      glClear(GL_COLOR_BUFFER_BIT);

      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window_);
    }

    return true;
  }

protected:
  void render_chart_widget(const char* title)
  {
    if (!ImPlot::BeginPlot(title, ImVec2(-1, -1)))
      return;

    ImPlot::EndPlot();
  }

  void render_main_window()
  {
    int w = 0;
    int h = 0;
    glfwGetWindowSize(window_, &w, &h);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);

    ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::InputText("IPv4", &ip_);

    ImGui::SameLine();

    if (ImGui::Button("Connect")) {
    }

    if (ImGui::BeginTabBar("##Tabs")) {

      if (ImGui::BeginTabItem("Accelerometer")) {

        render_chart_widget("Accelerometer");

        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    ImGui::End();
  }

private:
  GLFWwindow* window_;

  std::string ip_;

  uv_loop_t loop_;
};

} // namespace

int
main()
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  {
    program prg("MXChip Client");
    prg.run();
  }

  glfwTerminate();

  return EXIT_SUCCESS;
}
