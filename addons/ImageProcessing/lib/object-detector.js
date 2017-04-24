function ObjectDetect() {
    const self = this;
    const ImageUtils = require("./image-utils.js");
    const ImageProcessingC = require('../build/Release/imageprocessing.node');
    self.detect = function(data, shape) {
        shape = shape || "circle";
        var image = ImageUtils.toImage(data);
        return ImageProcessingC.objectDetect(image.data, image.width, image.height, 8, 15, 6, shape);
    }
    return self;
}
module.exports = new ObjectDetect;