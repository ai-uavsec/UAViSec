# UAV-Camera-Capture

The `capture.py` is used to capture the frames of the camera live stream.

## CAPTURE
The required packages are installed when make PX4, so no need to install extra packages.

We need to change the stream from `unicast` to `multicast` mode to capture and download the live frames.

1. Open the SDF file for your target vehicle (e.g. `./Tools/simulation/gazebo-classic/sitl_gazebo-classic/models/typhoon_h480/typhoon_h480.sdf`).

2. Search for the `GstCameraPlugin` plugin.
```XML
    <plugin name='GstCameraPlugin' filename='libgazebo_gst_camera_plugin.so'>
    <robotNamespace><robotNamespace/>
    <udpHost>127.0.0.1</udpHost>
    <udpPort>5600</udpPort>
    </plugin>
``````
3. Add the line ` <multicast>true</multicast>` before `</plugin>`. Remember, after this operation, the QGC's Video Address setting needs to be modified to `239.255.12.42` to receive the camera live data or set `multicast` element to `false` to receive the camera live stream.
4. You can use a `gst-launch` to get the stream video window.
 ```console
gst-launch-1.0 -v udpsrc multicast-group=239.255.12.42 auto-multicast=true port=5600 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink fps-update-interval=1000 sync=false
```   

5. Run the `Capture.py` script.

    ```console
	  python3 capture.py
``````xml
      <options> -c : capture the current frame.
                -q : quit the script.``````
``````
## Live detection with YOLO


### OPTION 1 python script:

1. Download the YOLO config files. 

```console
wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4.cfg \
&& wget https://github.com/AlexeyAB/darknet/releases/download/darknet_yolo_v3_optimal/yolov4.weights \
&& wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/data/coco.names 
```
2. Run the python script `yolo.py`. We can get a video window of the live detection on gazebo camera. 
```console
python3 yolo.py
```

### OPTION 2 Use A Gstreamer plugin:

However the fps for the python3 script may be low than expect. We have the other option use a gstreamer plugin [gst-darknet](https://github.com/aler9/gst-darknet).

1. Follow the installation to install build dependencies


2. Run the `rtp2rtsp.py`  Transform the RTP into RTSP with gstreamer
```console
python3 rtp2rtsp.py
```
3. Run the `gst-dark.sh` 
```console
chmod +x gst-dark.sh
./gst-dark.sh
```
