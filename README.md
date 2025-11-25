# Unihiker K10 time lapse camera
This Arduino sketch implements a time lapse camera for the Unihiker K10 ESP32s3 device. It saves photo images in jpg format to the SD card.
The on-screen setup allows for time intervals from 1 second to 24 hours. It also allows for the images to be captured in 5 different resolutions:
- 240x320
- 640x480
- 800x600
- 1280x720
- 1280x1024

You can select between 'streaming' mode (direct output to the screen display, or time lapse mode (save jpg files to SD cards).
Use the 'A' button to toggle between selections and the 'B' button to confirm your selection.

Whwn setting the time interval for time lapse mode, use the 'A' button to increment hours, minutes, or seconds, and the 'B' button to lock in your selection.
 
Although the Unihiker API documentation only allows for 240x320 format images, I found
that by using the ESP32 API for the camera (esp_camera_init and esp_camera_fb_get), it is possible to capture camera images at higher resolutions.
For some reason that I can't figure out, the framesize descriptors that generate the different resolutions are not what you would expect.
- FRAMESIZE_QVGA => 240x320,
- FRAMESIZE_SVGA => 640x480,
- FRAMESIZE_XGA => 800x600,
- FRAMESIZE_SXGA => 1280x720,
- FRAMESIZE_UXGA => 1280x1024

Also, I was not able to change any of the camera's 'sensor' settings like brightness, contrast, or saturation except for hmirror(), which seems
to be required for proper operation.

If anyone knows why the framesize descriptors aren't accurate, or how to implement the rest of the camera's 'sensor' settings, I'd appreciate
your advice.

The file names of the jpg files on the SD card are imgxxxxx.jpg where xxxxx is the sequence number. You can make an animation
from the jpg files using ffmpeg:

ffmpeg -framerate 25 -i img%05d.jpg -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p output.mp4

The file output.mp4 is me first attempt at a timelapse movie. I hope I can make some better examples.

Please feel free use and improve this code. I'd appreciate hearing about what you do with it in the **Discussions** section.
