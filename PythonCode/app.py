import cv2
import time
import datetime
import signal
import sys
from flask import Flask, render_template, Response
from firebase_admin import credentials, storage, initialize_app
from apscheduler.schedulers.background import BackgroundScheduler


# Set up Flask app
app = Flask(__name__)

# Connect to Firebase
cred = credentials.Certificate("./thesis-e256f-firebase-adminsdk-q07bn-9b933ee51c.json")
initialize_app(cred, {
    'storageBucket': 'thesis-e256f.appspot.com'
})

bucket = storage.bucket()

def take_picture():
    print("Taking picture... ")
    ret, frame = camera.read()
    if ret:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        image_name = f"images/{timestamp}.jpg"

        # Save the image locally
        cv2.imwrite(image_name, frame)

        # Upload the image to Firebase Storage
        with open(image_name, "rb") as image_file:
            blob = bucket.blob(image_name)
            blob.upload_from_file(image_file)

def generate_frames():
    global camera
    camera = cv2.VideoCapture(0)  # Use the default camera

    while True:
        ret, frame = camera.read()
        if not ret:
            break

        # Encode the frame for the live feed website
        _, jpeg = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\nContent-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')

        time.sleep(0.1)

    camera.release()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

def shutdown_server():
    camera.release()
    sys.exit()

def signal_handler(sig, frame):
    print("Shutting down gracefully...")
    shutdown_server()

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    
    scheduler = BackgroundScheduler()
    scheduler.add_job(take_picture, 'interval', minutes=30) # Set Timer HERE!
    scheduler.start()
    app.run(host='0.0.0.0', port=5000, threaded=True)