/*
 * Copyright (C) 2012 Open Source Robotics Foundation
 * Copyright (C) 2017-2018 PX4 Pro Development Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
/**
 * @brief GPS Plugin
 *
 * This plugin publishes GPS and Groundtruth data to be used and propagated
 *
 * @author Amy Wagoner <arwagoner@gmail.com>
 * @author Nuno Marques <nuno.marques@dronesolutions.io>
 */

#include <gazebo_gps_plugin.h>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace gazebo {
GZ_REGISTER_SENSOR_PLUGIN(GpsPlugin)

GpsPlugin::GpsPlugin() : SensorPlugin()
{ }

GpsPlugin::~GpsPlugin()
{
  if (updateSensorConnection_)
    updateSensorConnection_->~Connection();
  parentSensor_.reset();
}

void GpsPlugin::Load(sensors::SensorPtr _parent, sdf::ElementPtr _sdf)
{
  // Get then name of the parent sensor
  parentSensor_ = std::dynamic_pointer_cast<sensors::GpsSensor>(_parent);

  if (!parentSensor_)
    gzthrow("GpsPlugin requires a GPS Sensor as its parent");

  // Get the root model name
  const string scopedName = _parent->ParentName();
  vector<std::string> names_splitted;
  boost::split(names_splitted, scopedName, boost::is_any_of("::"));
  names_splitted.erase(std::remove_if(begin(names_splitted), end(names_splitted),
                            [](const string& name)
                            { return name.size() == 0; }), end(names_splitted));

  // get the root model name
  const string rootModelName = names_splitted.front();

  // store the model name
  model_name_ = names_splitted.at(0);

  // the second to the last name is the model name
  const string parentSensorModelName = names_splitted.rbegin()[1];

  // get gps topic name
  if(_sdf->HasElement("topic")) {
    gps_topic_ = _sdf->GetElement("topic")->Get<std::string>();
  } else {
    // if not set by parameter, get the topic name from the model name
    gps_topic_ = parentSensorModelName;
    gzwarn << "[gazebo_gps_plugin]: " + rootModelName + "::" + parentSensorModelName +
      " using gps topic \"" << parentSensorModelName << "\"\n";
  }

  // Store the pointer to the world.
  world_ = physics::get_world(parentSensor_->WorldName());

  #if GAZEBO_MAJOR_VERSION >= 9
    last_time_ = world_->SimTime();
    last_gps_time_ = world_->SimTime();
    start_time_ = world_->StartTime();
  #else
    last_time_ = world_->GetSimTime();
    last_gps_time_ = world_->GetSimTime();
    start_time_ = world_->GetStartTime();
  #endif

  // Use environment variables if set for home position.
  const char *env_lat = std::getenv("PX4_HOME_LAT");
  const char *env_lon = std::getenv("PX4_HOME_LON");
  const char *env_alt = std::getenv("PX4_HOME_ALT");

  // Get noise param
  if (_sdf->HasElement("gpsNoise")) {
    getSdfParam<bool>(_sdf, "gpsNoise", gps_noise_, gps_noise_);
  } else {
    gps_noise_ = false;
  }

  const bool world_has_origin = checkWorldHomePosition(world_, world_latitude_, world_longitude_, world_altitude_);

  if (env_lat) {
    lat_home_ = std::stod(env_lat) * M_PI / 180.0;
    gzmsg << "[gazebo_gps_plugin] Home latitude is set to " << std::stod(env_lat) << ".\n";
  } else if (world_has_origin) {
    lat_home_ = world_latitude_;
    gzmsg << "[gazebo_gps_plugin] Home latitude is set to " << lat_home_ << ".\n";
  } else if(_sdf->HasElement("homeLatitude")) {
    double latitude;
    getSdfParam<double>(_sdf, "homeLatitude", latitude, lat_home_);
    lat_home_ = latitude * M_PI / 180.0;
  }

  if (env_lon) {
    lon_home_ = std::stod(env_lon) * M_PI / 180.0;
    gzmsg << "[gazebo_gps_plugin] Home longitude is set to " << std::stod(env_lon) << ".\n";
  } else if (world_has_origin) {
    lon_home_ = world_longitude_;
    gzmsg << "[gazebo_gps_plugin] Home longitude is set to " << lon_home_ << ".\n";
  } else if(_sdf->HasElement("homeLongitude")) {
    double longitude;
    getSdfParam<double>(_sdf, "homeLongitude", longitude, lon_home_);
    lon_home_ = longitude * M_PI / 180.0;
  }

  if (env_alt) {
    alt_home_ = std::stod(env_alt);
    gzmsg << "[gazebo_gps_plugin] Home altitude is set to " << alt_home_ << ".\n";
  } else if (world_has_origin) {
    alt_home_ = world_altitude_;
    gzmsg << "[gazebo_gps_plugin] Home altitude is set to " << alt_home_ << ".\n";
  } else if(_sdf->HasElement("homeAltitude")) {
    getSdfParam<double>(_sdf, "homeAltitude", alt_home_, alt_home_);
  }

  // get random walk in XY plane
  if (_sdf->HasElement("gpsXYRandomWalk")) {
    getSdfParam<double>(_sdf, "gpsXYRandomWalk", gps_xy_random_walk_, kDefaultGpsXYRandomWalk);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default random walk in XY plane: "
           << kDefaultGpsXYRandomWalk << "\n";
  }

  // get random walk in Z
  if (_sdf->HasElement("gpsZRandomWalk")) {
    getSdfParam<double>(_sdf, "gpsZRandomWalk", gps_z_random_walk_, kDefaultGpsZRandomWalk);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default random walk in Z: "
           << kDefaultGpsZRandomWalk << "\n";
  }

  // get position noise density in XY plane
  if (_sdf->HasElement("gpsXYNoiseDensity")) {
    getSdfParam<double>(_sdf, "gpsXYNoiseDensity", gps_xy_noise_density_, kDefaultGpsXYNoiseDensity);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default position noise density in XY plane: "
           << kDefaultGpsXYNoiseDensity << "\n";
  }

  // get position noise density in Z
  if (_sdf->HasElement("gpsZNoiseDensity")) {
    getSdfParam<double>(_sdf, "gpsZNoiseDensity", gps_z_noise_density_, kDefaultGpsZNoiseDensity);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default position noise density in Z: "
           << kDefaultGpsZNoiseDensity << "\n";
  }

  // get velocity noise density in XY plane
  if (_sdf->HasElement("gpsVXYNoiseDensity")) {
    getSdfParam<double>(_sdf, "gpsVXYNoiseDensity", gps_vxy_noise_density_, kDefaultGpsVXYNoiseDensity);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default velocity noise density in XY plane: "
           << kDefaultGpsVXYNoiseDensity << "\n";
  }

  // get velocity noise density in Z
  if (_sdf->HasElement("gpsVZNoiseDensity")) {
    getSdfParam<double>(_sdf, "gpsVZNoiseDensity", gps_vz_noise_density_, kDefaultGpsVZNoiseDensity);
  } else {
    gzwarn << "[gazebo_gps_plugin] Using default velocity noise density in Z: "
           << kDefaultGpsVZNoiseDensity << "\n";
  }

  namespace_.clear();
  if (_sdf->HasElement("robotNamespace")) {
    namespace_ = _sdf->GetElement("robotNamespace")->Get<std::string>();
  } else {
    gzerr << "[gazebo_gps_plugin] Please specify a robotNamespace.\n";
  }

  // get update rate
  if (_sdf->HasElement("update_rate")) {
    getSdfParam<double>(_sdf, "update_rate", update_rate_, kDefaultUpdateRate);
  } else {
    update_rate_ = kDefaultUpdateRate;
    gzwarn << "[gazebo_gps_plugin] Using default update rate of "
           << kDefaultUpdateRate << "hz \n";
  }
  // parentSensor_->SetUpdateRate(update_rate_);
  parentSensor_->SetUpdateRate(500.0);

  node_handle_ = transport::NodePtr(new transport::Node());
  node_handle_->Init(namespace_);

  parentSensor_->SetActive(false);
  updateSensorConnection_ = parentSensor_->ConnectUpdated(boost::bind(&GpsPlugin::OnSensorUpdate, this));
  parentSensor_->SetActive(true);

  // Listen to the update event. This event is broadcast every simulation iteration.
  updateWorldConnection_ = event::Events::ConnectWorldUpdateBegin(
      boost::bind(&GpsPlugin::OnWorldUpdate, this, _1));

  gravity_W_ = world_->Gravity();

  info_pub_=node_handle_->Advertise<gazebo::msgs::Vector3d>("~/"+rootModelName+"/gps",10);
  gps_pub_ = node_handle_->Advertise<sensor_msgs::msgs::SITLGps>("~/" + rootModelName + "/link/" + gps_topic_, 10);
  gps_offset_list_ = node_handle_->Subscribe("~/attack/gps", &GpsPlugin::OnOffsetUpdate, this);
  gps_rot_offset_list_ = node_handle_->Subscribe("~/attack/gps_rot", &GpsPlugin::OnRotationOffsetUpdate, this);
  gps_data_pub_ = node_handle_->Advertise<sensor_msgs::msgs::SITLGps>("~/log/gps", 10);
}

