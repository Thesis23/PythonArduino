import cv2
import numpy as np

# Load the image
image_path = './testimages/cropped.jpg'
image = cv2.imread(image_path)
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

def update(*args):
    alpha = cv2.getTrackbarPos('Alpha', 'Masked Image') / 100
    beta = cv2.getTrackbarPos('Beta', 'Masked Image')
    lower_range_v = cv2.getTrackbarPos('Lower Range V', 'Masked Image')
    upper_range_v = cv2.getTrackbarPos('Upper Range V', 'Masked Image')

    gray_image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    adjusted_image = cv2.convertScaleAbs(gray_image, alpha=alpha, beta=beta)
    rgb_image = cv2.cvtColor(adjusted_image, cv2.COLOR_GRAY2RGB)
    hsv_image = cv2.cvtColor(rgb_image, cv2.COLOR_RGB2HSV)

    lower_range = np.array([0, 0, lower_range_v])
    upper_range = np.array([180, 255, upper_range_v])

    mask = cv2.inRange(hsv_image, lower_range, upper_range)
    masked_image = cv2.bitwise_and(rgb_image, rgb_image, mask=mask)
    masked_image_bgr = cv2.cvtColor(masked_image, cv2.COLOR_RGB2BGR)

    cv2.imshow('Masked Image', masked_image_bgr)

cv2.namedWindow('Masked Image')
cv2.createTrackbar('Alpha', 'Masked Image', 10, 300, update)  # Multiply slider range by 10
cv2.createTrackbar('Beta', 'Masked Image', 0, 100, update)
cv2.createTrackbar('Lower Range V', 'Masked Image', 100, 255, update)
cv2.createTrackbar('Upper Range V', 'Masked Image', 240, 255, update)

update()

while True:
    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break

cv2.destroyAllWindows()
