import cv2
import numpy as np
import gi

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# Init Gstreamer live data
Gst.init(None)

class GstVideo:
    def __init__(self, pipeline_str):
        self.pipeline = Gst.parse_launch(pipeline_str)
        self.appsink = self.pipeline.get_by_name("appsink")
        self.appsink.set_property("emit-signals", True)
        self.appsink.connect("new-sample", self.on_new_sample)
        self.frame = None

    def on_new_sample(self, sink):
        sample = sink.emit("pull-sample")
        buf = sample.get_buffer()
        caps = sample.get_caps()
        arr = np.ndarray(
            (
                caps.get_structure(0).get_value("height"),
                caps.get_structure(0).get_value("width"),
                3,
            ),
            buffer=buf.extract_dup(0, buf.get_size()),
            dtype=np.uint8,
        )
        self.frame = arr.copy()  # Make the array writable
        return Gst.FlowReturn.OK

    def read(self):
        return self.frame

def load_yolo_model(config_path, weights_path, labels_path):
    net = cv2.dnn.readNetFromDarknet(config_path, weights_path)
    with open(labels_path, 'r') as f:
        labels = f.read().strip().split('\n')
    return net, labels

def draw_bounding_boxes(image, boxes, confidences, class_ids, labels):
    for i in range(len(boxes)):
        x, y, w, h = boxes[i]
        label = str(labels[class_ids[i]])
        confidence = confidences[i]
        color = (0, 255, 0)  # Green color for the bounding box
        cv2.rectangle(image, (x, y), (x + w, y + h), color, 2)
        cv2.putText(image, f'{label} {confidence:.2f}', (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

def detect_objects(net, layer_names, frame, conf_threshold=0.5, nms_threshold=0.4):
    height, width = frame.shape[:2]
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (416, 416), swapRB=True, crop=False)
    net.setInput(blob)
    layer_outputs = net.forward(layer_names)

    boxes = []
    confidences = []
    class_ids = []

    for output in layer_outputs:
        for detection in output:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > conf_threshold:
                center_x = int(detection[0] * width)
                center_y = int(detection[1] * height)
                w = int(detection[2] * width)
                h = int(detection[3] * height)
                x = int(center_x - w / 2)
                y = int(center_y - h / 2)
                boxes.append([x, y, w, h])
                confidences.append(float(confidence))
                class_ids.append(class_id)

    indices = cv2.dnn.NMSBoxes(boxes, confidences, conf_threshold, nms_threshold)
    if isinstance(indices, np.ndarray):
        indices = indices.flatten().tolist()
    else:
        indices = []

    boxes = [boxes[i] for i in indices]
    confidences = [confidences[i] for i in indices]
    class_ids = [class_ids[i] for i in indices]

    return boxes, confidences, class_ids


config_path = "yolov4.cfg"  
weights_path = "yolov4.weights"  
labels_path = "coco.names"  

net, labels = load_yolo_model(config_path, weights_path, labels_path)
layer_names = net.getUnconnectedOutLayersNames()

## gstreamer source
pipeline_str = (
    "udpsrc multicast-group=239.255.12.42 auto-multicast=true port=5600 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264\" ! "
    "rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=BGR ! appsink name=appsink"
)

video = GstVideo(pipeline_str)
video.pipeline.set_state(Gst.State.PLAYING)

while True:
    frame = video.read()
    if frame is not None:
        boxes, confidences, class_ids = detect_objects(net, layer_names, frame)
        draw_bounding_boxes(frame, boxes, confidences, class_ids, labels)
        cv2.imshow("YOLO Object Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

video.pipeline.set_state(Gst.State.NULL)
cv2.destroyAllWindows()
