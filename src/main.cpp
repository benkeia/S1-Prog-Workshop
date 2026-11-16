#include <sil/sil.hpp>

void keep_green_only(sil::Image &image)
{
    for (glm::vec3 &color : image.pixels())
    {
        color.r = 0.f;
        color.b = 0.f;
    }
}

void swap_channel(sil::Image &image)
{

    for (glm::vec3 &color : image.pixels())
    {
        std::swap(color.r, color.b);
    }
}

void black_white(sil::Image &image)
{

    for (glm::vec3 &color : image.pixels())
    {

        float luminance = color.r * 0.2126f + color.g * 0.7152f + color.b * 0.0722f;

        color.r = luminance;
        color.g = luminance;
        color.b = luminance;
    }
}

int main()
{
    sil::Image image{"images/logo.png"};
    // sil::Image image{"images/photo.jpg"};
    // sil::Image image{500, 500};

    black_white(image);

    image.save("output/black_white.png");
}
