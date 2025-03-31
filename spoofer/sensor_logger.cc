#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <filesystem>
#include <chrono>
#include <thread>

#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include <boost/shared_ptr.hpp>
#include "sensors/SITLGps.pb.h"
#include "sensors/Groundtruth.pb.h"
#include "sensors/Imu.pb.h"
#include "sensors/Pressure.pb.h"
#include "sensors/MagneticField.pb.h"

std::string file_prefix;

void write_data_to_file(std::string fileName, std::string data)
{
	std::ofstream csv("../logs/" + file_prefix + "/" + fileName, std::ofstream::out | std::ofstream::app);
	csv << data << std::endl;
	csv.close();
}

void read_field_value(const google::protobuf::Message &_msg, const google::protobuf::FieldDescriptor *&field, std::stringstream &ss)
{
	const google::protobuf::Reflection *reflection = _msg.GetReflection();
	int cpp_Type = field->cpp_type();

	if (reflection == nullptr)
	{
		return;
	}

	ss << std::fixed;

	if (!field->is_repeated())
	{
		if (cpp_Type == 1)
		{
			ss << reflection->GetInt32(_msg, field);
		}
		else if (cpp_Type == 2)
		{
			ss << reflection->GetInt64(_msg, field);
		}
		else if (cpp_Type == 3)
		{
			ss << reflection->GetUInt32(_msg, field);
		}
		else if (cpp_Type == 4)
		{
			ss << reflection->GetUInt64(_msg, field);
		}
		else if (cpp_Type == 5)
		{
			ss << reflection->GetDouble(_msg, field);
		}
		else if (cpp_Type == 6)
		{
			ss << reflection->GetFloat(_msg, field);
		}
		else if (cpp_Type == 7)
		{
			ss << reflection->GetBool(_msg, field);
		}
		else if (cpp_Type == 8)
		{
			ss << reflection->GetEnum(_msg, field);
		}
		else if (cpp_Type == 9)
		{
			ss << reflection->GetString(_msg, field);
		}
		else if (cpp_Type == 10)
		{
			const google::protobuf::Descriptor *messageDescriptor = reflection->GetMessage(_msg, field).GetDescriptor();
			int messageFieldCount = messageFieldCount = messageDescriptor->field_count();

			for (int i = 0; i < messageFieldCount; i++)
			{
				const google::protobuf::FieldDescriptor *messageFieldDescriptor = messageDescriptor->field(i);
				read_field_value(reflection->GetMessage(_msg, field), messageFieldDescriptor, ss);
			}
			return;
		}

		ss << ",";
	}
	else
	{
		int fieldSize = reflection->FieldSize(_msg, field);
		for (int i = 0; i < fieldSize; i++)
		{
			if (cpp_Type == 1)
			{
				ss << reflection->GetRepeatedInt32(_msg, field, i);
			}
			else if (cpp_Type == 2)
			{
				ss << reflection->GetRepeatedInt64(_msg, field, i);
			}
			else if (cpp_Type == 3)
			{
				ss << reflection->GetRepeatedUInt32(_msg, field, i);
			}
			else if (cpp_Type == 4)
			{
				ss << reflection->GetRepeatedUInt64(_msg, field, i);
			}
			else if (cpp_Type == 5)
			{
				ss << reflection->GetRepeatedDouble(_msg, field, i);
			}
			else if (cpp_Type == 6)
			{
				ss << reflection->GetRepeatedFloat(_msg, field, i);
			}
			else if (cpp_Type == 7)
			{
				ss << reflection->GetRepeatedBool(_msg, field, i);
			}
			else if (cpp_Type == 8)
			{
				ss << reflection->GetRepeatedEnum(_msg, field, i);
			}
			else if (cpp_Type == 9)
			{
				ss << reflection->GetRepeatedString(_msg, field, i);
			}
			else if (cpp_Type == 10)
			{
				const google::protobuf::Descriptor *messageDescriptor = reflection->GetRepeatedMessage(_msg, field, i).GetDescriptor();
				int messageFieldCount = messageFieldCount = messageDescriptor->field_count();

				for (int i = 0; i < messageFieldCount; i++)
				{
					const google::protobuf::FieldDescriptor *messageFieldDescriptor = messageDescriptor->field(i);
					read_field_value(reflection->GetRepeatedMessage(_msg, field, i), messageFieldDescriptor, ss);
				}
				return;
			}

			ss << ",";
		}
	}
}

