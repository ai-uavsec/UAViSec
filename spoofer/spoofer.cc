#include <boost/shared_ptr.hpp>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include <string>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

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

// 0.0001 = 36 feet
// 0.00001 = 3.6 feet
// 0.000001 = 4 inches
int main(int _argc, char **_argv)
{

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

  // Initialize Gazebo
  gazebo::client::setup();
  gazebo::transport::NodePtr node(new gazebo::transport::Node());
  node->Init();

  // gazebo::msgs::Vector3d data;

  if (vm.count("abs")) {
	  /*std::vector<double> offsets = vm["abs"].as<std::vector<double>>();
	  gazebo::msgs::Vector3d gps_data;
	  fill_data(&gps_data, offsets);
	  gazebo::transport::PublisherPtr gps_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/gps", 10);
	  gps_pub->WaitForConnection();
	  gps_pub->Publish(gps_data);*/
  }

  if (vm.count("rot")) {
	  std::vector<double> offsets = vm["rot"].as<std::vector<double>>();
	  gazebo::msgs::Vector3d rot_data;
	  fill_data(&rot_data, offsets);
	  gazebo::transport::PublisherPtr rot_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/gps", 10);
	  rot_pub->WaitForConnection();
	  rot_pub->Publish(rot_data);
  }

  if (vm.count("imu")) {
    std::cout << "IMU";
    std::vector<double> offsets = vm["imu"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/imu",10);

    gazebo::msgs::Vector3d data;
    fill_data(&data, offsets);

    gps_pub->WaitForConnection();
    gps_pub->Publish(data);
  }

  if (vm.count("lidar")) {
    std::vector<double> offsets = vm["lidar"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr lidar_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/lidar",10);

    gazebo::msgs::Vector3d lidar_data;
    fill_data(&lidar_data, offsets);

      // gazebo::msgs::Any data;
      // data=gazebo::msgs::ConvertAny(atof(_argv[2]));
      
  
    lidar_pub->WaitForConnection();
    lidar_pub->Publish(lidar_data);
  }

  if (vm.count("sonar")) {
    std::vector<double> offsets = vm["sonar"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/sonar",10);

    gazebo::msgs::Vector3d data;
    fill_data(&data, offsets);
  
    gps_pub->WaitForConnection();
    gps_pub->Publish(data);
  }

  if (vm.count("baro")) {
    std::vector<double> offsets = vm["baro"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/baro",10);

    gazebo::msgs::Vector3d baro_data;
    fill_data(&baro_data, offsets);
    gps_pub->WaitForConnection();
    gps_pub->Publish(baro_data);
  }

  if (vm.count("wind")) {
    std::vector<double> offsets = vm["wind"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr wind_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/wind", 10);
    gazebo::transport::PublisherPtr wind_enable_pub = node->Advertise<gazebo::msgs::Int>("~/attack/wind/enabled", 10);

    gazebo::msgs::Vector3d data;
    fill_data(&data, offsets);

    gazebo::msgs::Int enable_data;
    enable_data.set_data(0);

    wind_pub->WaitForConnection();
    wind_pub->Publish(data);

    wind_enable_pub->WaitForConnection();
    wind_enable_pub->Publish(enable_data);
    

  }

  if (vm.count("mag")) {
    std::vector<double> offsets = vm["mag"].as<std::vector<double>>();
    gazebo::transport::PublisherPtr mag_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/mag", 10);
    

    gazebo::msgs::Vector3d mag_data;
    fill_data(&mag_data, offsets);
    std::cout << "Wat\n";
    mag_pub->WaitForConnection();
    mag_pub->Publish(mag_data);
    std::cout << "MAG\n";
  }


  // gazebo::msgs::Any data1;
  const char *arg = _argv[1];
    /*if (std::strcmp(arg, "-abs") == 0)
    {
      // Construct PublisherPtr with topic "~/attack/gps"
      gazebo::transport::PublisherPtr gps_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/gps", 10);

      // Create Vector3d
      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(atof(_argv[4]));

      // Publish Message
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);
    }
    else if (std::strcmp(arg, "-rot") == 0)
    {
      // Construct PublisherPtr with topic "~/attack/gps_rot"
      gazebo::transport::PublisherPtr gps_pub = node->Advertise<gazebo::msgs::Vector3d>("~/attack/gps_rot", 10);

      // Create Vector3d
      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(0.0);

      // Publish Message
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout << "Spoofing Arguments: -rot \n"  ;
    } */  
    if (std::strcmp(arg,"-imu")==0)
    {
      // gazebo::transport::PublisherPtr gps_pub1=node->Advertise<gazebo::msgs::Any>("~/attack/imu",10);
      gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/imu",10);

      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(atof(_argv[4]));


      // gazebo::msgs::Any data;
      // data=gazebo::msgs::ConvertAny(atof(_argv[2]));
      
  
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout<< "Influence imu Data: -imu \n"  ;
    }
    else if (std::strcmp(arg,"-lidar")==0)
    {
      // gazebo::transport::PublisherPtr gps_pub1=node->Advertise<gazebo::msgs::Any>("~/attack/imu",10);
      gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/lidar",10);

      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(0.0);


      // gazebo::msgs::Any data;
      // data=gazebo::msgs::ConvertAny(atof(_argv[2]));
      
  
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout<< "Influence lidar Data: -lidar \n"  ;
    }
    else if (std::strcmp(arg,"-sonar")==0)
    {
      gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/sonar",10);

      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(0.0);

      
  
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout<< "Influence Sonar Data: -sonar \n"  ;
    }
    else if (std::strcmp(arg,"-baro")==0)
    {
      gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/baro",10);

      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(0.0);


      // gazebo::msgs::Any data;
      // data=gazebo::msgs::ConvertAny(atof(_argv[2]));
      
  
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout<< "Influence baro Data:  \n"  ;
    }
    else if (std::strcmp(arg,"-mag")==0)
    {
      gazebo::transport::PublisherPtr gps_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/mag",20);

      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(atof(_argv[4]));


      // gazebo::msgs::Any data;
      // data=gazebo::msgs::ConvertAny(atof(_argv[2]));
      
  
      gps_pub->WaitForConnection();
      gps_pub->Publish(data);

      std::cout<< "Influence Mag Data: -mag \n"  ;
    }

    else if (std::strcmp(arg,"-camera")==0)
    {
      gazebo::transport::PublisherPtr camera_pub=node->Advertise<gazebo::msgs::Int>("~/video_stream",10);

      gazebo::msgs::Int m;
    // m.set_double_value(atof(_argv[2]));
      m.set_data(atof(_argv[2]));
      camera_pub->WaitForConnection();
      camera_pub->Publish(m);
      // camera_pub->ConnectWorldUpdateBegin
    }

    
    else if (std::strcmp(arg,"-stream")==0)
    {
      gazebo::transport::PublisherPtr stream_pub=node->Advertise<gazebo::msgs::Int>("~/attack/stream",10);
      gazebo::msgs::Int m;
      m.set_data(atof(_argv[2]));
      stream_pub->WaitForConnection();
      stream_pub->Publish(m);
    }
    else if (std::strcmp(arg,"-motor")==0){
      gazebo::transport::PublisherPtr motor_pub=node->Advertise<gazebo::msgs::Vector3d>("~/attack/motor",10);
      gazebo::msgs::Vector3d data;
      data.set_x(atof(_argv[2]));
      data.set_y(atof(_argv[3]));
      data.set_z(atof(_argv[4]));
      motor_pub->WaitForConnection();
      motor_pub->Publish(data); 
    } 

  gazebo::transport::fini();
  return 0;
  
}
