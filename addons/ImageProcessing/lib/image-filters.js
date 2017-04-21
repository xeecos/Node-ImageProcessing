function ImageFilter() {
    const self = this;
    const JpegDecoder = require("./jpeg-decoder.js");
    const ImageUtils = require("./image-utils.js");
    const ImageProcessingC = require('../build/Release/imageprocessing.node');

    self.grayScale = function(data) {
        var image = ImageUtils.toImage(data);
        var obj = ImageProcessingC.grayScale(image.buffer, image.width, image.height);
        return obj;
    }
    self.colorScale = function(data, threshold) {
        threshold = threshold || 4;
        var image = ImageUtils.toImage(data);
        return ImageProcessingC.colorScale(image.buffer, image.width, image.height, threshold);
    }
    self.binarization = function(data, threshold) {
        var image = ImageUtils.toImage(data);
        return ImageProcessingC.binarization(image.buffer, image.width, image.height, threshold);
    }
    self.edge = function(data, sigma, lowThreshold, highThreshold) {
        var image = ImageUtils.toImage(data);
        return ImageProcessingC.edge(image.buffer, image.width, image.height, sigma, lowThreshold, highThreshold, true);
    }
    self.histogram = function(data) {
        var image = ImageUtils.toImage(data);
        var t = new Date().getTime();
        var obj = ImageProcessingC.histogram(image.buffer, image.width, image.height);
        console.log(new Date().getTime() - t);
        return obj;
    }
    return self;
}
module.exports = new ImageFilter;