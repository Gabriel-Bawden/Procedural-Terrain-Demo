from PIL import Image

# Open the BMP image
img = Image.open("C:\\Users\\gabeb\\Downloads\\NormalMap.bmp").convert('RGB')
pixels = img.load() # Create a pixel map

width, height = img.size

# Iterate through pixels to invert only the Red channel
for x in range(width):
    for y in range(height):
        r, g, b = pixels[x, y]
        # Invert only the R value
        pixels[x, y] = (255 - r, g, b)

# Save the result
img.save("C:\\Users\\gabeb\\Downloads\\NormalMap2.bmp")