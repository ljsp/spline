#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glew.h"
#include <GLFW/glfw3.h>

#include <math.h>
#include <iostream>

#include "cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "discretization/discretization.h"

static void init_glfw_and_imgui(GLFWwindow*& window)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Erreur : Impossible d'initialiser GLFW\n");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1280, 720, "Spline Editor", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Erreur : Impossible de creer une fenetre GLFW\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Erreur : Impossible d'initialiser GLEW\n");
        return;
    }

    // Activez vsync
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static void shutdown_glfw_and_imgui(GLFWwindow* const& window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

static void draw_context_menu(bool is_context_menu_drawn, bool& adding_line, std::vector<glm::vec2>& points)
{
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (is_context_menu_drawn && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
    {
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    }
    if (ImGui::BeginPopup("context"))
    {
        if (adding_line)
        {
            points.resize(points.size() - 2);
        }
        adding_line = false;
        if (ImGui::MenuItem("Remove one", nullptr, false, points.empty())) { points.resize(points.size() - 2); }
        if (ImGui::MenuItem("Remove all", nullptr, false, points.empty())) { points.clear(); }
        ImGui::EndPopup();
    }
}

static void draw_point_info(const glm::vec2& point)
{
    static ImVec2 offset(5.0f, 25.0f);
	ImVec2 mouse_pos = ImGui::GetMousePos();
    ImGui::SetNextWindowPos({mouse_pos.x + offset.x, mouse_pos.y - offset.y});
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        ImGui::OpenPopup("point info", ImGuiPopupFlags_MouseButtonLeft);
    }
    if (ImGui::BeginPopup("point info"))
    {
        ImGui::Text("(%f, %f)", point.x, point.y);
        ImGui::EndPopup();
    }
}

static void draw_border(const ImVec2& canvas_p0, const ImVec2& canvas_p1)
{
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
}

static void draw_grid(bool is_grid_drawn, const ImVec2& canvas_p0, const ImVec2& canvas_size, const glm::vec2& scrolling)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_p1(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y);
    draw_list->PushClipRect(canvas_p0, canvas_p1, true);
    if (is_grid_drawn)
    {
        const float GRID_STEP = 64.0f;
        for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_size.x; x += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        }
        for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_size.y; y += GRID_STEP)
        {
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
        }
    }
}

static void draw_discrete_points(const std::vector<glm::vec2>& points, const glm::vec2& origin, const bool draw_normals)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    for (int n = 0; n < points.size() - 1; n++)
    {
        const glm::vec2 point = points[n] + origin;
		const glm::vec2 next = points[n + 1] + origin;
        
        draw_list->AddLine({ point.x, point.y }, { next.x, next.y }, IM_COL32(255, 255, 0, 255), 2.0f);
        draw_list->AddCircleFilled(ImVec2(origin.x + points[n].x, origin.y + points[n].y), 3, IM_COL32(255, 0, 0, 255));
        if (draw_normals && n > 0)
        {
			glm::vec2 tangent = points[n + 1] - points[n - 1];
			glm::vec2 normal(-tangent.y, tangent.x);
            normal = glm::normalize(normal);
            normal *= 20.;
            draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n].x + normal.x, origin.y + points[n].y + normal.y), IM_COL32(0, 255, 0, 255), 2.0f);
        }
    
    }
    if(!points.empty())
    {
        draw_list->AddCircleFilled(ImVec2(origin.x + points.back().x, origin.y + points.back().y), 3, IM_COL32(255, 0, 0, 255));
    }
}

static void draw_control_points(const std::vector<glm::vec2>& control_points, const glm::vec2& origin, const glm::vec2& mouse_pos_in_canvas, const float point_radius, const bool draw_control_polygon)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const glm::vec2* previous_point = nullptr;

    for (const auto& point : control_points)
    {
        ImVec2 screen_pos(origin.x + point.x, origin.y + point.y);

        if (draw_control_polygon && previous_point)
        {
            ImVec2 prev_screen_pos(origin.x + previous_point->x, origin.y + previous_point->y);
            draw_list->AddLine(prev_screen_pos, screen_pos, IM_COL32(255, 255, 255, 255), 2.0f);
        }
        
        glm::vec2 mouse_to_point = glm::vec2(mouse_pos_in_canvas.x, mouse_pos_in_canvas.y) - point;
        ImU32 color = glm::length(mouse_to_point) < point_radius ? IM_COL32(0, 255, 0, 255) : IM_COL32(0, 0, 255, 255);
        draw_list->AddCircleFilled(screen_pos, point_radius, color);

        previous_point = &point;
    }
}

