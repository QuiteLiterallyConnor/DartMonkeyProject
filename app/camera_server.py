import cv2
from flask import Flask, Response

app = Flask(__name__)

# Open the camera
camera = cv2.VideoCapture(0)  # 0 is the default camera, you can change it if you have multiple cameras

def generate():
    while True:
        success, frame = camera.read()
        if not success:
            break
        ret, jpeg = cv2.imencode('.jpg', frame)
        if not ret:
            continue
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n\r\n')

@app.route('/stream')
def video_feed():
    return Response(generate(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='localhost', port=8081)  # Serve on all interfaces on port 8080
