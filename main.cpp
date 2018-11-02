#include <iostream>
#include <thread>
#include <stdlib.h>
#include <string>
#include <ctime>
#include <unistd.h>

#include <ApplicationServices/ApplicationServices.h>

using namespace std;

void saveImageToFile(const CGImageRef& image, const char* path) {
    CFStringRef file = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8); 
    CFStringRef type = CFSTR("public.jpeg");
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file, kCFURLPOSIXPathStyle, false);
    CGImageDestinationRef idst = CGImageDestinationCreateWithURL(urlRef, type, 1, NULL);
    CGImageDestinationAddImage(idst, image, NULL);
    CGImageDestinationFinalize(idst);

    CFRelease(file);
    CFRelease(type);
}

inline CGImageRef takeScreenShot() {
    return CGWindowListCreateImage(CGRectInfinite,
      kCGWindowListOptionOnScreenOnly, kCGNullWindowID, kCGWindowImageDefault);
}

void getArgValue(string arg, int argc, const char* argv[], string* outImagePath)
{
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i] == arg) {
            *outImagePath = argv[i+1];
            return;
        }
    }
}

int main(int argc, const char* argv[]) {
    static const int DEFAULT_DELAY_SECS = 3;

    string outImagePath = "Image.jpg";
    getArgValue("-out", argc, argv, &outImagePath);

    int delay = DEFAULT_DELAY_SECS;
    string rawDelaySecs;
    getArgValue("-delay", argc, argv, &rawDelaySecs);
    if (rawDelaySecs.size()) {
        delay = atoi(rawDelaySecs.c_str());
    }

    CGImageRef screenShot1 = takeScreenShot();

    cout << "waiting " << delay << " seconds..." << endl;
    usleep(delay*1000*1000);
    cout << "Done. New screenshot taken.\n";

    CGImageRef screenShot2 = takeScreenShot();

    int width  = CGImageGetWidth(screenShot2);
    int height = CGImageGetHeight(screenShot2);
    assert(width == CGImageGetWidth(screenShot1));
    assert(height == CGImageGetHeight(screenShot1));

    // allocate memory for pixels
    uint32_t* pixels1 = new uint32_t[width * height];
    uint32_t* pixels2 = new uint32_t[width * height];

    // create a context with RGBA pixels
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context1 = CGBitmapContextCreate(
        pixels1, width, height, 8, width * sizeof(uint32_t), colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedLast );
    CGContextRef context2 = CGBitmapContextCreate(
        pixels2, width, height, 8, width * sizeof(uint32_t), colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedLast );

    // // draw the image into the context
    CGContextDrawImage(context1, CGRectMake( 0, 0, width, height), screenShot1);
    CGContextDrawImage(context2, CGRectMake( 0, 0, width, height), screenShot2);

    // // manipulate the pixels
    uint32_t* bufptr1 = pixels1;
    uint32_t* bufptr2 = pixels2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
        {
            if (*bufptr1 != *bufptr2) {
                uint32_t value = *bufptr1 ;
                value = 0x00FF00FF;  // RGBA
                *bufptr1 = value;
            }
            ++bufptr1;
            ++bufptr2;
        }
    }   

    // // create a new CGImage from the context with modified pixels
    CGImageRef resultImage = CGBitmapContextCreateImage(context1);
    saveImageToFile(resultImage, outImagePath.c_str());
    cout << "Image saved to " << outImagePath << endl;

    //release resources to free up memory
    CGContextRelease(context1);
    CGContextRelease(context2);
    CGColorSpaceRelease(colorSpace);
    delete[] pixels1;
    delete[] pixels2;

    return 0;
}