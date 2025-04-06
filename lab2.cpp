#include <iostream>
#include <vector>
#include <stack>
#include </usr/local/Cellar/libpng/1.6.44/include/libpng16/png.h>
#include <cmath>
#include <fstream>

using namespace std;

png_byte closest(int value, int levels) {
    int k=256/(levels-1);
    int new_=round(value/(double)k)*k;
    new_ = max(0, min(new_, 255));
    return new_;
}

vector< vector<png_byte> > dithering(vector< vector<png_byte> > image, int n_bits) {
    png_uint_32 height = image.size();
    png_uint_32 width = image[0].size();
    int levels = (1 << n_bits);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int old_pixel = image[y][x];
            int new_pixel = closest(old_pixel, levels);
            image[y][x] = new_pixel;
            int mistake = (int)old_pixel - (int)new_pixel;
            if (x + 1 < width) image[y][x + 1] = static_cast<png_byte>(max(0.0f, min(255.0f, image[y][x + 1] + mistake * (7.0f / 16.0f))));
            if (x > 0 && y + 1 < height) image[y + 1][x - 1] = static_cast<png_byte>(max(0.0f, min(255.0f, image[y + 1][x - 1] + mistake * (3.0f / 16.0f))));
            if (y + 1 < height) image[y + 1][x] = static_cast<png_byte>(max(0.0f, min(255.0f, image[y + 1][x] + mistake * (5.0f / 16.0f))));
            if (x + 1 < width && y + 1 < height) image[y + 1][x + 1] = static_cast<png_byte>(max(0.0f, min(255.0f, image[y + 1][x + 1] + mistake * (1.0f / 16.0f))));
        }
    }
    return image;
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
    /*
    if (bit_depth == 16) {
        png_set_strip_16(png);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
     */
    png_set_rgb_to_gray(png, 1, 0.299, 0.587);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    png_read_update_info(png, info);
    vector< vector<png_byte> > row_pointers(height, vector<png_byte>(png_get_rowbytes(png, info)));
    for (int y = 0; y < height; y++) {
        png_bytep row = row_pointers[y].data();
        png_read_row(png, row, NULL);
    }
    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);
    return row_pointers;
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


int main() {
    png_uint_32 width=635, height=356;
    vector< vector<png_byte> > img1 = read("initial.png", width, height);
    img1 = dithering(img1, 1);
    write("new.png", img1);
    return 0;
}
