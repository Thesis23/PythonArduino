import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage import img_as_float
from scipy.ndimage import gaussian_filter
from skimage.morphology import reconstruction

image = cv2.imread('./testimages/fig.png')

image = img_as_float(image)
image = gaussian_filter(image, 1)

# define the contrast and brightness value
contrast = 60. # Contrast control ( 0 to 127)
brightness = 100. # Brightness control (0-100)

# call addWeighted function. use beta = 0 to effectively only operate on one image
image = cv2.addWeighted( image, contrast, image, 0, brightness)

seed = np.copy(image)
seed[1:-1, 1:-1] = image.min()
mask = image

dilated = reconstruction(seed, mask, method='dilation')

fig, (ax0, ax1, ax2) = plt.subplots(nrows=1,
                                    ncols=3,
                                    figsize=(8, 2.5),
                                    sharex=True,
                                    sharey=True)

ax0.imshow(image, cmap='gray')
ax0.set_title('original image')
ax0.axis('off')

ax1.imshow(dilated, vmin=image.min(), vmax=image.max(), cmap='gray')
ax1.set_title('dilated')
ax1.axis('off')

ax2.imshow(image - dilated, cmap='gray')
ax2.set_title('image - dilated')
ax2.axis('off')

fig.tight_layout()

# img = image - dilated
# plt.imsave('img.jpg', img, cmap='gray')

plt.show()