void GpsPlugin::OnWorldUpdate(const common::UpdateInfo& /*_info*/)
{
  // Store the pointer to the model.
  if (model_ == NULL)
#if GAZEBO_MAJOR_VERSION >= 9
    model_ = world_->ModelByName(model_name_);
#else
    model_ = world_->GetModel(model_name_);
#endif

  common::Time current_time;

#if GAZEBO_MAJOR_VERSION >= 9
  current_time = world_->SimTime();
#else
  current_time = world_->GetSimTime();
#endif
  double dt = (current_time - last_time_).Double();

#if GAZEBO_MAJOR_VERSION >= 9
  ignition::math::Pose3d T_W_I = model_->WorldPose();
#else
  ignition::math::Pose3d T_W_I = ignitionFromGazeboMath(model_->GetWorldPose());
#endif
  // Use the model world position for GPS
  ignition::math::Vector3d& pos_W_I = T_W_I.Pos();
  ignition::math::Quaterniond& att_W_I = T_W_I.Rot();

  // Use the models' world position for GPS velocity.
#if GAZEBO_MAJOR_VERSION >= 9
  ignition::math::Vector3d velocity_current_W = model_->WorldLinearVel();
#else
  ignition::math::Vector3d velocity_current_W = ignitionFromGazeboMath(model_->GetWorldLinearVel());
#endif

  ignition::math::Vector3d velocity_current_W_xy = velocity_current_W;
  velocity_current_W_xy.Z() = 0;

  // update noise parameters if gps_noise_ is set
  if (gps_noise_) {
    noise_gps_pos_.X() = gps_xy_noise_density_ * sqrt(dt) * randn_(rand_);
    noise_gps_pos_.Y() = gps_xy_noise_density_ * sqrt(dt) * randn_(rand_);
    noise_gps_pos_.Z() = gps_z_noise_density_ * sqrt(dt) * randn_(rand_);
    noise_gps_vel_.X() = gps_vxy_noise_density_ * sqrt(dt) * randn_(rand_);
    noise_gps_vel_.Y() = gps_vxy_noise_density_ * sqrt(dt) * randn_(rand_);
    noise_gps_vel_.Z() = gps_vz_noise_density_ * sqrt(dt) * randn_(rand_);
    random_walk_gps_.X() = gps_xy_random_walk_ * sqrt(dt) * randn_(rand_);
    random_walk_gps_.Y() = gps_xy_random_walk_ * sqrt(dt) * randn_(rand_);
    random_walk_gps_.Z() = gps_z_random_walk_ * sqrt(dt) * randn_(rand_);
  }
  else {
    noise_gps_pos_.X() = 0.0;
    noise_gps_pos_.Y() = 0.0;
    noise_gps_pos_.Z() = 0.0;
    noise_gps_vel_.X() = 0.0;
    noise_gps_vel_.Y() = 0.0;
    noise_gps_vel_.Z() = 0.0;
    random_walk_gps_.X() = 0.0;
    random_walk_gps_.Y() = 0.0;
    random_walk_gps_.Z() = 0.0;
  }

  // gps bias integration
  gps_bias_.X() += random_walk_gps_.X() * dt - gps_bias_.X() / gps_corellation_time_;
  gps_bias_.Y() += random_walk_gps_.Y() * dt - gps_bias_.Y() / gps_corellation_time_;
  gps_bias_.Z() += random_walk_gps_.Z() * dt - gps_bias_.Z() / gps_corellation_time_;

  // reproject position with noise into geographic coordinates
  auto pos_with_noise = pos_W_I + noise_gps_pos_ + gps_bias_;
  auto latlon = reproject(pos_with_noise, lat_home_, lon_home_, alt_home_);

  // fill SITLGps msg
  sensor_msgs::msgs::SITLGps gps_msg;
  sensor_msgs::msgs::SITLGps raw_gps_msg;

  gps_msg.set_time_usec(current_time.Double() * 1e6);
  gps_msg.set_time_utc_usec((current_time.Double() + start_time_.Double()) * 1e6);

  // @note Unfurtonately the Gazebo GpsSensor seems to provide bad readings,
  // starting to drift and leading to global position loss
  // gps_msg.set_latitude_deg(parentSensor_->Latitude().Degree());
  // gps_msg.set_longitude_deg(parentSensor_->Longitude().Degree());
  // gps_msg.set_altitude(parentSensor_->Altitude());
  gps_msg.set_latitude_deg(latlon.first * 180.0 / M_PI);
  gps_msg.set_longitude_deg(latlon.second * 180.0 / M_PI);
  gps_msg.set_altitude(pos_W_I.Z() + alt_home_ - noise_gps_pos_.Z() + gps_bias_.Z());

  std_xy_ = 1.0;
  std_z_ = 1.0;
  gps_msg.set_eph(std_xy_);
  gps_msg.set_epv(std_z_);

  gps_msg.set_velocity_east(velocity_current_W.X() + noise_gps_vel_.Y());
  gps_msg.set_velocity(velocity_current_W_xy.Length());
  gps_msg.set_velocity_north(velocity_current_W.Y() + noise_gps_vel_.X());
  gps_msg.set_velocity_up(velocity_current_W.Z() - noise_gps_vel_.Z());

  gps_msg.set_attack_enabled(attack_enabled || (rot_attack_enabled && !is_nearly_equal(degree_offset_, 0.0)) );

  {
    // protect shared variables
    std::lock_guard<std::mutex> lock(data_mutex_);

    // add msg to buffer
    gps_delay_buffer_.push(gps_msg);
    current_time_ = current_time;
  }

  last_time_ = current_time;
}
bool GpsPlugin::is_nearly_equal(double a, double b)
{
  return fabs(a-b) < offset_increment;
}
void GpsPlugin::OnSensorUpdate()
{
  // protect shared variables
  std::lock_guard<std::mutex> lock(data_mutex_);

  sensor_msgs::msgs::SITLGps gps_msg;
  sensor_msgs::msgs::SITLGps raw_gps_msg;
  gazebo::msgs::Vector3d info_msg;

  // apply GPS delay
  if ((current_time_ - last_gps_time_).Double() > 1 / parentSensor_->UpdateRate()) {
    last_gps_time_ = current_time_;

    // do not sent empty msg
    // abort if buffer is empty
    if (gps_delay_buffer_.empty()) {
      return;
    }

    while (true) {

      if (gps_delay_buffer_.empty()) {
        // abort if buffer is empty already
        break;
      }

      gps_msg = gps_delay_buffer_.front();
      // add
      raw_gps_msg=gps_msg;

      double gps_current_delay = current_time_.Double() - gps_delay_buffer_.front().time_usec() / 1e6f;

      // remove data that is too old or if buffer size is too large
      if (gps_current_delay >= gps_delay_) {
        gps_delay_buffer_.pop();
      // remove data if buffer too large
      } else if (gps_delay_buffer_.size() > gps_buffer_size_max_) {
        gps_delay_buffer_.pop();
      } else {
        // if we get here, we have good data, stop
        break;
      }
    }
    if (attack_enabled == true)
    {
      // Latitude
      if (latitude_deg_offset_ < latitude_deg_offset_target_)
      {
        latitude_deg_offset_ = latitude_deg_offset_ + offset_increment;
      }
      else if (latitude_deg_offset_ > latitude_deg_offset_target_)
      {
        latitude_deg_offset_ = latitude_deg_offset_ - offset_increment;
      }

      // Longitude
      if (longitude_deg_offset_ < longitude_deg_offset_target_)
      {
        longitude_deg_offset_ = longitude_deg_offset_ + offset_increment;
      }
      else if (longitude_deg_offset_ > longitude_deg_offset_target_)
      {
        longitude_deg_offset_ = longitude_deg_offset_ - offset_increment;
      }

      // Altitude
      if (altitude_offset_ < altitude_offset_target_)
      {
        altitude_offset_ = altitude_offset_ + offset_increment*10000;
      }
      else if (altitude_offset_ > altitude_offset_target_)
      {
        altitude_offset_ = altitude_offset_ - offset_increment*10000;
      }

      // Set Attack Enabled
      // if (is_nearly_equal((pow(latitude_deg_offset_, 2) + pow(longitude_deg_offset_, 2) + pow(altitude_offset_, 2)), 0.0))
      // {
      //   attack_enabled = false;
      // }

      if(altitude_offset_ == altitude_offset_target_ && latitude_deg_offset_ == latitude_deg_offset_target_ && longitude_deg_offset_== longitude_deg_offset_target_){
        attack_enabled = false;
      }

      double i = (rand() % 9 + 1) * 0.0000001;
      gps_msg.set_latitude_deg(gps_msg.latitude_deg() + latitude_deg_offset_ );
      gps_msg.set_longitude_deg(gps_msg.longitude_deg() + longitude_deg_offset_);
      gps_msg.set_altitude(gps_msg.altitude() + altitude_offset_);

    }

    // Set GPS Spoofing Rotation Offsets
    if (rot_attack_enabled == true)
    {
      // Degree Offset
      if (degree_offset_ < degree_offset_target_)
      {
        degree_offset_ = degree_offset_ + offset_increment;
      }
      else if (degree_offset_ > degree_offset_target_)
      {
        degree_offset_ = degree_offset_ - offset_increment;
      }

      // Direction
      if (direction_needed_ == true)
      {
        double velocity_east = gps_msg.velocity_east();
        double velocity_north = gps_msg.velocity_north();
        double magnitude = sqrt(pow(velocity_east, 2) + pow(velocity_north, 2));

        if (magnitude >= 0.1)
        {
          unit_vector_east_ = velocity_east / magnitude;
          unit_vector_north_ = velocity_north / magnitude;
          direction_needed_ = false;
        }
      }

      double offset_cos = degree_offset_ * cos_rot_;
      double offset_sin = degree_offset_ * sin_rot_;

      // Rotate by 'rotation_' degrees.
      // | cos(rotation_rad) -sin(rotation_rad) | * | u_v_velocity_east  |
      // | sin(rotation_rad) cos(rotation_rad)  |   | u_v_velocity_north |
      gps_msg.set_latitude_deg(gps_msg.latitude_deg() + (offset_cos * unit_vector_east_) - (offset_sin * unit_vector_north_));
      gps_msg.set_longitude_deg(gps_msg.longitude_deg() + (offset_sin * unit_vector_east_) + (offset_cos * unit_vector_north_));

      // Set Rot Attack Enabled
      if (is_nearly_equal(degree_offset_, 0.0))
      {
        std::lock_guard<std::mutex> lock(mutex_);
        rot_attack_enabled = false;
        cv_.notify_one();
      }
    }
    // gps_msg.set_altitude(gps_msg.altitude() + 1.5);
    // publish SITLGps msg at the defined update rate
    info_msg.set_x(latitude_deg_offset_target_);
    info_msg.set_y(attack_enabled);
    info_msg.set_z(latitude_deg_offset_);
    info_pub_->Publish(info_msg);
    gps_pub_->Publish(gps_msg);
    gps_data_pub_->Publish(raw_gps_msg);
  }
}
void GpsPlugin::OnOffsetUpdate(const boost::shared_ptr<const msgs::Vector3d> &_msg)
{
  latitude_deg_offset_target_ = _msg->x();
  longitude_deg_offset_target_ = _msg->y();
  altitude_offset_target_ = _msg->z();
  attack_enabled = ((pow(latitude_deg_offset_target_, 2.0) + pow(longitude_deg_offset_target_, 2.0) + pow(altitude_offset_target_, 2.0)) != 0.0) || ((pow(latitude_deg_offset_, 2) + pow(longitude_deg_offset_, 2) + pow(altitude_offset_, 2)) != 0.0);

  gzmsg << "[gazebo_gps_plugin] GPS offset updated to: latitude: " <<
    latitude_deg_offset_ <<
    " longitude: " << longitude_deg_offset_ <<
    " altitude: " << altitude_offset_ << "\n";
}

void GpsPlugin::OnRotationOffsetUpdate(const boost::shared_ptr<const msgs::Vector3d> &_msg)
{
  std::thread t([this, _msg](){
      std::unique_lock<std::mutex> lock(mutex_);

      if (rot_attack_enabled == true)
      {
        degree_offset_target_ = 0.0;
      }

      while (rot_attack_enabled)
      {
        cv_.wait(lock);
      }

      // Can perform spoofing without modifying
      degree_offset_target_ = _msg->x();
      rotation_ = _msg->y() * PI / 180.0;
      rot_attack_enabled = degree_offset_target_ > 0.0;
      direction_needed_ = true;

      // Change Constants
      cos_rot_ = cos(rotation_);
      sin_rot_ = sin(rotation_);
    if ((degree_offset_target_ ==0.0) and (rotation_==0.0))
      rot_attack_enabled = false;
  });
  t.detach();
  gzmsg << "[gazebo_gps_plugin] GPS rotation offset updated to: offset degrees: " <<
    degree_offset_ <<
    " rotation: " << rotation_ << "\n";
}

} // namespace gazebo
