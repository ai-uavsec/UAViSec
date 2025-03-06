#include <boost/shared_ptr.hpp>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include <string>
#include <iostream>
#include <boost/program_options.hpp>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

namespace po = boost::program_options;

gazebo::transport::NodePtr node;

void fill_data(gazebo::msgs::Vector3d *data, std::vector<double> fill) {
    int size = fill.size();
    data->set_y(0.0);
    data->set_z(0.0);
    if (size > 0) {
        data->set_x(fill[0]);
        std::cout << "X: " << fill[0] << "\n";
    }
    if (size > 1) {
        data->set_y(fill[1]);
        std::cout << "Y: " << fill[1] << "\n";
    }
    if (size > 2) {
        data->set_z(fill[2]);
        std::cout << "Z: " << fill[2] << "\n";
    }
}

void publishSensorAttack(const std::string& topic, const std::vector<double>& values) {
    gazebo::transport::PublisherPtr pub = node->Advertise<gazebo::msgs::Vector3d>(topic, 10);
    gazebo::msgs::Vector3d data;
    fill_data(&data, values);
    pub->WaitForConnection();
    pub->Publish(data);
}

void publishWindAttack(const std::vector<double>& values) {
    gazebo::transport::PublisherPtr wind_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/wind", 10);
    gazebo::transport::PublisherPtr wind_enable_pub = node->Advertise<gazebo::msgs::Int>("~/attack/wind/enabled", 10);

    gazebo::msgs::Vector3d data;
    fill_data(&data, values);

    gazebo::msgs::Int enable_data;
    enable_data.set_data(0); 

    wind_pub->WaitForConnection();
    wind_pub->Publish(data);

    wind_enable_pub->WaitForConnection();
    wind_enable_pub->Publish(enable_data);
}

void publishInt(const std::string& topic, int value) {
    gazebo::transport::PublisherPtr pub = node->Advertise<gazebo::msgs::Int>(topic, 10);
    gazebo::msgs::Int data;
    data.set_data(value);
    pub->WaitForConnection();
    pub->Publish(data);
}

