function ImageProcessing() {
    const self = this;
    const ImageFilter = require("./lib/image-filters.js");
    self.grayScale = function(data) {
        return ImageFilter.grayScale(data);
    }
    self.colorScale = function(data, threshold) {
        return ImageFilter.colorScale(data, threshold);
    }
    self.binarization = function(data, threshold) {
        return ImageFilter.binarization(data, threshold);
    }
    self.edge = function(data, sigma, lowThreshold, highThreshold) {
        return ImageFilter.edge(data, sigma, lowThreshold, highThreshold);
    }
    self.histogram = function(data) {
        return ImageFilter.histogram(data);
    }
    return self;
}
module.exports = new ImageProcessing;