# Unix Image Processing and Manipulation Tool
This is a Unix command line application for quick and easy image manipulation.

## Supported Image Types
* JPEG
* PNG
* GIF
* TIFF

## Installation
### Step 1:
Pull the project from this repository.
### Step 2:
Navigate into the project folder

    $ cd ../ImageProcessing2022SeniorProj/Project
### Step 3:
To install the dependencies run these commands 

    $ sudo apt-get update -y
    $ sudo apt-get install libgtk2.0-dev
    $ sudo apt-get install -y libgd-dev

Now the application is ready to run.
### Step 4:
To run the program, use the commands

    $ make
    $ ./run

## Function Library

### Read Image

    read <image-name> into <new-buffer-name>
* image-name = name of input image
* new-buffer-name = name of the buffer where this image is stored
* example: "read cars.png into buff"
### Write Image

    write <buffer-name> into <new-image-name>
* new-image-name = name of the output image that will be created
### Display Buffer

    display <buffer-name>
* buffer-name = name of a buffer created with the read function or other functions that output buffers
### Pointwise Arithmetic (+-*/)

    addition : <new-buffer-name> = <buffer1> + <buffer2>
* buffer1 and buffer2 are existing buffers
* for the other operations replace "addition" and "+" with "operation-name" and "operation-symbol" respectively. e.g. "multiplication" and "*"
### Brighten / Darken

    brighten <buffer-name> into <new-buffer-name> by <value between 0 and 255>
* for darkening replace "brighten" with "darken"
### Edge Detection

    <horizontal/vertical/combined> <kirsch/prewitt/sobel> <buffer-name> into <new-buffer-name>
* there are 6 different combinations to be made here: there are three different methods of edge detections, Kirsch, Sobel, and Prewitt, and there are three orientations 
### Histogram Equalization

    histeq <buffer-name> into <new-buffer-name>
### Flip

    flip <vertical/horizontal> <buffer-name>
### Rotate

    rotate <buffer-name> by <degrees>
* degrees must be a value between -360 and 360
### Blur

    blurr <buffer> <radius> <sigma>
* radius is the defines the blur pattern (10 is a good number for this)
* sigma is the factor to blur by (bigger number = more blur)
### Sharpen

    sharpen <low/high> <buffer>
### Convolution 3x3

    convolve3x3 <template> <buffer-name>
* template in this case is a 3x3 array and the input must be in the form \[a b c\]\[d e f\]\[g h i\] where a,...,i are any rational numbers

### Padding to nearest 2^n

    pad <buffer> into <new-buffer-name>
### Define Template

    define_template <template-name> <template-structure> = <(x,y)>
* template structure is like the one mentioned in convolve3x3 but is not limited to a 3x3 array
* x,y is the target pixel for the convolution

### Templated Convolution

    convolve_template <buff-name> <template-name>