static void pan(bool is_active, bool is_context_menu_drawn, glm::vec2& scrolling)
{
    const ImGuiIO& io = ImGui::GetIO(); (void)io;
    const float mouse_threshold_for_pan = is_context_menu_drawn ? -1.0f : 0.0f;
    if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
    {
        scrolling.x += io.MouseDelta.x;
        scrolling.y += io.MouseDelta.y;
    }
}

static void add_line(const bool is_canvas_hovered, bool& adding_line, const ImVec2& mouse_pos_in_canvas, ImVector<ImVec2>& points)
{
    // Add first and second point
    if (is_canvas_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        points.push_back(mouse_pos_in_canvas);
        points.push_back(mouse_pos_in_canvas);
        adding_line = true;
    }
    if (adding_line)
    {
        points.back() = mouse_pos_in_canvas;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            adding_line = false;
    }
}

static void move_point(const bool is_canvas_hovered, int& selected_point, const glm::vec2& mouse_pos_in_canvas, std::vector<glm::vec2>& control_points, const float point_radius)
{
    if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        int point_near_mouse_id = -1;
		auto point_is_near_mouse = [&](const glm::vec2& point)
		{
			glm::vec2 mouse_to_point = glm::vec2(mouse_pos_in_canvas.x, mouse_pos_in_canvas.y) - point;
			return glm::length(mouse_to_point) < point_radius;
		};
        
        if (auto it = std::ranges::find_if(control_points, point_is_near_mouse); it != control_points.end())
        {
            point_near_mouse_id = static_cast<int>(std::distance(control_points.begin(), it));
        }
         
        if(point_near_mouse_id != -1)
        {
            selected_point = point_near_mouse_id;
        }
    }
    
    if (selected_point != -1)
    {
        control_points[selected_point] =  {mouse_pos_in_canvas.x, mouse_pos_in_canvas.y};
        draw_point_info(control_points[selected_point]);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selected_point = -1;
        }
    } 
}

int main() 
{
    GLFWwindow* window = nullptr;   

    init_glfw_and_imgui(window);

    const ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    static bool show_window = false;
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static bool adding_line = false;
    static float point_radius = 5.;
    static int discretization = 100;
    static bool draw_control_polygon = false;
    static bool draw_normals = false;
    
    static glm::vec2 scrolling(0.0f, 0.0f);
    static int selected_point = -1;
    static std::vector<glm::vec2> points;
    static std::vector<glm::vec2> bezier_control_points = { {0, 0}, {0, 100}, {100, 100}, {100, 0}, {200, 0}, {200, 100}, {200, 200}, {300, 0} };

    while (!glfwWindowShouldClose(window)) 
    {
        // Data
        CubicBezierSpline2d bezier_spline(bezier_control_points);
        std::vector<glm::vec2> bezier_points = Discretization::Linear(bezier_spline, discretization);
        points.clear();
        for (const auto& point : bezier_points)
        {
            points.emplace_back(point.x, point.y);
        }

        // GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Spline options", nullptr, window_flags);

        ImGui::Checkbox("Enable grid", &opt_enable_grid);
        ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
        ImGui::Checkbox("Show demo window", &show_window);
        ImGui::Checkbox("Show control polygon", &draw_control_polygon);
        ImGui::Checkbox("Draw normals", &draw_normals);
        ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
        ImGui::Text("Number of points: %d", points.size());
        ImGui::SliderFloat("Point size", &point_radius, 1., 20.);
		ImGui::SliderInt("Discretization", &discretization, 2, 200);
        
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_p1(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_canvas_hovered = ImGui::IsItemHovered();
        const bool is_active = ImGui::IsItemActive();
        const glm::vec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y);
        const glm::vec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

        draw_border(canvas_p0, canvas_p1);

        move_point(is_canvas_hovered, selected_point, mouse_pos_in_canvas, bezier_control_points, point_radius);

        pan(is_active, opt_enable_context_menu, scrolling);

        draw_context_menu(opt_enable_context_menu, adding_line, points);

        draw_grid(opt_enable_grid, canvas_p0, canvas_sz, scrolling);

        draw_discrete_points(points, origin, draw_normals);

        draw_control_points(bezier_control_points, origin, mouse_pos_in_canvas, point_radius, draw_control_polygon);

        if (show_window) { ImGui::ShowDemoWindow(&show_window); }

        ImGui::GetWindowDrawList()->PopClipRect();
        ImGui::End();
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shutdown_glfw_and_imgui(window);
    return 0;
}