void processCommandLine(int _argc, char **_argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("abs", po::value<std::vector<double>>()->multitoken(), "Absolute GPS offset")
        ("rot", po::value<std::vector<double>>()->multitoken(), "Rotational offset")
        ("imu", po::value<std::vector<double>>()->multitoken(), "IMU offset")
        ("lidar", po::value<std::vector<double>>()->multitoken(), "Lidar offset")
        ("sonar", po::value<std::vector<double>>()->multitoken(), "Sonar offset")
        ("baro", po::value<std::vector<double>>()->multitoken(), "Barometer offset")
        ("mag", po::value<std::vector<double>>()->multitoken(), "Magnetometer offset")
        ("camera", po::value<double>()->multitoken(), "Camera")
        ("stream", po::value<double>()->multitoken(), "Stream")
        ("motor", po::value<std::vector<double>>()->multitoken(), "Motor")
        ("wind", po::value<std::vector<double>>()->multitoken(), "Toggle wind attack");

    po::positional_options_description p;
    p.add("abs", 3);
    p.add("rot", 2);
    p.add("imu", 3);
    p.add("sonar", 2);
    p.add("baro", 3);
    p.add("wind", 3);
    p.add("mag", 3);
    p.add("lidar", 3);
    po::variables_map vm;
    po::store(po::command_line_parser(_argc, _argv)
            .options(desc)
            .positional(p)
            .run(), vm);
    po::notify(vm);

    if (vm.count("rot")) {
        std::vector<double> offsets = vm["rot"].as<std::vector<double>>();
        publishSensorAttack("~/attack/gps", offsets);
    }

    if (vm.count("imu")) {
        std::cout << "IMU";
        std::vector<double> offsets = vm["imu"].as<std::vector<double>>();
        publishSensorAttack("~/attack/imu", offsets);
    }

    if (vm.count("lidar")) {
        std::vector<double> offsets = vm["lidar"].as<std::vector<double>>();
        publishSensorAttack("~/attack/lidar", offsets);
    }

    if (vm.count("sonar")) {
        std::vector<double> offsets = vm["sonar"].as<std::vector<double>>();
        publishSensorAttack("~/attack/sonar", offsets);
    }

    if (vm.count("baro")) {
        std::vector<double> offsets = vm["baro"].as<std::vector<double>>();
        publishSensorAttack("~/attack/baro", offsets);
    }

    if (vm.count("wind")) {
        std::vector<double> offsets = vm["wind"].as<std::vector<double>>();
        publishWindAttack(offsets);
    }

    if (vm.count("mag")) {
        std::vector<double> offsets = vm["mag"].as<std::vector<double>>();
        publishSensorAttack("~/attack/mag", offsets);
    }

    if (vm.count("camera")) {
        double value = vm["camera"].as<double>();
        publishInt("~/video_stream", (int)value);
    }

    if (vm.count("stream")) {
        double value = vm["stream"].as<double>();
        publishInt("~/attack/stream", (int)value);
    }

    if (vm.count("motor")) {
        std::vector<double> offsets = vm["motor"].as<std::vector<double>>();
        publishSensorAttack("~/attack/motor", offsets);
    }
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int _argc, char **_argv) {
    gazebo::client::setup();
    node.reset(new gazebo::transport::Node());
    node->Init();

    printf("It's cool man");

    // Process command line arguments if provided
    if (_argc > 1) {
        processCommandLine(_argc, _argv);
        gazebo::transport::fini();
        return 0;
    }

    printf("It's cool man");

    // Initialize GUI otherwise
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Sensor Attack Interface", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    static float gps_rot[3] = {0.0f, 0.0f, 0.0f};
    static float imu[3] = {0.0f, 0.0f, 0.0f};
    static float lidar[3] = {0.0f, 0.0f, 0.0f};
    static float sonar[3] = {0.0f, 0.0f, 0.0f};
    static float baro[3] = {0.0f, 0.0f, 0.0f};
    static float mag[3] = {0.0f, 0.0f, 0.0f};
    static float motor[3] = {0.0f, 0.0f, 0.0f};
    static float wind[3] = {0.0f, 0.0f, 0.0f};
    static int camera = 0;
    static int stream = 0;
    static bool wind_enabled = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set custom style for a more polished UI
        ImGui::GetStyle().FrameRounding = 4.0f;
        ImGui::GetStyle().GrabRounding = 4.0f;
        ImGui::GetStyle().WindowRounding = 0.0f;
        ImGui::GetStyle().FramePadding = ImVec2(8, 6);
        ImGui::GetStyle().ItemSpacing = ImVec2(10, 10);
        ImGui::GetStyle().ButtonTextAlign = ImVec2(0.5f, 0.5f);
        
        // Slightly nicer color scheme
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.40f, 0.50f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.30f, 0.40f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.20f, 0.21f, 1.00f);
        
        // Create a window that takes up entire screen
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Sensor Attack Interface", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        // Title and description centered
        float windowWidth = ImGui::GetWindowWidth();
        float titleWidth = ImGui::CalcTextSize("UAV Sensor Attack Interface").x;
        ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
        ImGui::Text("UAV Sensor Attack Interface");
        ImGui::Spacing();
        
        float offsetTextWidth = ImGui::CalcTextSize("Offsets (0.0001 ~= 36 feet, 0.00001 ~= 3.6 feet, 0.000001 ~= 4 inches)").x;
        ImGui::SetCursorPosX((windowWidth - offsetTextWidth) * 0.5f);
        ImGui::Text("Offsets (0.0001 ~= 36 feet, 0.00001 ~= 3.6 feet, 0.000001 ~= 4 inches)");
        ImGui::Separator();
        ImGui::Spacing();

        // Calculate column widths for a symmetrical layout
        float totalWidth = ImGui::GetContentRegionAvail().x;
        float columnWidth = totalWidth / 2.0f - 10.0f;
        float inputWidth = columnWidth * 0.7f;
        float buttonWidth = columnWidth * 0.25f;
        
        // We'll use the table feature directly, no need for BeginChild here
        float panelWidth = std::min(windowWidth - 40.0f, 1200.0f); // Max width with 20px padding on each side
        float colWidth = panelWidth / 2.0f - 10.0f;
        
        // Calculate heights to ensure vertical alignment
        float panelHeaderHeight = 30.0f;  // Approximate header height
        float panelContentHeight = 110.0f; // Fixed height for panel content to ensure alignment
        
        // Helper function to create a consistent sensor control panel
        auto createSensorPanel = [&](const char* title, float* values, const char* tooltip, 
                                  const char* buttonId, const std::string& topic) {
            // Always create the header (regular alignment, not centered)
            bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
            
            // Add tooltip for the panel
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
            
            if (isOpen) {
                // Use BeginChild for creating a child window to contain the controls
                ImGui::BeginChild(buttonId, ImVec2(colWidth - 10.0f, panelContentHeight), false, ImGuiWindowFlags_NoScrollbar);
                
                // Center the entire content area
                float panelWidth = ImGui::GetContentRegionAvail().x;
                
                // Center the description text
                float textWidth = ImGui::CalcTextSize(tooltip).x;
                float textWrappedWidth = std::min(panelWidth * 0.9f, 500.0f);
                ImGui::SetCursorPosX((panelWidth - textWrappedWidth) * 0.5f);
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textWrappedWidth);
                ImGui::TextUnformatted(tooltip);
                ImGui::PopTextWrapPos();
                ImGui::Spacing();
                ImGui::Spacing();
                
                // Center the input control
                float controlWidth = panelWidth * 0.7f;
                float btnWidth = panelWidth * 0.2f;
                
                // Create centered input row
                float inputRowWidth = controlWidth * 0.9f;
                float fieldWidth = inputRowWidth / 3.5f;
                
                // Center the input controls
                ImGui::SetCursorPosX((panelWidth - inputRowWidth) * 0.5f);
                
                // X input
                ImGui::Text("X:");
                ImGui::SameLine();
                ImGui::PushItemWidth(fieldWidth);
                ImGui::InputFloat("##x", &values[0], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("Y:");
                ImGui::SameLine();
                ImGui::PushItemWidth(fieldWidth);
                ImGui::InputFloat("##y", &values[1], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("Z:");
                ImGui::SameLine();
                ImGui::PushItemWidth(fieldWidth);
                ImGui::InputFloat("##z", &values[2], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                ImGui::Spacing();
                
                // Center the button
                ImGui::SetCursorPosX((panelWidth - btnWidth) * 0.5f);
                if (ImGui::Button(buttonId, ImVec2(btnWidth, 0))) {
                    std::vector<double> vals = {values[0], values[1], values[2]};
                    publishSensorAttack(topic, vals);
                }
                
                ImGui::EndChild();
                ImGui::Separator();
            } else {
                // Add a spacer if panel is closed to maintain vertical alignment
                ImGui::Dummy(ImVec2(colWidth - 20, 5.0f));
            }
        };
        
        // Helper function for sliders
        auto createSliderPanel = [&](const char* title, int* value, const char* tooltip, 
                                  const char* buttonId, const std::string& topic) {                         
            // Always create the header (regular alignment, not centered)
            bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
            
            // Add tooltip for the panel
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
            
            if (isOpen) {
                // Use BeginChild for creating a child window to contain the controls
                ImGui::BeginChild(buttonId, ImVec2(colWidth - 10.0f, panelContentHeight), false, ImGuiWindowFlags_NoScrollbar);
                
                // Center the entire content area
                float panelWidth = ImGui::GetContentRegionAvail().x;
                
                // Center the description text
                float textWidth = ImGui::CalcTextSize(tooltip).x;
                float textWrappedWidth = std::min(panelWidth * 0.9f, 500.0f);
                ImGui::SetCursorPosX((panelWidth - textWrappedWidth) * 0.5f);
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textWrappedWidth);
                ImGui::TextUnformatted(tooltip);
                ImGui::PopTextWrapPos();
                ImGui::Spacing();
                ImGui::Spacing();
                
                // Center the slider control
                float controlWidth = panelWidth * 0.7f;
                float btnWidth = panelWidth * 0.2f;
                
                // Center the label and slider
                ImGui::SetCursorPosX((panelWidth - controlWidth - 50) * 0.5f);
                ImGui::Text("Value:");
                ImGui::SameLine();
                
                ImGui::PushItemWidth(controlWidth);
                ImGui::SliderInt("##slider", value, 0, 10);
                ImGui::PopItemWidth();
                
                ImGui::Spacing();
                
                // Center the button
                ImGui::SetCursorPosX((panelWidth - btnWidth) * 0.5f);
                if (ImGui::Button(buttonId, ImVec2(btnWidth, 0))) {
                    publishInt(topic, *value);
                }
                
                ImGui::EndChild();
                ImGui::Separator();
            } else {
                // Add a spacer if panel is closed to maintain vertical alignment
                ImGui::Dummy(ImVec2(colWidth - 20, 5.0f));
            }
        };
        
        // Calculate optimal width for the table - use almost the full window width
        float tableWidth = ImGui::GetWindowWidth() * 0.95f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - tableWidth) * 0.5f);
        
        // Use ImGui's proper table feature with flags for auto-resizing
        if (ImGui::BeginTable("sensorTable", 2, 
                ImGuiTableFlags_Borders | 
                ImGuiTableFlags_SizingStretchSame | 
                ImGuiTableFlags_Resizable | 
                ImGuiTableFlags_ScrollY, 
                ImVec2(tableWidth, 0))) {
            
            // Set column widths to be equal
            for (int col = 0; col < 2; col++)
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            
            // Row 1
            ImGui::TableNextRow();
            
            // Cell 1,1: GPS
            ImGui::TableSetColumnIndex(0);
            createSensorPanel("GPS Offset", gps_rot, 
                           "Controls GPS position offset (x,y,z). Values around 0.0001 (~36 feet) are significant.",
                           "Apply##GPS", "~/attack/gps");
            
            // Cell 1,2: IMU
            ImGui::TableSetColumnIndex(1);
            createSensorPanel("IMU Offset", imu,
                           "Controls inertial measurement unit offset. Affects orientation readings.",
                           "Apply##IMU", "~/attack/imu");
            
            // Row 2
            ImGui::TableNextRow();
            
            // Cell 2,1: Lidar
            ImGui::TableSetColumnIndex(0);
            createSensorPanel("Lidar Offset", lidar,
                           "Controls Lidar sensor readings offset. Affects distance measurements.",
                           "Apply##Lidar", "~/attack/lidar");
            
            // Cell 2,2: Sonar
            ImGui::TableSetColumnIndex(1);
            createSensorPanel("Sonar Offset", sonar,
                           "Controls sonar/ultrasonic sensor offset. Affects height measurements.",
                           "Apply##Sonar", "~/attack/sonar");
            
            // Row 3
            ImGui::TableNextRow();
            
            // Cell 3,1: Barometer
            ImGui::TableSetColumnIndex(0);
            createSensorPanel("Barometer Offset", baro,
                           "Controls barometric pressure sensor offset. Affects altitude readings.",
                           "Apply##Baro", "~/attack/baro");
            
            // Cell 3,2: Magnetometer
            ImGui::TableSetColumnIndex(1);
            createSensorPanel("Magnetometer Offset", mag,
                           "Controls magnetometer sensor offset. Affects compass heading.",
                           "Apply##Mag", "~/attack/mag");
            
            // Row 4
            ImGui::TableNextRow();
            
            // Cell 4,1: Motor
            ImGui::TableSetColumnIndex(0);
            createSensorPanel("Motor Offset", motor,
                           "Controls motor speed offset values. Affects UAV thrust.",
                           "Apply##Motor", "~/attack/motor");
            
            // Cell 4,2: Wind
            ImGui::TableSetColumnIndex(1);
            
            // Wind control is special due to checkbox
            bool windIsOpen = ImGui::CollapsingHeader("Wind Attack", ImGuiTreeNodeFlags_DefaultOpen);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Controls wind simulation parameters. Affects UAV stability.");
            
            if (windIsOpen) {
                // Use BeginChild for the wind panel
                ImGui::BeginChild("windPanel", ImVec2(colWidth - 10.0f, panelContentHeight), false, ImGuiWindowFlags_NoScrollbar);
                
                // Center the entire content area
                float panelWidth = ImGui::GetContentRegionAvail().x;
                
                // Center the description text
                float textWrappedWidth = std::min(panelWidth * 0.9f, 500.0f);
                ImGui::SetCursorPosX((panelWidth - textWrappedWidth) * 0.5f);
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textWrappedWidth);
                ImGui::TextUnformatted("Controls wind simulation parameters. Affects UAV stability.");
                ImGui::PopTextWrapPos();
                ImGui::Spacing();
                ImGui::Spacing();
                
                float controlWidth = panelWidth * 0.7f;
                float btnWidth = panelWidth * 0.2f;
                
                // Center the input controls
                ImGui::SetCursorPosX((panelWidth - controlWidth) * 0.5f);
                
                ImGui::Text("X:");
                ImGui::SameLine();
                ImGui::PushItemWidth(controlWidth / 3 - 10);
                ImGui::InputFloat("##windx", &wind[0], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("Y:");
                ImGui::SameLine();
                ImGui::PushItemWidth(controlWidth / 3 - 10);
                ImGui::InputFloat("##windy", &wind[1], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("Z:");
                ImGui::SameLine();
                ImGui::PushItemWidth(controlWidth / 3 - 10);
                ImGui::InputFloat("##windz", &wind[2], 0.0001f, 0.001f, "%.6f");
                ImGui::PopItemWidth();
                
                // Center checkbox
                float checkboxWidth = ImGui::CalcTextSize("Enable Wind").x + 24.0f;
                ImGui::SetCursorPosX((panelWidth - checkboxWidth) * 0.5f);
                ImGui::Checkbox("Enable Wind", &wind_enabled);
                
                // Center the Apply button
                ImGui::SetCursorPosX((panelWidth - btnWidth) * 0.5f);
                if (ImGui::Button("Apply##Wind", ImVec2(btnWidth, 0))) {
                    std::vector<double> values = {wind[0], wind[1], wind[2]};
                    publishWindAttack(values);
                }
                
                ImGui::EndChild();
                ImGui::Separator();
            } else {
                // Add a spacer if panel is closed to maintain vertical alignment
                ImGui::Dummy(ImVec2(colWidth - 20, 5.0f));
            }
            
            // Row 5 - Add Camera and Stream controls
            ImGui::TableNextRow();
            
            // Cell 5,1: Camera Control
            ImGui::TableSetColumnIndex(0);
            createSliderPanel("Camera Control", &camera,
                         "Controls camera settings. Higher values increase exposure.",
                         "Apply##Camera", "~/video_stream");
                
            // Cell 5,2: Stream Control
            ImGui::TableSetColumnIndex(1);
            createSliderPanel("Stream Control", &stream,
                         "Controls data stream parameters. Higher values increase bandwidth.",
                         "Apply##Stream", "~/attack/stream");
            
            ImGui::EndTable();
        }
        
        // Ensure no EndChild() is needed here
        
        // Center the action buttons and make them responsive
        // Using the previously defined windowWidth
        float buttonAreaWidth = std::min(windowWidth * 0.6f, 800.0f);
        float halfButtonWidth = (buttonAreaWidth / 2.0f) - 10.0f;
        
        // Container to center the buttons
        ImGui::SetCursorPosX((windowWidth - buttonAreaWidth) * 0.5f);
        ImGui::BeginGroup();
        
        // Reset All button with tooltip
        if (ImGui::Button("Reset All Sensors", ImVec2(halfButtonWidth, 50))) {
            std::vector<double> zeroes = {0.0, 0.0, 0.0};
            
            // Only send reset messages for non-zero values to prevent freezing
            bool hasNonZeroGPS = false;
            bool hasNonZeroIMU = false;
            bool hasNonZeroLidar = false;
            bool hasNonZeroSonar = false;
            bool hasNonZeroBaro = false;
            bool hasNonZeroMag = false;
            bool hasNonZeroMotor = false;
            bool hasNonZeroWind = false;
            
            // Check for non-zero values
            for (int i = 0; i < 3; i++) {
                if (gps_rot[i] != 0.0f) hasNonZeroGPS = true;
                if (imu[i] != 0.0f) hasNonZeroIMU = true;
                if (lidar[i] != 0.0f) hasNonZeroLidar = true;
                if (sonar[i] != 0.0f) hasNonZeroSonar = true;
                if (baro[i] != 0.0f) hasNonZeroBaro = true;
                if (mag[i] != 0.0f) hasNonZeroMag = true;
                if (motor[i] != 0.0f) hasNonZeroMotor = true;
                if (wind[i] != 0.0f) hasNonZeroWind = true;
            }
            
            // Only publish reset for non-zero values
            if (hasNonZeroGPS) publishSensorAttack("~/attack/gps", zeroes);
            if (hasNonZeroIMU) publishSensorAttack("~/attack/imu", zeroes);
            if (hasNonZeroLidar) publishSensorAttack("~/attack/lidar", zeroes);
            if (hasNonZeroSonar) publishSensorAttack("~/attack/sonar", zeroes);
            if (hasNonZeroBaro) publishSensorAttack("~/attack/baro", zeroes);
            if (hasNonZeroMag) publishSensorAttack("~/attack/mag", zeroes);
            if (hasNonZeroMotor) publishSensorAttack("~/attack/motor", zeroes);
            if (hasNonZeroWind) publishWindAttack(zeroes);
            
            // Only reset non-zero UI controls
            if (camera != 0) publishInt("~/video_stream", 0);
            if (stream != 0) publishInt("~/attack/stream", 0);
            
            // Reset all UI values to 0
            for (int i = 0; i < 3; i++) {
                gps_rot[i] = 0.0f;
                imu[i] = 0.0f;
                lidar[i] = 0.0f;
                sonar[i] = 0.0f;
                baro[i] = 0.0f;
                mag[i] = 0.0f;
                motor[i] = 0.0f;
                wind[i] = 0.0f;
            }
            camera = 0;
            stream = 0;
            wind_enabled = false;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Resets all sensor offsets to zero and publishes zero values to all attack channels");
        
        ImGui::SameLine(0, 20.0f);
        
        // Exit button
        if (ImGui::Button("Exit Application", ImVec2(halfButtonWidth, 50))) {
            break;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Close the application");
            
        ImGui::EndGroup();
        
        // No status bar text at the bottom

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    gazebo::transport::fini();
    return 0;
}