#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glew.h"
#include <GLFW/glfw3.h>

#include <math.h>
#include <iostream>
#include <format>
#include <ranges>

#include "cubic_bezier_spline_2d/cubic_bezier_spline_2d.h"
#include "cubic_hermite_spline_2d/cubic_hermite_spline_2d.h"
#include "cubic_bspline_2d/cubic_bspline_2d.h"
#include "discretization/discretization.h"

enum class spline_type
{
	BEZIER,
    HERMITE,
    BSPLINE
};

struct data
{
    std::vector < std::vector< glm::vec2 > > splines_points;
    std::vector<spline_type> splines_type;
    std::vector<glm::uvec3> splines_color;
    std::vector<int32_t> splines_discretization;
	std::vector<bool> splines_draw_control_polygon;
    std::vector<bool> splines_draw_normals;

    void add_spline
    (
        const std::vector<glm::vec2>& spline_points,
        const spline_type spline_type,
        const glm::uvec3 spline_color = glm::uvec3(255, 255, 255),
		const int32_t spline_discretization = 100
	)
	{
		splines_points.push_back(spline_points);
		splines_type.push_back(spline_type);
		splines_color.push_back(spline_color);
		splines_discretization.push_back(spline_discretization);
	}
    
	void remove_spline(size_t index)
	{
		splines_points.erase(splines_points.begin() + index);
		splines_type.erase(splines_type.begin() + index);
		splines_color.erase(splines_color.begin() + index);
		splines_discretization.erase(splines_discretization.begin() + index);
	}
};

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

static void draw_discrete_points(const data& data, const glm::vec2& origin, const bool draw_normals)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < data.splines_points.size(); i++)
    {
        std::vector<glm::vec2> points;
        const std::vector<glm::vec2>& control_points = data.splines_points[i];
        const int32_t discretization = data.splines_discretization[i];
        switch (data.splines_type[i])
        {
            using enum spline_type;
            case BEZIER:  { points = Discretization::Linear(CubicBezierSpline2d(control_points), discretization);  } break;
            case HERMITE: { points = Discretization::Linear(CubicHermiteSpline2d(control_points), discretization); } break;
            case BSPLINE: { points = Discretization::Linear(CubicBSpline2d(control_points), discretization);       } break;
            default:                                                                                                 break;
        }

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
        if (!points.empty())
        {
            draw_list->AddCircleFilled(ImVec2(origin.x + points.back().x, origin.y + points.back().y), 3, IM_COL32(255, 0, 0, 255));
        }
    }
}

static void draw_control_points(const data& data, const glm::vec2& origin, const glm::vec2& mouse_pos_in_canvas, const float point_radius, const bool draw_control_polygon)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

	for (size_t i = 0; i < data.splines_points.size(); i++)
	{
        const glm::vec2* previous_point = nullptr;

        for (const auto& point : data.splines_points[i])
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

static void move_point(data& data, const bool is_canvas_hovered, int& selected_curve, int& selected_point, const glm::vec2& mouse_pos_in_canvas, const float point_radius)
{
    if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        int point_near_mouse_id = -1;
		auto point_is_near_mouse = [&](const glm::vec2& point)
		{
			glm::vec2 mouse_to_point = glm::vec2(mouse_pos_in_canvas.x, mouse_pos_in_canvas.y) - point;
			return glm::length(mouse_to_point) < point_radius;
		};
        
        for (size_t i = 0; i < data.splines_points.size(); i++)
        {
			const auto& control_points = data.splines_points[i];
            if (auto it = std::ranges::find_if(control_points, point_is_near_mouse); it != control_points.end())
            {
                point_near_mouse_id = static_cast<int>(std::distance(control_points.begin(), it));
            }
        
            if(point_near_mouse_id != -1)
            {
                selected_curve = static_cast<int>(i);
                selected_point = point_near_mouse_id;
            }
        }
         
    }
    
    if (selected_point != -1 && selected_curve != -1)
    {
        data.splines_points[selected_curve][selected_point] =  {mouse_pos_in_canvas.x, mouse_pos_in_canvas.y};
        draw_point_info(data.splines_points[selected_curve][selected_point]);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            selected_curve = -1;
            selected_point = -1;
        }
    } 
}

