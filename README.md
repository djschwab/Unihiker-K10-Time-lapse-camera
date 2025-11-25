# Unihiker K10 time lapse camera
This sketch implements a time lapse camera for the Unihiker K10 ESP32s3 device. It saves photo images in jpg format to the SD card.
The on-screen setup allows for time intervals from 1 second to 99 hours. It also allows for the images to be captured in 5 different resolutions:
240x320, 640x480, 800x600, 1280x720, and 1280x1024. Although the Unihiker API documentation only allows for 240x320 format images, I found
that by using the ESP32 API for the camers (esp_camera_init and esp_camera_fb_get), it is possible to capture camera images at higher resolutions.
For some reason that I can't figure out, the framesize descriptors that generate the different resolutions are not what you would expect.
FRAMESIZE_QVGA => 240x320,
FRAMESIZE_SVGA => 640x480,
FRAMESIZE_XGA => 800x600,
FRAMESIZE_SXGA => 1280x720,
FRAMESIZE_UXGA} => 1280x1024
Also, I was not able to change any of the camera's 'sensor' settings like brightness, contrast, or saturation except for hmirror(), which seems
to be required for proper operation.
If anyone knows why the framesize descriptors aren't accurate, or how to implement the rest of the camera's 'sensor' settings, I'd appreciate
your advice.



