#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include "jpeg-decoder.h"
#include "canny.h"

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;

using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Array;
using v8::String;
using v8::Value;
int max(int a,int b){
    return a>b?a:b;
}
int min(int a,int b){
    return a>b?b:a;
}
int getHSV(int r,int g, int b,int threshold){
	// int cv = 3;
	int h = 0;
	// r = floor(r/cv)*cv;
	// g = floor(g/cv)*cv;
	// b = floor(b/cv)*cv;
	int max_c = max(r,max(g,b));
	int min_c = min(r,min(g,b));
	if(abs(max_c-min_c)<threshold){
		h = -1000;
	}else if(r==max_c){
		if(g>=b){
			h = 60*(g-b)/(max_c-min_c);
		}else{
			h = 60*(g-b)/(max_c-min_c)+360;
		}
	}else if(g==max_c){
		h = 60*(b-r)/(max_c-min_c)+120;
	}else if(b==max_c){
		h = 60*(r-g)/(max_c-min_c)+240;
	}
	return h;
}
#define GRAY_LEVEL 255
void histogram(unsigned char*pixels,int width,int height){
    unsigned long numPixelR[GRAY_LEVEL] = { 0 };  // number of each gray level
	unsigned long numPixelG[GRAY_LEVEL] = { 0 };
	unsigned long numPixelB[GRAY_LEVEL] = { 0 };
    double length = width * height;
    long index = 0;
    int gray = 0;
    for (long i = 0; i <  width * height; i++) {
        pixels[i*3] *= 0.999;
        pixels[i*3+1] *= 0.999;
        pixels[i*3+2] *= 0.999;
	}
    for (long i = 0; i <  width * height; i++) {
        index = i*3;
        numPixelR[pixels[index]]++;
        numPixelG[pixels[index+1]]++;
        numPixelB[pixels[index+2]]++;
	}
    
	double probPixelR[GRAY_LEVEL] = { 0 };
	double probPixelG[GRAY_LEVEL] = { 0 };
	double probPixelB[GRAY_LEVEL] = { 0 };
	for (int i = 0; i < GRAY_LEVEL; i++) {
		probPixelR[i] = numPixelR[i] / length;
		probPixelG[i] = numPixelG[i] / length;
		probPixelB[i] = numPixelB[i] / length;
	}
	double cumuPixelR[GRAY_LEVEL] = { 0 };
	double cumuPixelG[GRAY_LEVEL] = { 0 };
	double cumuPixelB[GRAY_LEVEL] = { 0 };
	cumuPixelR[0] = probPixelR[0];
	cumuPixelG[0] = probPixelG[0];
	cumuPixelB[0] = probPixelB[0];
	for (int i = 1; i < GRAY_LEVEL; i++) {
		cumuPixelR[i] = cumuPixelR[i - 1] + probPixelR[i];
		cumuPixelG[i] = cumuPixelG[i - 1] + probPixelG[i];
		cumuPixelB[i] = cumuPixelB[i - 1] + probPixelB[i];
	}

	for (int i = 0; i < GRAY_LEVEL; i++) {
		cumuPixelR[i] = 255 * cumuPixelR[i];
		cumuPixelG[i] = 255 * cumuPixelG[i];
		cumuPixelB[i] = 255 * cumuPixelB[i];
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            index = (x*height+y)*3;
			pixels[index] = cumuPixelR[pixels[index]];
			pixels[index+1] = cumuPixelG[pixels[index+1]];
			pixels[index+2] = cumuPixelB[pixels[index+2]];
		}
	}
}
void colorScale(unsigned char*pixels,int width,int height,int threshold){
    int gray;
    for (int i = 0; i < width * height; ++i) {
        // gray = 0.299 * pixels[3*i] + 0.587 * pixels[3*i + 1] + 0.114 * pixels[3*i + 2];
        int h = getHSV(pixels[3*i],pixels[3*i+1],pixels[3*i+2],threshold);
        if(h==-1000){
            gray = 0.299 * pixels[3*i] + 0.587 * pixels[3*i + 1] + 0.114 * pixels[3*i + 2];
            gray = 0x0;//gray>127?0xff:0x0;
            pixels[3*i] = gray;
            pixels[3*i + 1] = gray;
            pixels[3*i + 2] = gray;
        }
        // gray = 255-h;
        // if((h<30||h>330)&&gray<200){
        //   gray = 0xff;
        // }else{
        //   gray = 0x0;
        // }
        // pixels[3*i] = gray;
        // pixels[3*i + 1] = gray;
        // pixels[3*i + 2] = gray;
    }
}
void grayScale(unsigned char*pixels,int width,int height){
    int gray;
    int index;
    for (int i = 0; i < width * height; ++i) {
        index = i*3;
        gray = 0.299 * pixels[index] + 0.587 * pixels[index + 1] + 0.114 * pixels[index + 2];
        pixels[3*i] = gray;
        pixels[3*i + 1] = gray;
        pixels[3*i + 2] = gray;
    }
}
void binarization(unsigned char*pixels,int width,int height,int threshold){
    int gray;
    for (int i = 0; i < width * height; ++i) {
        if (pixels[3 * i] < threshold) {
            pixels[3 * i] = 0;
            pixels[3 * i + 1] = 0;
            pixels[3 * i + 2] = 0;
        } else {
            pixels[3 * i] = 255;
            pixels[3 * i + 1] = 255;
            pixels[3 * i + 2] = 255;
        }
    }
}
void erosion(unsigned char*pixels,int width,int height) {
    unsigned char *pixelData = (unsigned char *)malloc(width*height*sizeof(unsigned char)); // 読み取り用ピクセルデータ
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            pixelData[i*height+j] = pixels[3* (width * j + i)];
        }
    }
    unsigned char *buf = (unsigned char *)malloc(width*height*sizeof(unsigned char));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            buf[i*height+j] = pixelData[i*height+j];
        }
    }
    // 縮小処理
    int isAdjacentToBlack, x, y;
    int selectAry[8] = {1, 0,
        0, 1,
        -1, 0,
        0, -1};
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            // pixel が黒なら飛ばす
            if (pixelData[i*height+j] == 0) {
                continue;
            }
            isAdjacentToBlack = 0;
            for (int k = 0; k < 4; ++k) {
                x = i + selectAry[k*2];
                y = j + selectAry[k*2+1];
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (pixelData[x*height+y] == 0) {
                        isAdjacentToBlack = 1;
                        break;
                    }
                }
                if (isAdjacentToBlack==1) {
                    break;
                }
            }
            if (isAdjacentToBlack==1) {
                buf[i*height+j] = 0;
            }
        }
    }
    // 結果をコピー
    int idx;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            idx = width * i + j;
            pixels[3 * idx] = buf[j*height+i];
            pixels[3 * idx + 1] = buf[j*height+i];
            pixels[3 * idx + 2] = buf[j*height+i];
        }
    }
}
void dilation(unsigned char*pixels,int width,int height){
    unsigned char *pixelData = (unsigned char *)malloc(width*height*sizeof(unsigned char)); // 読み取り用ピクセルデータ
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            pixelData[i*height+j] = pixels[3* (width * j + i)];
        }
    }
    unsigned char *buf = (unsigned char *)malloc(width*height*sizeof(unsigned char));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            buf[i*height+j] = pixelData[i*height+j];
        }
    }
    // 膨張処理
    int x, y;
    int selectAry[8] = {1, 0,
        0, 1,
        -1, 0,
        0, -1};
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (pixelData[i*height+j] == 255) {
                for (int k = 0; k < 4; ++k) {
                    x = i + selectAry[k*2];
                    y = j + selectAry[k*2+1];
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        buf[x*height+y] = 255;
                    }
                }
            }
        }
    }
    // 結果をコピー
    int idx;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            idx = width * i + j;
            pixels[3 * idx] = buf[j*height+i];
            pixels[3 * idx + 1] = buf[j*height+i];
            pixels[3 * idx + 2] = buf[j*height+i];
        }
    }
}
void openning(unsigned char*pixels,int width,int height,int numIteration){
  // 収縮
    for (int i = 0; i < numIteration; ++i) {
        erosion(pixels,width,height);
    }
    // 膨張
    for (int i = 0; i < numIteration; ++i) {
        dilation(pixels,width,height);
    }
}

