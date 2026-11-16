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
void invert(sil::Image &image)
{
    for (glm::vec3 &color : image.pixels())
    {
        color = 1.f - color;
    }
}

void gradient(sil::Image &image)
{
    for (int x{0}; x < image.width(); x++)
    {
        float color_value = static_cast<float>(x) / static_cast<float>(image.width() - 1);
        for (int y{0}; y < image.height(); y++)
        {
            image.pixel(x, y) = glm::vec3{color_value};
        }
    }
}

void mirror(sil::Image &image)
{
    for (int x{0}; x < image.width() / 2; x++)
    {
        for (int y{0}; y < image.height(); y++)
        {
            std::swap(image.pixel(x, y), image.pixel(image.width() - 1 - x, y));
        }
    }
}

int main()
{
    sil::Image image{"images/logo.png"};
    //  sil::Image image{"images/photo.jpg"};
    // sil::Image image{300, 200};

    mirror(image);

    image.save("output/mirror.png");
}
