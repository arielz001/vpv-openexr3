#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <Imath/half.h>
#include <Imath/ImathBox.h>
#include <cstdio>
#include <cstdlib>
#include <exception>

struct iio_image {
    int w, h, pd, type;
    void* data;
};

#define IIO_TYPE_FLOAT 1

void iio_image_init2d(struct iio_image* x, int w, int h, int pd, int type) {
    x->w = w; x->h = h; x->pd = pd; x->type = type;
    x->data = nullptr;
}

extern "C" int read_whole_exr(struct iio_image *x, const char *filename)
{
    try {
        OPENEXR_IMF_NAMESPACE::RgbaInputFile f(filename);
        const OPENEXR_IMF_NAMESPACE::Header &header = f.header();
        const IMATH_NAMESPACE::Box2i &dataWindow = header.dataWindow();

        int xmin = dataWindow.min.x;
        int ymin = dataWindow.min.y;
        int xmax = dataWindow.max.x;
        int ymax = dataWindow.max.y;

        int width = xmax - xmin + 1;
        int height = ymax - ymin + 1;

        OPENEXR_IMF_NAMESPACE::Array2D<OPENEXR_IMF_NAMESPACE::Rgba> pixels(height, width);
        f.setFrameBuffer(&pixels[0][0], 1, width);
        f.readPixels(ymin, ymax);

        float* finaldata = (float*)malloc(4 * width * height * sizeof(*finaldata));

        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++) {
                int idx = (i * width + j) * 4;
                finaldata[idx + 0] = pixels[i][j].r;
                finaldata[idx + 1] = pixels[i][j].g;
                finaldata[idx + 2] = pixels[i][j].b;
                finaldata[idx + 3] = pixels[i][j].a;
            }

        iio_image_init2d(x, width, height, 4, IIO_TYPE_FLOAT);
        x->data = finaldata;

        return 0;
    } catch (const std::exception &e) {
        fprintf(stderr, "Error leyendo EXR %s: %s\n", filename, e.what());
        return -1;
    }
}
