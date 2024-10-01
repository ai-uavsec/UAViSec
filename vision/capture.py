import gi
import cv2
import numpy as np
import threading
import sys
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib



# Initialize GStreamer
Gst.init(None)

class VideoCapture:
    def __init__(self, pipeline_description):
        self.pipeline = Gst.parse_launch(pipeline_description)
        self.appsink = self.pipeline.get_by_name("sink")
        self.appsink.set_property("emit-signals", True)
        self.appsink.set_property("sync", False)
        self.appsink.connect("new-sample", self.on_new_sample)
        self.loop = GLib.MainLoop()
        self.frame_count = 0
        self.capture_frame = False
        self.quit_program = False

        # Start a thread to listen for keyboard input
        self.input_thread = threading.Thread(target=self.listen_for_input)
        self.input_thread.start()

    def on_new_sample(self, sink):
        sample = sink.emit("pull-sample")
        buf = sample.get_buffer()
        caps = sample.get_caps()

        if self.capture_frame:
            self.frame_count += 1

            # Retrieve frame data
            result, map_info = buf.map(Gst.MapFlags.READ)
            if result:
                frame_data = map_info.data
                # Retrieve the frame dimensions
                width = caps.get_structure(0).get_value('width')
                height = caps.get_structure(0).get_value('height')
                # Create a numpy array from the raw frame data
                frame = np.frombuffer(frame_data, np.uint8).reshape((height, width, 3))
                # Convert from RGB to BGR
                frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
                frame_file = f"./frames/frame_{self.frame_count}.jpg"
                cv2.imwrite(frame_file, frame)
                print(f"Saved {frame_file}")
                buf.unmap(map_info)
            
            self.capture_frame = False
        
        return Gst.FlowReturn.OK

    def listen_for_input(self):
        while not self.quit_program:
            user_input = input()
            if user_input == 'c':
                self.capture_frame = True
            elif user_input == 'q':
                self.quit_program = True
                self.pipeline.set_state(Gst.State.NULL)
                self.loop.quit()

    def start(self):
        self.pipeline.set_state(Gst.State.PLAYING)
        try:
            self.loop.run()
        except:
            pass
        self.pipeline.set_state(Gst.State.NULL)
        self.quit_program = True
        self.input_thread.join()

if __name__ == "__main__":
    # Define the GStreamer pipeline
    pipeline_description = (
        "udpsrc port=5600 caps=application/x-rtp,media=video,clock-rate=90000,encoding-name=H264 ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw,format=RGB ! appsink name=sink"
    )
    
    # Create the VideoCapture object
    video_capture = VideoCapture(pipeline_description)
    
    # Start capturing video frames
    video_capture.start()
