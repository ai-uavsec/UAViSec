import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GLib

class RTSPServer:
    def __init__(self):
        self.server = GstRtspServer.RTSPServer()
        self.server.set_service('8554')  # RTSP server port

        self.factory = GstRtspServer.RTSPMediaFactory()
        self.factory.set_launch('( udpsrc multicast-group=239.255.12.42 auto-multicast=true port=5600 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" ! rtph264depay ! h264parse ! rtph264pay name=pay0 pt=96 )')
        # self.factory.set_launch('( udpsrc multicast-group=239.255.12.42 auto-multicast=true port=5600 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! videocrop left=40 right=40 top=20 bottom=20 ! x264enc ! rtph264pay name=pay0 pt=96 )')
        self.factory.set_shared(True)


        self.server.get_mount_points().add_factory('/test', self.factory)
        self.server.attach(None)

    def run(self):
        loop = GLib.MainLoop()
        loop.run()

if __name__ == '__main__':
    Gst.init(None)
    server = RTSPServer()
    server.run()
# gst-launch-1.0 \
# rtspsrc location=rtsp://127.0.0.1:8554/test ! decodebin ! videoconvert \
# ! darknetinfer config=yolov4.cfg weights=yolov4.weights \
# ! darknetrender labels=coco.names \
# ! videoconvert \
# ! xvimagesink sync=1
