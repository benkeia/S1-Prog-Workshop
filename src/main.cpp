#include <sil/sil.hpp>
#include <cmath>
#include "random.hpp"
#include <vector>
#include <algorithm>

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

void noise(sil::Image &image)
{
    for (int x{0}; x < image.width(); x++)
    {
        for (int y{0}; y < image.height(); y++)
        {
            int noise = random_int(0, 3);
            if (noise == 0)
            {

                image.pixel(x, y).r = random_float(0.f, 1.f);
                image.pixel(x, y).g = random_float(0.f, 1.f);
                image.pixel(x, y).b = random_float(0.f, 1.f);
            }
        }
    }
}

void rotate_image(sil::Image &image)
{
    sil::Image rotated_image{image.height(), image.width()};
    for (int x{0}; x < image.width(); x++)
    {
        for (int y{0}; y < image.height(); y++)
        {
            // 90° horaire (pour anti-horaire : pixel(image.height() - 1 - y, x))
            rotated_image.pixel(y, image.width() - 1 - x) = image.pixel(x, y);
        }
    }

    image = rotated_image;
}

void rgb_split(sil::Image &image)
{
    int offset{30};

    sil::Image splitted_image{image.width(), image.height()};
    for (int x{0}; x < image.width(); x++)
    {
        for (int y{0}; y < image.height(); y++)
        {

            if (x - offset >= 0)
            {
                splitted_image.pixel(x - offset, y).r = image.pixel(x, y).r;
            }
            if (x + offset < image.width())
            {
                splitted_image.pixel(x + offset, y).b = image.pixel(x, y).b;
            }

            splitted_image.pixel(x, y).g = image.pixel(x, y).g;
        }
    }

    image = splitted_image;
}

void brightness(sil::Image &image)
{

    // Plus la valeur s'approche de 0, plus on éclaircit ; plus elle s'éloigne de 1, plus elle s'assombrit.

    float power = 1.5f;

    for (glm::vec3 &color : image.pixels())
    {
        color.r = std::pow(color.r, power);
        color.g = std::pow(color.g, power);
        color.b = std::pow(color.b, power);
    }
}

float getLuminance(const glm::vec3 &color)
{
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

void pixel_sort(sil::Image &image)
{
    std::sort(image.pixels().begin(), image.pixels().end(), [](glm::vec3 const &color1, glm::vec3 const &color2)
              { return getLuminance(color1) < getLuminance(color2); });
}

sil::Image resize(const sil::Image &image, int new_width, int new_height)
{
    sil::Image result{new_width, new_height};

    for (int x{0}; x < new_width; ++x)
    {
        for (int y{0}; y < new_height; ++y)
        {
            result.pixel(x, y) = image.pixel(
                x * image.width() / new_width,
                y * image.height() / new_height);
        }
    }

    return result;
}

sil::Image rearrange(sil::Image source, const sil::Image &target)
{
    source = resize(source, target.width(), target.height());
    pixel_sort(source);

    sil::Image result{target.width(), target.height()};

    // 1. On liste chaque case de destination avec sa luminance cible
    struct Slot
    {
        float luminance;
        glm::vec3 *pixel;
    };

    std::vector<Slot> slots;
    slots.reserve(target.pixels().size());
    for (size_t i{0}; i < target.pixels().size(); ++i)
    {
        slots.push_back({getLuminance(target.pixels()[i]), &result.pixels()[i]});
    }

    // 2. On trie les cases de la plus sombre à la plus claire
    std::sort(slots.begin(), slots.end(), [](const Slot &a, const Slot &b)
              { return a.luminance < b.luminance; });

    // 3. On remplit chaque case triée avec le pixel source correspondant
    for (size_t i{0}; i < slots.size(); ++i)
    {
        *slots[i].pixel = source.pixels()[i];
    }

    return result;
}

void disc(sil::Image &image)
{
    int centerX = image.width() / 2;
    int centerY = image.height() / 2;
    int radius = 100;
    int squaredRadius = radius * radius;

    for (int x{0}; x < image.width(); x++)
    {
        for (int y{0}; y < image.height(); y++)
        {
            int dx = x - centerX;
            int dy = y - centerY;

            if (dx * dx + dy * dy <= squaredRadius)
            {
                image.pixel(x, y) = glm::vec3{1.f};
            }
        }
    }
}

void circle(sil::Image &image, int centerX, int centerY)
{
    int radius = 100;
    int thickness = 5;
    int outerSquared = radius * radius;
    int innerSquared = (radius - thickness) * (radius - thickness);

    for (int x{0}; x < image.width(); x++)
    {
        for (int y{0}; y < image.height(); y++)
        {
            int dx = x - centerX;
            int dy = y - centerY;
            int distSquared = dx * dx + dy * dy;

            if (distSquared <= outerSquared && distSquared >= innerSquared)
            {
                image.pixel(x, y) = glm::vec3{1.f};
            }
        }
    }
}

void circle(sil::Image &image)
{

    circle(image, image.width() / 2, image.height() / 2);
}

void animation(sil::Image &image)
{
    int step = image.width() / 8;

    for (int frameNumber{0}, centerX{0}; centerX <= image.width(); ++frameNumber, centerX += step)
    {
        sil::Image frame{image.width(), image.height()};
        circle(frame, centerX, frame.height() / 2);

        frame.save("output/animation/frame" + std::to_string(frameNumber) + ".png");
    }
}

void rosace(sil::Image &image)
{
    int centerX = image.width() / 2;
    int centerY = image.height() / 2;
    int radius = 100; // Même rayon que dans votre fonction circle
    int nombreCercles = 6;

    circle(image, centerX, centerY);

    float angleStep = (2.f * 3.14f) / nombreCercles;

    for (int i = 0; i < nombreCercles; ++i)
    {
        float angle = i * angleStep;

        int posX = centerX + static_cast<int>(radius * std::cos(angle));
        int posY = centerY + static_cast<int>(radius * std::sin(angle));

        circle(image, posX, posY);
    }
}

void mosaic(sil::Image &image)
{
    int longueurX = 4;
    int longueurY = 4;
    sil::Image mosaic_image{image.width() * longueurX, image.height() * longueurY};

    for (int x{0}; x < image.width() * longueurX; x++)
    {
        for (int y{0}; y < image.height() * longueurY; y++)
        {
            mosaic_image.pixel(x, y) = image.pixel(x % image.width(), y % image.height());
        }
    }
    mosaic_image.save("output/mosaique.png");
}

void mosaic_mirror(sil::Image &image)
{
    int longueurX = 4;
    int longueurY = 4;
    sil::Image mosaic{image.width() * longueurX, image.height() * longueurY};

    for (int x{0}; x < image.width() * longueurX; x++)
    {
        for (int y{0}; y < image.height() * longueurY; y++)

        {

            int srcx = x % image.width();
            int srcy = y % image.height();

            if (x / image.width() % 2 == 1)
            {
                srcx = image.width() - 1 - srcx;
            }
            if (y / image.height() % 2 == 1)
            {
                srcy = image.height() - 1 - srcy;
            }

            mosaic.pixel(x, y) = image.pixel(srcx, srcy);
        }
    }

    mosaic.save("output/mosaic_mirror.png");
}

int main()
{
    sil::Image image{"images/photo.jpg"};
    sil::Image photo{"images/lucas.png"};
    // sil::Image image{500, 500};

    mosaic_mirror(image);
}
