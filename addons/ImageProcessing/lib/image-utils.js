function ImageUtils() {
    const self = this;
    const JpegDecoder = require("./jpeg-decoder.js");
    const Image = require("./image.js");

    function isBuffer(data) {
        return data && typeof data === "object" && Buffer.isBuffer(data);
    }

    function isArray(data) {
        return (typeof data).toLowerCase().indexOf("object") > -1;
    }

    function isImage(data) {
        return data.buffer != null;
    }

    function isString(data) {
        return (typeof data).toLowerCase().indexOf("string") > -1;
    }

    function isPixels(data) {
        return data.data != null;
    }

    self.toImage = function(data) {
        if (isBuffer(data)) {
            var image = JpegDecoder.decode(data);
            return image;
        } else if (isImage(data)) {
            return data;
        } else if (isString(data)) {
            var image = JpegDecoder.fromFile(data);
            return image;
        } else if (isPixels(data)) {
            var image = new Image();
            image.putPixels(data.data, data.width, data.height);
            return image;
        }
        return null;
    }
    return self;
}
module.exports = new ImageUtils;