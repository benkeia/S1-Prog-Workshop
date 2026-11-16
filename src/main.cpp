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

int main()
{
    sil::Image image{"images/logo.png"};
    // sil::Image image{"images/photo.jpg"};
    // sil::Image image{500, 500};

    swap_channel(image);

    image.save("output/debug.png");
}
