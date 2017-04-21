function JpegDecoder() {
    const Image = require("./image.js");
    const fs = require("fs");
    const ImageProcessingC = require('../build/Release/imageprocessing.node');
    const self = this;
    self.decode = function(buffer) {
        var obj = ImageProcessingC.getPixels(buffer);
        var image = new Image();
        image.putPixels(obj.data, obj.width, obj.height);
        return image;
    }
    self.fromFile = function(path) {
        return self.decode(fs.readFileSync(path));
    }
    return self;
}
module.exports = new JpegDecoder;