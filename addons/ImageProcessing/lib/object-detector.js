function ObjectDetect() {
    const self = this;
    const ImageUtils = require("./image-utils.js");
    const ImageProcessingC = require('../build/Release/imageprocessing.node');
    self.detect = function(data, minRadius, maxRadius, threshold, shape) {
        shape = shape || "circle";
        var image = ImageUtils.toImage(data);
        var t = new Date().getTime();
        var obj = ImageProcessingC.objectDetect(image.data, image.width, image.height, minRadius, maxRadius, threshold, shape);
        console.log("time:", new Date().getTime() - t);
        return obj;
    }
    return self;
}
module.exports = new ObjectDetect;