const fs = require("fs");
const jpeg = require('jpeg-js');
var buffer = fs.readFileSync("./assets/test.jpg");
const ImageProcessing = require('../addons/ImageProcessing/index.js');
// .getPixels(buffer, 250, 3);
var image = ImageProcessing.circleDetect(ImageProcessing.edge(ImageProcessing.resize(buffer, 0.05), 1, 30, 30), 4, 8, 6); //ImageProcessing.circleDetect((buffer));
var width = image.width;
var height = image.height;
console.log(width, height, image.objects)
var frameData = new Buffer(width * height * 4);
var i = 0;
while (i < frameData.length / 4) {
    frameData[i * 4] = image.data[i * 3]; // red 
    frameData[i * 4 + 1] = image.data[i * 3 + 1]; // green 
    frameData[i * 4 + 2] = image.data[i * 3 + 2]; // blue 
    frameData[i * 4 + 3] = 0xFF; // alpha - ignored in JPEGs 
    i++;
}
var rawImageData = {
    data: frameData,
    width: width,
    height: height
};
var jpegImageData = jpeg.encode(rawImageData, 100);
fs.writeFileSync("./assets/output.jpg", jpegImageData.data);