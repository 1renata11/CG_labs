#include <iostream>
#include <vector>
#include <stack>
#include </usr/local/Cellar/libpng/1.6.44/include/libpng16/png.h>
#include <cmath>
#include <fstream>

using namespace std;

void createImage(const char* filename, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        cerr << "Cannot open file" << endl;
        return;
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        cerr << "Cannot create PNG write struct" << endl;
        return;
    }
    png_infop info = png_create_info_struct(png);
    if (!info) {
        cerr << "Cannot create PNG info struct" << endl;
        return;
    }
    if(setjmp(png_jmpbuf(png))) return;
    png_init_io(png, fp);
    png_set_IHDR(
            png,
            info,
             width,
             height,
            8,
            PNG_COLOR_TYPE_GRAY,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    vector<png_byte> row(width);
    int centerX = width / 2;
    int centerY = height / 2;
    double maxDistance = min(centerX, centerY);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            double distance = sqrt((x - centerX)*(x - centerX) + (y - centerY)*(y - centerY));
            if (distance > maxDistance) row[x] = static_cast<png_byte>(255);
            else {
                row[x] = static_cast<png_byte>(255 * distance / maxDistance);
            }
        }
        png_write_row(png, row.data());
    }
    png_write_end(png, nullptr);
    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

vector< vector<png_byte> > read(const char* filename, png_uint_32& width, png_uint_32& height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        cerr << "Cannot open file" << endl;
        exit(1);
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) exit(1);
    png_infop info = png_create_info_struct(png);
    if (!info) exit(1);
    if (setjmp(png_jmpbuf(png))) exit(1);
    png_init_io(png, fp);
    png_read_info(png, info);
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    png_byte color_type = png_get_color_type(png, info);
    if (color_type!=PNG_COLOR_TYPE_GRAY || bit_depth != 8) {
        cerr << "Not 8 bpp image" << endl;
        exit(1);
    }
    vector< vector<png_byte> > image(height, vector<png_byte>(width));
    for (int y = 0; y < height; y++) {
        png_read_row(png, image[y].data(), nullptr);
    }
    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);
    return image;
}

void write(const char* filename, const vector< vector<png_byte> >& image) {
    size_t width = image[0].size();
    size_t height = image.size();
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        cerr << "Cannot open file" << endl;
        exit(1);
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) exit(1);
    png_infop info = png_create_info_struct(png);
    if (!info) exit(1);
    if (setjmp(png_jmpbuf(png))) exit(1);
    png_init_io(png, fp);
    png_set_IHDR(
            png,
            info,
             width,
             height,
            8,
            PNG_COLOR_TYPE_GRAY,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    for (int y = 0; y < height; y++) {
        png_write_row(png, image[y].data());
    }
    png_write_end(png, nullptr);
    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

vector< vector<png_byte> > blend(vector< vector<png_byte> > img1, vector< vector<png_byte> > img2, vector< vector<png_byte> > a) {
    size_t height = img1.size();
    size_t width = img1[0].size();
    vector< vector<png_byte> > blended(height, vector<png_byte>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            png_byte alpha = a[y][x];
            blended[y][x] = img1[y][x] * (255 - alpha) + img2[y][x] * alpha;
            blended[y][x]/=255;
        }
    }
    return blended;
}


int main() {
    createImage("image.png", 640, 480);
    png_uint_32 width=640, height=480;
    vector< vector<png_byte> > img1 = read("image.png", width, height);
    vector< vector<png_byte> > img2 = read("alpha.png", width, height);
    vector< vector<png_byte> > alpha = read("image1.png", width, height);
    vector<vector<png_byte> > blended = blend(img1, img2, alpha);
    write("blended_image.png", blended);
    return 0;
}
