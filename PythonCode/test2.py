import cv2
import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.colors import hsv_to_rgb
import numpy as np

# Load and display the image
img = cv2.imread('./testimages/fig.jpg')
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
plt.imshow(img)
plt.show()

r, g, b = cv2.split(img)
fig = plt.figure()
axis = fig.add_subplot(1, 1, 1, projection="3d")

pixel_colors = img.reshape((np.shape(img)[0]*np.shape(img)[1], 3))
norm = colors.Normalize(vmin=-1.,vmax=1.)
norm.autoscale(pixel_colors)
pixel_colors = norm(pixel_colors).tolist()

axis.scatter(r.flatten(), g.flatten(), b.flatten(), facecolors=pixel_colors, marker=".")
axis.set_xlabel("Red")
axis.set_ylabel("Green")
axis.set_zlabel("Blue")
plt.show()

# Convert the image to HSV
hsv_img = cv2.cvtColor(img, cv2.COLOR_RGB2HSV)

h, s, v = cv2.split(hsv_img)
fig = plt.figure()
axis = fig.add_subplot(1, 1, 1, projection="3d")

axis.scatter(h.flatten(), s.flatten(), v.flatten(), facecolors=pixel_colors, marker=".")
axis.set_xlabel("Hue")
axis.set_ylabel("Saturation")
axis.set_zlabel("Value")
plt.show()

rgb_light_sediment = np.uint8([[[255, 255, 255]]])
rgb_dark_sediment = np.uint8([[[0, 0, 0]]])

# Convert the RGB color range to HSV
hsv_light_sediment = cv2.cvtColor(rgb_light_sediment, cv2.COLOR_RGB2HSV)
hsv_dark_sediment = cv2.cvtColor(rgb_dark_sediment, cv2.COLOR_RGB2HSV)

light_sediment = tuple(int(x) for x in hsv_light_sediment[0][0])
dark_sediment = tuple(int(x) for x in hsv_dark_sediment[0][0])

print("HSV Light sediment:", light_sediment)
print("HSV Dark sediment:", dark_sediment)

plt.subplot(1, 2, 1)
plt.imshow(hsv_to_rgb(np.full((10, 10, 3), light_sediment, dtype=np.uint8) / 255.0))
plt.subplot(1, 2, 2)
plt.imshow(hsv_to_rgb(np.full((10, 10, 3), dark_sediment, dtype=np.uint8) / 255.0))
plt.show()

# Create the mask using the HSV color range
mask = cv2.inRange(hsv_img, light_sediment, dark_sediment) 

# Apply the mask to the original image
result = cv2.bitwise_and(img, img, mask=mask)

# Display the mask and the resulting image
plt.subplot(1, 2, 1)
plt.imshow(mask, cmap="gray")
plt.subplot(1, 2, 2)
plt.imshow(result)
plt.show()

rgb_light_foam = np.uint8([[[255, 255, 255]]])
rgb_dark_foam = np.uint8([[[0, 0, 0]]])

# Convert the RGB color range to HSV
hsv_light_foam = cv2.cvtColor(rgb_light_foam, cv2.COLOR_RGB2HSV)
hsv_dark_foam = cv2.cvtColor(rgb_dark_foam, cv2.COLOR_RGB2HSV)

light_foam = tuple(int(x) for x in hsv_light_foam[0][0])
dark_foam = tuple(int(x) for x in hsv_dark_foam[0][0])

print("HSV Light foam:", light_foam)
print("HSV Dark foam:", dark_foam)

plt.subplot(1, 2, 1)
plt.imshow(hsv_to_rgb(np.full((10, 10, 3), light_foam, dtype=np.uint8) / 255.0))
plt.subplot(1, 2, 2)
plt.imshow(hsv_to_rgb(np.full((10, 10, 3), dark_foam, dtype=np.uint8) / 255.0))
plt.show()

# Create the mask using the HSV color range
mask = cv2.inRange(hsv_img, light_foam, dark_foam) 

# Apply the mask to the original image
result = cv2.bitwise_and(img, img, mask=mask)

# Display the mask and the resulting image
plt.subplot(1, 2, 1)
plt.imshow(mask, cmap="gray")
plt.subplot(1, 2, 2)
plt.imshow(result)
plt.show()