static void ShowPropertiesWindow()
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        // Top
        {
            ImGui::BeginChild("top pane", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);
            ImGui::Text("General settings");
            ImGui::EndChild();
        }
        
        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            for (int i = 0; i < 10; i++)
            {
                if (ImGui::Selectable(std::format("Spline {}", i).c_str(), selected == i))
                {
                    selected = i;
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("Spline: %d", selected);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Properties"))
                {
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                        ImGui::BeginChild("ChildR", ImVec2(0, 150), ImGuiChildFlags_Borders, ImGuiWindowFlags_MenuBar);
                        if (ImGui::BeginMenuBar())
                        {
                            if (ImGui::BeginMenu("Points"))
                            {
                                ImGui::EndMenu();
                            }
                            ImGui::EndMenuBar();
                        }
                        if (ImGui::BeginTable("split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
                        {
                            for (int i = 0; i < 10; i++)
                            {                                
                                ImGui::TableNextColumn();
                                ImGui::Button(std::format("{}", i).c_str(), ImVec2(-FLT_MIN, 0.0f));
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndChild();
                        ImGui::PopStyleVar();
                    }
                    static int discretization = 100;
                    static bool draw_control_polygon = false;
					static bool draw_normals = false;
                    ImGui::SliderInt("Discretization", &discretization, 2, 200);
                    ImGui::Checkbox("Show control polygon", &draw_control_polygon);
                    ImGui::Checkbox("Draw normals", &draw_normals);
                    
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Derivatives"))
                {
                    // TODO : Draw the canvas of the curve derivatives
                    ImGui::Text("TODO");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("Delete")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

int main() 
{
    GLFWwindow* window = nullptr;   

    init_glfw_and_imgui(window);

    const ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    static bool show_window = false;
    static bool opt_enable_grid = true;
    static bool opt_enable_context_menu = true;
    static float point_radius = 5.;
    static int discretization = 100;
    static bool draw_control_polygon = false;
    static bool draw_normals = false;
    
    static glm::vec2 scrolling(0.0f, 0.0f);
	static int selected_curve = -1;
    static int selected_point = -1;
    
    data data;
    std::vector<glm::vec2> bezier_control_points = { {0, 0}, {0, 100}, {100, 100}, {100, 0}, {200, 0}, {200, 100}, {200, 200}, {300, 0} };
    std::vector<glm::vec2> bspline_control_points = { {0, 0}, {0, 100}, {100, 100}, {100, 0}, {200, 0}, {200, 100}, {200, 200}, {300, 0} };
    data.add_spline(bezier_control_points, spline_type::BEZIER);
    data.add_spline(bspline_control_points, spline_type::BSPLINE);

    while (!glfwWindowShouldClose(window)) 
    {
        // Data
        CubicBSpline2d bezier_spline(bezier_control_points);
        std::vector<glm::vec2> bezier_points = Discretization::Linear(bezier_spline, discretization);

        // GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::Checkbox("Enable grid", &opt_enable_grid); ImGui::SameLine();
        ImGui::Checkbox("Enable context menu", &opt_enable_context_menu); ImGui::SameLine();
        ImGui::Checkbox("Show demo window", &show_window);

        ImGui::Checkbox("Show control polygon", &draw_control_polygon);
        ImGui::Checkbox("Draw normals", &draw_normals);
        ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
        //ImGui::Text("Number of points: %d", points.size());
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

        move_point(data, is_canvas_hovered, selected_curve, selected_point, mouse_pos_in_canvas, point_radius);

        pan(is_active, opt_enable_context_menu, scrolling);

        //draw_context_menu(opt_enable_context_menu, adding_line, points);

        draw_grid(opt_enable_grid, canvas_p0, canvas_sz, scrolling);

        draw_discrete_points(data, origin, draw_normals);

        draw_control_points(data, origin, mouse_pos_in_canvas, point_radius, draw_control_polygon);

        if (show_window) { ImGui::ShowDemoWindow(&show_window); }
        
        ShowPropertiesWindow();

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