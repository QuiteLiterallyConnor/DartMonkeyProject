from flask import Flask, Response
from vidgear.gears import VideoGear
import cv2

app = Flask(__name__)

# Open the webcam
stream = VideoGear(source=1).start()

def generate_frames():
    while True:
        frame = stream.read()
        if frame is None:
            continue
        # Encode frame as JPEG
        _, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video')
def video():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/')
def index():
    return """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Webcam Stream</title>
</head>
<body>
    <img src="/video" width="640" height="480" />
</body>
</html>"""

if __name__ == "__main__":
    app.run(port=sys.argv[1])
