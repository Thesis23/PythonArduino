import firebase_admin
from firebase_admin import credentials
from firebase_admin import storage as fb_storage
import os
import tempfile
from google.cloud import storage as google_storage

# Initialize the Firebase app with the provided credentials and bucket
cred = credentials.Certificate("./thesis-e256f-firebase-adminsdk-q07bn-9b933ee51c.json")
firebase_admin.initialize_app(cred, {
    'storageBucket': 'thesis-e256f.appspot.com'
})

bucket = fb_storage.bucket()

def get_latest_image():
    client = google_storage.Client.from_service_account_json(
        "./thesis-e256f-firebase-adminsdk-q07bn-9b933ee51c.json"
    )
    bucket_name = bucket.name
    gcs_bucket = client.get_bucket(bucket_name)
    
    blobs = list(gcs_bucket.list_blobs())
    blobs = [blob for blob in blobs if blob.content_type.startswith('')]
    
    if not blobs:
        print("No images found in the Firebase Storage.")
        return

    # Sort the blobs by their name, which should be in the format YYYY-MM-DD_HH-MM-SS
    latest_blob = max(blobs, key=lambda x: x.name)

    print(f"Downloading the latest image: {latest_blob.name}")

    # Download the image to a temporary file
    with tempfile.NamedTemporaryFile(delete=False) as temp_file:
        latest_blob.download_to_file(temp_file)
        temp_file.flush()

    # Save the image as a JPG in the same folder as the script
    script_dir = os.path.dirname(os.path.realpath(__file__))
    image_filename = f"{latest_blob.name}"
    image_path = os.path.join(script_dir, image_filename)

    with open(temp_file.name, "rb") as src, open(image_path, "wb") as dest:
        dest.write(src.read())

    # Clean up the temporary file
    os.remove(temp_file.name)

    print(f"Image saved to: {image_path}")

if __name__ == "__main__":
    get_latest_image()
