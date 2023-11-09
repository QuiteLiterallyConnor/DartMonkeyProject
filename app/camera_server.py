from flask import Flask, Response, render_template
import cv2
import sys
from pyngrok import ngrok

app = Flask(__name__)

# Open the webcam
video_capture = cv2.VideoCapture(0)

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

@app.route('/')
def index():
    return render_template('camera_index.html')

@app.route('/video')
def video():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    # Check if a port number is provided as a command-line argument
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    else:
        # Default port to use if no argument is provided
        port = 5000
    
    # Start ngrok tunnel to the specified port
    ngrok_tunnel = ngrok.connect(port)
    print('ngrok tunnel "http://localhost:{}" -> "{}"'.format(port, ngrok_tunnel.public_url))

    # Run the Flask app on the specified port
    app.run(host='0.0.0.0', port=port)