template <typename T>
void base_log_cb(const auto &_msg, std::string fileName)
{
	T log_msg;
	log_msg.ParseFromString(_msg);
	const google::protobuf::Descriptor *descriptor = log_msg.GetDescriptor();
	int count = descriptor->field_count();

	std::stringstream ss;
	for (int i = 0; i < count; i++)
	{
		auto *field = descriptor->field(i);
		read_field_value(log_msg, field, ss);
	}

	std::string data = ss.str();
	data = data.substr(0, data.size() - 1);
	write_data_to_file(fileName, data);
}

int main(int _argc, char **_argv)
{
	// File Name
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "log_%d_%m_%Y_%H_%M_%S", timeinfo);
	std::string str(buffer);
	file_prefix = str;

	namespace fs = std::filesystem;
	fs::create_directories("../logs/" + file_prefix);
	// Write CSV Headers
	write_data_to_file("raw_gps.csv", "time_usec,time_utc_usec,latitude_deg,longitude_deg,altitude,eph,epv,velocity,velocity_east,velocity_north,velocity_up,attack_enabled");
	write_data_to_file("groundtruth.csv", "time_usec,latitude_rad,longitude_rad,altitude,velocity_east,velocity_north,velocity_up,attitude_q_w,attitude_q_x,attitude_q_y,attitude_q_z");
	write_data_to_file("imu.csv", "orientation_x,orientation_y,orientation_z,orientation_w,orientation_covariance_1,orientation_covariance_2,orientation_covariance_3,orientation_covariance_4,orientation_covariance_5,orientation_covariance_6,orientation_covariance_7,orientation_covariance_8,orientation_covariance_9,angular_velocity_x,angular_velocity_y,angular_velocity_z,angular_velocity_covariance_1,angular_velocity_covariance_2,angular_velocity_covariance_3,angular_velocity_covariance_4,angular_velocity_covariance_5,angular_velocity_covariance_6,angular_velocity_covariance_7,angular_velocity_covariance_8,angular_velocity_covariance_9,linear_acceleration_x,linear_acceleration_y,linear_acceleration_z,linear_acceleration_covariance_1,linear_acceleration_covariance_2,linear_acceleration_covariance_3,linear_acceleration_covariance_4,linear_acceleration_covariance_5,linear_acceleration_covariance_6,linear_acceleration_covariance_7,linear_acceleration_covariance_8,linear_acceleration_covariance_9,time_usec,seq");
	write_data_to_file("barometer.csv", "time_usec,temperature,absolute_pressure,pressure_altitude");
	write_data_to_file("magnetometer.csv", "time_usec,magnetic_field_x,magnetic_field_y,magnetic_field_z,magnetic_field_covariance_1,magnetic_field_covariance_2,magnetic_field_covariance_3,magnetic_field_covariance_4,magnetic_field_covariance_5,magnetic_field_covariance_6,magnetic_field_covariance_7,magnetic_field_covariance_8,magnetic_field_covariance_9");

	// Gazebo Initialization
	gazebo::client::setup();
	gazebo::transport::NodePtr node(new gazebo::transport::Node());
	node->Init();

	// Sensor Subscriptions
	gazebo::transport::SubscriberPtr raw_gps_sub = node->Subscribe("~/log/gps", [](auto &m) { base_log_cb<sensor_msgs::msgs::SITLGps>(m, "raw_gps.csv"); });
	gazebo::transport::SubscriberPtr groundtruth_sub = node->Subscribe("~/log/groundtruth", [](auto &m) { base_log_cb<sensor_msgs::msgs::Groundtruth>(m, "groundtruth.csv"); });
	gazebo::transport::SubscriberPtr imu_sub = node->Subscribe("~/log/imu", [](auto &m) { base_log_cb<sensor_msgs::msgs::Imu>(m, "imu.csv"); });
	gazebo::transport::SubscriberPtr barometer_sub = node->Subscribe("~/log/baro", [](auto &m) { base_log_cb<sensor_msgs::msgs::Pressure>(m, "barometer.csv"); });
	gazebo::transport::SubscriberPtr magnetometer_sub = node->Subscribe("~/log/mag", [](auto &m) { base_log_cb<sensor_msgs::msgs::MagneticField>(m, "magnetometer.csv"); });
	std::cout << "Press any key to stop logging..." << std::endl;
	std::cin.ignore();
	std::cout << "Logging completed" << std::endl;
	gazebo::client::shutdown();
	return 0;
}