void searchStartPixel(unsigned char*pixels,unsigned char*buf,int width,int height,int *size){
        int idx;
        int x, y;
        int leftPx;
        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                if (x == 0) {
                    leftPx = 255;
                } else {
                    leftPx = pixels[((x - 1)*height+y)];
                }
                if (leftPx == 255 && pixels[x*height+y] == 0 && buf[x*height+y] == 255) {
                    buf[x*height+y] = 0;
                    size[0] = x;
                    size[1] = y;
                    return ;
                }
            }
        }
        size[0] = width;
        size[1] = height;
}
void contourDetection(unsigned char*pixels,int width,int height,v8::Isolate* isolate){
    // 読み取り用ピクセルデータ（書き換えない）
    unsigned char *pixelData = (unsigned char *)malloc(width*height*sizeof(unsigned char)); // 読み取り用ピクセルデータ
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            pixelData[i*height+j] = pixels[3* (width * j + i)];
        }
    }
    unsigned char *buf = (unsigned char *)malloc(width*height*sizeof(unsigned char));
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            buf[i*height+j] = 255;
        }
    }

    // あるピクセルを * で表し、
    // 周囲のピクセルを下のように番号を付けて表す
    // 3 2 1
    // 4 * 0
    // 5 6 7 
    int nextCode[8] = {7, 7, 1, 1, 3, 3, 5, 5};
    // Freeman's chain code
    int chainCode[16] = {1, 0,
        1, -1,
        0, -1,
        -1, -1,
        -1, 0,
        -1, 1,
        0, 1,
        1, 1};
    int idx;
    int rel; // relativee pisition
    int relBuf; // previous rel
    int dPx[2] = {0,0}; // detected pixel 輪郭として検出されたピクセルのテンポラリー変数
    int startPx[2] = {0,0}; // 輪郭追跡の開始ピクセル
    int sPx[2] = {0,0}; // searching pixel
    int isClosed = 0; // 輪郭が閉じていれば true
    int isStandAlone; // 孤立点ならば true
    Local<Array> pixelsArr[50];
    for(int i=0;i<50;i++){
        pixelsArr[i]=Array::New(isolate);
    }
    Local<Array> dupPixelsArr=Array::New(isolate);
    int pxs[1024]; // 輪郭のピクセル座標の配列を格納するテンポラリー配列
    // int boundaryPxs = []; // 複数の輪郭を格納する配列
    int pxVal; // 着目するピクセルの色
    // int duplicatedPx = []; // 複数回、輪郭として検出されたピクセル座標を格納（将来的にこのような重複を許さないアルゴリズムにしたい）
    int index = 0;
    while (1) {
    //     // 輪郭追跡開始ピクセルを探す
        searchStartPixel(pixelData,buf,width,height,dPx);
        // 画像全体が検索された場合はループを終了
        if (dPx[0] == width && dPx[1] == height) {
            break;
        }
        pixelsArr[index]->Set(pixelsArr[index]->Length(),v8::Integer::New(isolate, dPx[0]));
        pixelsArr[index]->Set(pixelsArr[index]->Length(),v8::Integer::New(isolate, dPx[1]));
        startPx[0] = dPx[0];
        startPx[1] = dPx[1];
        isStandAlone = 0;
        isClosed = 0;
        relBuf = 5; // 最初に調べるのは5番
    //     
    //     // 輪郭が閉じるまで次々に周囲のピクセルを調べる
        while (!isClosed) {
            for (int i = 0; i < 8; ++i) {
                rel = (relBuf + i) % 8; // relBufから順に調べる
                sPx[0] = dPx[0] + chainCode[rel*2];
                sPx[1] = dPx[1] + chainCode[rel*2+1];
    //             // sPx が画像上の座標外ならば白として評価する
                if (sPx[0] < 0 || sPx[0] >= width || sPx[1] < 0 || sPx[1] >= height) {
                    pxVal = 255;
                } else {
                    pxVal = pixelData[sPx[0]*height+sPx[1]];
                }
    //             // もし調べるピクセルの色が黒ならば新しい輪郭とみなす
    //             // 最初のピクセルに戻れば次の輪郭を探す
    //             // 周囲の8ピクセルがすべて白ならば孤立点なので次の輪郭を探す
                if (pxVal == 0) {
                    if (buf[sPx[0]*height+sPx[1]] == 0) {
                      dupPixelsArr->Set(dupPixelsArr->Length(),v8::Integer::New(isolate, sPx[0]));
                      dupPixelsArr->Set(dupPixelsArr->Length(),v8::Integer::New(isolate, sPx[1]));
                    }
    //                 // 検出されたピクセルが輪郭追跡開始ピクセルならば
    //                 // 追跡を終了して次の輪郭に移る
                    if (sPx[0] == startPx[0] && sPx[1] == startPx[1]) {
                        isClosed = 1;
                        break;
                    }
                    buf[sPx[0]*height+sPx[1]] = 0; // 検出された点を黒にする
                    dPx[0] = sPx[0];
                    dPx[1] = sPx[1];
                    pixelsArr[index]->Set(pixelsArr[index]->Length(),v8::Integer::New(isolate, dPx[0]));
                    pixelsArr[index]->Set(pixelsArr[index]->Length(),v8::Integer::New(isolate, dPx[1]));
                    relBuf = nextCode[rel];
                    break;
                }
                if (i == 7) {
                    isStandAlone = 1;
                }
            }
            if (isStandAlone==1) {
                break;
            }
        }
        index++;
        if(index>49){
          break;
        }
    }
    for(int i=0; i<height; ++i) {
    	for(int j=0; j<width; ++j) {
    		idx=width*i+j;
    		pixels[3*idx]=0xff;//buf[j*height+i];
    		pixels[3*idx+1]=0xff;//buf[j*height+i];
    		pixels[3*idx+2]=0xff;//buf[j*height+i];
    	}
    }

    for (int i = 0; i < 50; ++i) {
        float cx = 0;
        float cy = 0;
        float minX = 1000;
        float maxX = 0;
        float minY = 1000;
        float maxY = 0;
        for (int j = 0; j < pixelsArr[i]->Length()/2; ++j) {
            float px = pixelsArr[i]->Get(j*2)->Int32Value();
            float py = pixelsArr[i]->Get(j*2+1)->Int32Value();
            cx += px;
            cy += py;
            if (minX > px) {
                minX = px;
            }
            if (maxX < px) {
                maxX = px;
            }
            if (minY > py) {
                minY = py;
            }
            if (maxY < py) {
                maxY = py;
            }
        }
        cx /= pixelsArr[i]->Length()/2;
        cy /= pixelsArr[i]->Length()/2;
        float dr = ((maxY - minY) / 2 + (maxX - minX) / 2) / 2;
        float r = 0;
        int count = 0;
        float cr = 0;
        for (int j = 0; j < pixelsArr[i]->Length()/2; ++j) {
            float dx = cx - pixelsArr[i]->Get(j*2)->Int32Value();
            float dy = cy - pixelsArr[i]->Get(j*2+1)->Int32Value();
            cr = sqrt(dx * dx + dy * dy);
            if (abs(cr - dr) > 10) {
                count++;
            }
            r += cr;
        }
        if (count > 5 || cr<20) {
             continue;
        }
        for(int j=0;j<pixelsArr[i]->Length();j++){
          idx=width*pixelsArr[i]->Get(j*2+1)->Int32Value()+pixelsArr[i]->Get(j*2)->Int32Value();
          pixels[3*idx]=255;
          pixels[3*idx+1]=0;
          pixels[3*idx+2]=0;
        }
    }
}
void getPixels(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);
  Local<Object> bufferObj = args[0]->ToObject();
