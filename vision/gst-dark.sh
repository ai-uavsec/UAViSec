#!/bin/bash

gst-launch-1.0 \
rtspsrc location=rtsp://127.0.0.1:8554/test ! decodebin ! videoconvert \
! darknetinfer config=yolov4.cfg weights=yolov4.weights \
! darknetrender labels=coco.names \
! videoconvert \
! xvimagesink sync=1
