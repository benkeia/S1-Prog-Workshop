#include <sil/sil.hpp>

void keep_green_only(sil::Image &image)
{
    for (glm::vec3 &color : image.pixels())
    {
        color.r = 0.f;
        color.b = 0.f;
    }
}

int main()
{
    sil::Image image{"images/logo.png"};
    // sil::Image image{"images/photo.jpg"};
    // sil::Image image{500, 500};

    keep_green_only(image);

    image.save("output/result.png");
}
