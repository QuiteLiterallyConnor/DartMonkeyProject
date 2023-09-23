from flask import Flask, Response
import cv2
import sys

app = Flask(__name__)

# Open the webcam
video_capture = cv2.VideoCapture(1)

def generate_frames():
    while True:
        success, frame = video_capture.read()  # read the camera frame
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/stream')
def video():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='192.168.0.106', port=sys.argv[1])
