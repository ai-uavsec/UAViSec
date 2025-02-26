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

        ImGui::Begin("Sensor Attack Interface");
        
        ImGui::Text("Offsets (0.0001 ~= 36 feet, 0.00001 ~= 3.6 feet, 0.000001 ~= 4 inches)");
        ImGui::Separator();

        if (ImGui::CollapsingHeader("GPS Rotational Offset")) {
            ImGui::InputFloat3("GPS Rot (x,y,z)", gps_rot);
            if (ImGui::Button("Apply GPS Rot")) {
                std::vector<double> values = {gps_rot[0], gps_rot[1], gps_rot[2]};
                publishSensorAttack("~/attack/gps", values);
            }
        }

        if (ImGui::CollapsingHeader("IMU Offset")) {
            ImGui::InputFloat3("IMU (x,y,z)", imu);
            if (ImGui::Button("Apply IMU")) {
                std::vector<double> values = {imu[0], imu[1], imu[2]};
                publishSensorAttack("~/attack/imu", values);
            }
        }

        if (ImGui::CollapsingHeader("Lidar Offset")) {
            ImGui::InputFloat3("Lidar (x,y,z)", lidar);
            if (ImGui::Button("Apply Lidar")) {
                std::vector<double> values = {lidar[0], lidar[1], lidar[2]};
                publishSensorAttack("~/attack/lidar", values);
            }
        }

        if (ImGui::CollapsingHeader("Sonar Offset")) {
            ImGui::InputFloat3("Sonar (x,y,z)", sonar);
            if (ImGui::Button("Apply Sonar")) {
                std::vector<double> values = {sonar[0], sonar[1], sonar[2]};
                publishSensorAttack("~/attack/sonar", values);
            }
        }

        if (ImGui::CollapsingHeader("Barometer Offset")) {
            ImGui::InputFloat3("Barometer (x,y,z)", baro);
            if (ImGui::Button("Apply Barometer")) {
                std::vector<double> values = {baro[0], baro[1], baro[2]};
                publishSensorAttack("~/attack/baro", values);
            }
        }

        if (ImGui::CollapsingHeader("Magnetometer Offset")) {
            ImGui::InputFloat3("Magnetometer (x,y,z)", mag);
            if (ImGui::Button("Apply Magnetometer")) {
                std::vector<double> values = {mag[0], mag[1], mag[2]};
                publishSensorAttack("~/attack/mag", values);
            }
        }

        if (ImGui::CollapsingHeader("Motor Offset")) {
            ImGui::InputFloat3("Motor (x,y,z)", motor);
            if (ImGui::Button("Apply Motor")) {
                std::vector<double> values = {motor[0], motor[1], motor[2]};
                publishSensorAttack("~/attack/motor", values);
            }
        }

        if (ImGui::CollapsingHeader("Wind Attack")) {
            ImGui::InputFloat3("Wind (x,y,z)", wind);
            ImGui::Checkbox("Enable Wind", &wind_enabled);
            if (ImGui::Button("Apply Wind")) {
                std::vector<double> values = {wind[0], wind[1], wind[2]};
                publishWindAttack(values);
            }
        }

        if (ImGui::CollapsingHeader("Camera and Stream")) {
            ImGui::SliderInt("Camera Value", &camera, 0, 10);
            if (ImGui::Button("Apply Camera")) {
                publishInt("~/video_stream", camera);
            }
            
            ImGui::SliderInt("Stream Value", &stream, 0, 10);
            if (ImGui::Button("Apply Stream")) {
                publishInt("~/attack/stream", stream);
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Exit")) {
            break;
        }

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