import cv2
import numpy as np
import matplotlib.pyplot as plt

def to_grayscale(image):
    # Convert the image to grayscale
    gray_image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    return gray_image

def increase_brightness_contrast(image, alpha=1.0, beta=0):
    # Adjust the brightness and contrast of the image
    adjusted_image = cv2.convertScaleAbs(image, alpha=alpha, beta=beta)
    return adjusted_image

def mask_foam_sediment(image_path):
    # Load the image
    image = cv2.imread(image_path)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Convert the image to grayscale
    image = to_grayscale(image)

    # Increase the brightness and contrast of the image
    image = increase_brightness_contrast(image, alpha=0.87, beta=0)

    # Convert the grayscale image back to RGB
    image = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)

    # Convert the image to HSV
    hsv = cv2.cvtColor(image, cv2.COLOR_RGB2HSV)

    # Define the color range for the foam and sediment
    lower_range = np.array([0, 0, 86])  # Lower range for foam and sediment
    upper_range = np.array([180, 240, 118])  # Upper range for foam and sediment (V <= 240)

    # Create a mask using the color range
    mask = cv2.inRange(hsv, lower_range, upper_range)

    # Apply the mask to the original image
    masked_image = cv2.bitwise_and(image, image, mask=mask)

    return masked_image

# Provide the path to the image of fermenting beer
image_path = './testimages/fig.jpg'

# Mask the image and display the foam and sediment
masked_image = mask_foam_sediment(image_path)

# Display the original and masked images
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 5))

ax1.imshow(cv2.cvtColor(cv2.imread(image_path), cv2.COLOR_BGR2RGB))
ax1.set_title('Original Image')
ax1.axis('off')

ax2.imshow(masked_image)
ax2.set_title('Foam and Sediment')
ax2.axis('off')

plt.show()