from flask import Flask, jsonify, request, render_template
import asyncio
from aiortc import RTCPeerConnection, RTCSessionDescription, VideoStreamTrack
import cv2
import av
import threading

app = Flask(__name__, template_folder='.')

class VideoCamera:
    def __init__(self, source=0):
        self.video = cv2.VideoCapture(source)
        self.video.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        self.video.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

    def get_frame(self):
        ret, frame = self.video.read()
        return frame

class VideoStreamReceiver(VideoStreamTrack):
    def __init__(self, camera):
        super().__init__()
        self.camera = camera
    
    async def recv(self):
        frame = self.camera.get_frame()
        av_frame = av.VideoFrame.from_ndarray(frame, format="bgr24")
        av_frame.pts = av_frame.time * 1000
        return av_frame


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/offer", methods=["POST"])
def offer():
    data = request.get_json()
    offer = RTCSessionDescription(sdp=data["sdp"], type=data["type"])
    
    pc = RTCPeerConnection()
    pc.addTrack(VideoStreamReceiver(VideoCamera()))

    loop = asyncio.new_event_loop()

    try:
        asyncio.set_event_loop(loop)
        loop.run_until_complete(pc.setRemoteDescription(offer))
        answer = loop.run_until_complete(pc.createAnswer())
        loop.run_until_complete(pc.setLocalDescription(answer))
    finally:
        loop.close()

    return jsonify({"sdp": pc.localDescription.sdp, "type": pc.localDescription.type})

def run():
    app.run(host="0.0.0.0", port=5050)


if __name__ == "__main__":
    web_thread = threading.Thread(target=run)
    web_thread.start()

    asyncio.get_event_loop().run_forever()
