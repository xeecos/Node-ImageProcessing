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

    self.toImage = function(data, scale) {
        var image = null;
        if (isBuffer(data)) {
            image = JpegDecoder.decode(data);
        } else if (isImage(data)) {
            image = data;
        } else if (isString(data)) {
            image = JpegDecoder.fromFile(data);
        } else if (isPixels(data)) {
            image = new Image();
            image.putPixels(data.data, data.width, data.height);
        }
        if (scale && image) {
            var per = Math.round(1 / scale);
            var output = new Image();
            var pixels = [];
            var oriWidth = image.width;
            var oriHeight = image.height;
            var width = Math.round(image.width * scale);
            var height = Math.round(image.height * scale)
            for (var j = 0; j < height; j++) {
                for (var i = 0; i < width; i++) {
                    var x = Math.floor(i * per);
                    var y = Math.floor(j * per);
                    var index = (y * oriWidth + x) * 3;
                    pixels.push(image.data[index]);
                    pixels.push(image.data[index + 1]);
                    pixels.push(image.data[index + 2]);
                }
            }
            console.log(pixels.length / 3 / 640);
            output.putPixels(pixels, width, height);
            return output;
        }
        return image;
    }
    return self;
}
module.exports = new ImageUtils;