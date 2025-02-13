#!/bin/bash

if [$# -ne 2] then
    echo "Usage: ./install.sh <PX4 directory>"
    exit 1
fi

px4_home = $1

cp "./bin/*" "$px4_home/Tools/simulation/gazebo-class/sitl_gazebo-classic/"
cp "./px4/*" "$px4_home/src/modules/simulation/simulator_mavlink/"
cd "$px4_home"
make px4_sitl gazebo-classic