function Image() {
    const self = this;
    self.pixels = [];
    self.data = null;
    self.width = 0;
    self.height = 0;
    self.putPixels = function(data, width, height) {
        if (data.length == width * height * 3) {
            self.pixels = [];
            for (var i = 0; i < width; i++) {
                var col = [];
                for (var j = 0; j < height; j++) {
                    col.push(data[i * height + j]);
                };
                self.pixels.push(col);
            }
            self.data = Buffer.from(data);
            self.width = width;
            self.height = height;
        }
    }
    self.isValid = function() {
        return self.width > 0 && self.height > 0;
    }
}
module.exports = Image;