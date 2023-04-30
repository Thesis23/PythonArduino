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

def download_all_images():
    client = google_storage.Client.from_service_account_json(
        "./thesis-e256f-firebase-adminsdk-q07bn-9b933ee51c.json"
    )
    bucket_name = bucket.name
    gcs_bucket = client.get_bucket(bucket_name)

    blobs = list(gcs_bucket.list_blobs())
    image_blobs = [blob for blob in blobs if blob.content_type.startswith("")]

    if not image_blobs:
        print("No images found in the Firebase Storage.")
        return

    script_dir = os.path.dirname(os.path.realpath(__file__))

    for image_blob in image_blobs:
        print(f"Downloading image: {image_blob.name}")

        # Download the image to a temporary file
        with tempfile.NamedTemporaryFile(delete=False) as temp_file:
            image_blob.download_to_file(temp_file)
            temp_file.flush()

        # Save the image as a JPG in the same folder as the script
        image_filename = f"{image_blob.name}"
        image_path = os.path.join(script_dir, image_filename)

        with open(temp_file.name, "rb") as src, open(image_path, "wb") as dest:
            dest.write(src.read())

        # Clean up the temporary file
        os.remove(temp_file.name)

        print(f"Image saved to: {image_path}")

if __name__ == "__main__":
    download_all_images()
