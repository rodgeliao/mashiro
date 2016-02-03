# mashiro
Using k-means cluster algorithm to computing the dominant colors of given image. 

## Use as library
* Copy all .cpp and .h files except main.cpp to your project.
* include mashiro.h
* Init class mashiro with an instance of cv::Mat
		
		Mat image = imread("/PATH/TO/AN/IMAGE");
		mashiro mashiro(image);

* Call color function

		mashiro.color(3, [](cv::Mat& image, Cluster colors){
		    for_each(colors.cbegin(), colors.cend(), [](const MashiroColor& color){
		        cout<<"("
		            <<std::get<mashiro::toType(MashiroColorRGBFields::Red)>(color)<<", "
		            <<std::get<mashiro::toType(MashiroColorRGBFields::Green)>(color)<<", "
		            <<std::get<mashiro::toType(MashiroColorRGBFields::Blue)>(color)<<")"
		        <<endl;
		    });
		});

    The first parameter is the k in K-means cluster algorithm
    
    The second parameter is a callback function, which gives the reference of the input image and clustered colors.

## Use as program
Just compile and install it with
```
$ make && make install
$ mashiro
Usage:
	-i [image file] -c [number of color to cluster]
	-h Print this help
```

Notice,

In Makefile, LIB and INCLUDE might need to be changed to give correct path to OpenCV.

### Example
```
mashiro -i cover.jpg -c 3
```

If everything is fine, you'll see

![Screenshot](https://raw.githubusercontent.com/BlueCocoa/mashiro/master/Screenshot.png)

#### Link
My [blog post](https://blog.0xbbc.com/2016/02/using-k-means-cluster-algorithm-to-compute-the-dominant-colors-of-given-image/)
