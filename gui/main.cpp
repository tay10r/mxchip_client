#include <mxchip_client/client.h>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <implot.h>

#include <GLFW/glfw3.h>

#include <vector>

#include <cstdlib>
#include <cmath>

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

    uv_loop_init(&loop_);

    client_ = mxchip_client_new(&loop_);

    mxchip_client_set_user_data(client_, this);
  }

  ~program()
  {
    mxchip_client_close(client_, nullptr);

    uv_run(&loop_, UV_RUN_DEFAULT);

    uv_loop_close(&loop_);

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

      uv_run(&loop_, UV_RUN_NOWAIT);
    }

    return true;
  }

protected:
  static void on_mxchip_read(void* self_ptr, mxchip_client*, const struct mxchip_data* data)
  {
    auto* self = static_cast<program*>(self_ptr);

    const float av[3]{
      data->accelerometer[0],
      data->accelerometer[1],
      data->accelerometer[2]
    };

    const auto vib = std::sqrt(av[0] * av[0] + av[1] * av[1] + av[2] * av[2]);

    self->vibration_.emplace_back(vib);

    self->time_.emplace_back(data->time);
  }

  static void on_mxchip_connect(void* self_ptr, mxchip_client*, const int status)
  {
    auto* self = static_cast<program*>(self_ptr);

    if (status == 0)
      mxchip_client_start_read(self->client_, on_mxchip_read);
  }

  void render_chart_widget(const char* title, const char* units, const std::vector<float>& data)
  {
    if (!ImPlot::BeginPlot(title, ImVec2(-1, -1)))
      return;

    ImPlot::SetupAxes("Time", units, ImPlotAxisFlags_AutoFit);

    ImPlot::PlotLine(title, time_.data(), data.data(), time_.size());

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
      mxchip_client_connect(client_, ip_.c_str(), on_mxchip_connect);
    }

    if (ImGui::BeginTabBar("##Tabs")) {

      if (ImGui::BeginTabItem("Accelerometer")) {

        render_chart_widget("Accelerometer", "mg", vibration_);

        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    ImGui::End();
  }

private:
  GLFWwindow* window_;

  std::string ip_{ "192.168.1.137" };

  uv_loop_t loop_{};

  mxchip_client* client_{ nullptr };

  std::vector<float> vibration_;

  std::vector<float> time_;
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
