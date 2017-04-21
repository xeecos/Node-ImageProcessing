function ObjectDetect() {
    const self = this;
    const JpegDecoder = require("./jpeg-decoder.js");
    const ImageProcessingC = require('../build/Release/imageprocessing.node');
    self.detect = function(data, shape) {
        shape = shape || "circle";
        var image = ImageUtils.toImage(data);
        return ImageProcessingC.objectDetect(image.buffer, image.width, image.height, shape);
    }
    return self;
}
module.exports = new ObjectDetect;