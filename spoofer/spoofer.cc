#include <boost/shared_ptr.hpp>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include <string>
#include <iostream>

// 0.0001 = 36 feet
// 0.00001 = 3.6 feet
// 0.000001 = 4 inches
int main(int _argc, char **_argv)
{
  // Initialize Gazebo
  gazebo::client::setup();
  gazebo::transport::NodePtr node(new gazebo::transport::Node());
  node->Init();

  gazebo::msgs::Vector3d data;
  // gazebo::msgs::Any data1;
  const char *arg = _argv[1];
    if (std::strcmp(arg, "-abs") == 0)
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
    }   
    else if (std::strcmp(arg,"-imu")==0)
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