//   Local<Value> threshold = args[1];
//   Local<Value> numIteration = args[2];
//   Local<Value> lowThreshold = args[2];
//   Local<Value> highThreshold = args[3];
//   Local<Value> sigma = args[1];
  const unsigned char * buf = (const unsigned char *)node::Buffer::Data(bufferObj);
  size_t size = node::Buffer::Length(bufferObj);
  Jpeg::Decoder decoder(buf, size);
  Local<Object> pixelsObj = Object::New(isolate);
  unsigned char *pixels = decoder.GetImage();
  int width = decoder.GetWidth();
  int height = decoder.GetHeight();
//   grayScale(pixels,width,height);
//   CannyEdgeDetector *canny = new CannyEdgeDetector();
//   unsigned char *edge ;
//   pixels = canny->ProcessImage(pixels,width,height,sigma->NumberValue(),lowThreshold->Int32Value(),highThreshold->Int32Value());
//  canny.edges(edge, pixels, filter, low_threshold,   high_threshold);
//   binarization(pixels,width,height,threshold->Int32Value());
//   openning(pixels,width,height,numIteration->Int32Value());
//   contourDetection(pixels,width,height,isolate);
  long pixelsLength = decoder.GetImageSize();
  Local<Array> pixelsArr = Array::New(isolate);
  for(int i=0;i<pixelsLength;i++){
    pixelsArr->Set(i,v8::Integer::New(isolate,pixels[i]));
  }
  pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
  pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
  pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
  args.GetReturnValue().Set(pixelsObj);
}
void getGrayScale(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Object> bufferObj = args[0]->ToObject();
    int width = args[1]->Int32Value();
    int height = args[2]->Int32Value();
    unsigned char * pixels = ( unsigned char *)node::Buffer::Data(bufferObj);
    Local<Object> pixelsObj = Object::New(isolate);
    grayScale(pixels,width,height);
    Local<Array> pixelsArr = Array::New(isolate);
    for(int i=0;i<width*height*3;i++){
        pixelsArr->Set(i,v8::Integer::New(isolate,pixels[i]));
    }
    pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
    pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
    pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
    args.GetReturnValue().Set(pixelsObj);
}
void getColorScale(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Object> bufferObj = args[0]->ToObject();
    int width = args[1]->Int32Value();
    int height = args[2]->Int32Value();
    int threshold = args[3]->Int32Value();
    unsigned char * pixels = ( unsigned char *)node::Buffer::Data(bufferObj);
    Local<Object> pixelsObj = Object::New(isolate);
    colorScale(pixels,width,height,threshold);
    Local<Array> pixelsArr = Array::New(isolate);
    for(int i=0;i<width*height*3;i++){
        pixelsArr->Set(i,v8::Integer::New(isolate,pixels[i]));
    }
    pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
    pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
    pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
    args.GetReturnValue().Set(pixelsObj);
}
void getBinarization(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Object> bufferObj = args[0]->ToObject();
    int width = args[1]->Int32Value();
    int height = args[2]->Int32Value();
    int threshold = args[3]->Int32Value();
    unsigned char * pixels = ( unsigned char *)node::Buffer::Data(bufferObj);
    Local<Object> pixelsObj = Object::New(isolate);
    grayScale(pixels,width,height);
    binarization(pixels,width,height,threshold);
    Local<Array> pixelsArr = Array::New(isolate);
    for(int i=0;i<width*height*3;i++){
        pixelsArr->Set(i,v8::Integer::New(isolate,pixels[i]));
    }
    pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
    pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
    pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
    args.GetReturnValue().Set(pixelsObj);
}
void getEdge(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Object> bufferObj = args[0]->ToObject();
    int width = args[1]->Int32Value();
    int height = args[2]->Int32Value();
    int sigma = args[3]->Int32Value();
    int lowThreshold = args[4]->Int32Value();
    int highThreshold = args[5]->Int32Value();
    int invert = args[6]->Int32Value();
    unsigned char * pixels = ( unsigned char *)node::Buffer::Data(bufferObj);
    Local<Object> pixelsObj = Object::New(isolate);
    CannyEdgeDetector *canny = new CannyEdgeDetector();
    pixels = canny->ProcessImage(pixels,width,height,sigma,lowThreshold,highThreshold);
    Local<Array> pixelsArr = Array::New(isolate);
    for(int i=0;i<width*height*3;i++){
        pixelsArr->Set(i,v8::Integer::New(isolate,invert?(255-pixels[i]):pixels[i]));
    }
    pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
    pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
    pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
    args.GetReturnValue().Set(pixelsObj);
}
void getHistogram(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Object> bufferObj = args[0]->ToObject();
    int width = args[1]->Int32Value();
    int height = args[2]->Int32Value();
    unsigned char * pixels = ( unsigned char *)node::Buffer::Data(bufferObj);
    Local<Object> pixelsObj = Object::New(isolate);
    histogram(pixels,width,height);
    Local<Array> pixelsArr = Array::New(isolate);
    for(int i=0;i<width*height*3;i++){
        pixelsArr->Set(i,v8::Integer::New(isolate,pixels[i]));
    }
    pixelsObj->Set(String::NewFromUtf8(isolate, "data"), pixelsArr);
    pixelsObj->Set(String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate,width));
    pixelsObj->Set(String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate,height));
    args.GetReturnValue().Set(pixelsObj);
}
void init(v8::Local<v8::Object> exports) {
  NODE_SET_METHOD(exports, "getPixels", getPixels);
  NODE_SET_METHOD(exports, "grayScale", getGrayScale);
  NODE_SET_METHOD(exports, "colorScale", getColorScale);
  NODE_SET_METHOD(exports, "binarization", getBinarization);
  NODE_SET_METHOD(exports, "edge", getEdge);
  NODE_SET_METHOD(exports, "histogram", getHistogram);
}

NODE_MODULE(binding, init);