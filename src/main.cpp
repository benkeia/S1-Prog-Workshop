#include <sil/sil.hpp>
#include <cmath>
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <complex>
#include <limits>
#include <glm/gtx/matrix_transform_2d.hpp>

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

void gradient(sil::Image &image, glm::vec3 startColor, glm::vec3 endColor)
{
    for (int x{0}; x < image.width(); x++)
    {
        float percentage = static_cast<float>(x) / static_cast<float>(image.width() - 1);
        for (int y{0}; y < image.height(); y++)
        {
            image.pixel(x, y) = glm::mix(startColor, endColor, percentage);
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

void glitch(sil::Image &image)
{

    int glitch_amount = 60;
    for (int i = 0; i < glitch_amount; i++)
    {
        int glitch_height = random_int(5, 10);
        int glitch_width = random_int(20, 40);

        int x1 = random_int(0, image.width() - glitch_width);
        int x2 = random_int(0, image.width() - glitch_width);
        int y1 = random_int(0, image.height() - glitch_height);
        int y2 = random_int(0, image.height() - glitch_height);

        for (int x{0}; x < image.width(); x++)
        {
            for (int y{0}; y < image.height(); y++)
            {

                if (x >= x1 && x < x1 + glitch_width &&
                    y >= y1 && y < y1 + glitch_height)
                {

                    int dx = x - x1;
                    int dy = y - y1;

                    std::swap(image.pixel(x, y), image.pixel(dx, dy));
                }
            }
        }
    }
}

void fractal(sil::Image &image)
{
    int maxIterations{200};

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            float real = 4.f * x / image.width() - 2.f;
            float imaginary = 4.f * y / image.height() - 2.f;
            std::complex<float> c{real, imaginary};
            std::complex<float> z{0.f, 0.f};

            int iterations{0};
            while (std::abs(z) <= 2.f && iterations < maxIterations)
            {
                z = z * z + c;
                ++iterations;
            }

            float gray = std::sqrt(iterations / static_cast<float>(maxIterations));
            image.pixel(x, y) = glm::vec3{gray};
        }
    }
}

void srgb_to_linear(glm::vec3 &color)
{
    for (int i{0}; i < 3; ++i)
    {
        float &channel = color[i];
        if (channel <= 0.04045f)
        {
            channel /= 12.92f;
        }
        else
        {
            channel = std::pow((channel + 0.055f) / 1.055f, 2.4f);
        }
    }
}

glm::vec3 linear_to_srgb(glm::vec3 color)
{
    for (int i{0}; i < 3; ++i)
    {
        float &channel = color[i];
        if (channel <= 0.0031308f)
        {
            channel *= 12.92f;
        }
        else
        {
            channel = 1.055f * std::pow(channel, 1.f / 2.4f) - 0.055f;
        }
    }

    return color;
}

glm::vec3 linear_rgb_to_oklab(glm::vec3 color)
{
    float l = 0.4122214708f * color.r + 0.5363325363f * color.g + 0.0514459929f * color.b;
    float m = 0.2119034982f * color.r + 0.6806995451f * color.g + 0.1073969566f * color.b;
    float s = 0.0883024619f * color.r + 0.2817188376f * color.g + 0.6299787005f * color.b;

    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);

    return glm::vec3{
        0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s,
        1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s,
        0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s};
}

glm::vec3 oklab_to_linear_rgb(glm::vec3 color)
{
    float l = color.x + 0.3963377774f * color.y + 0.2158037573f * color.z;
    float m = color.x - 0.1055613458f * color.y - 0.0638541728f * color.z;
    float s = color.x - 0.0894841775f * color.y - 1.2914855480f * color.z;

    l = l * l * l;
    m = m * m * m;
    s = s * s * s;

    return glm::vec3{
        4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s};
}

void gradient_oklab(sil::Image &image, glm::vec3 startColor, glm::vec3 endColor)
{
    srgb_to_linear(startColor);
    srgb_to_linear(endColor);
    startColor = linear_rgb_to_oklab(startColor);
    endColor = linear_rgb_to_oklab(endColor);

    for (int x{0}; x < image.width(); ++x)
    {
        float percentage = static_cast<float>(x) / static_cast<float>(image.width() - 1);
        for (int y{0}; y < image.height(); ++y)
        {
            glm::vec3 color = glm::mix(startColor, endColor, percentage);
            color = oklab_to_linear_rgb(color);
            color = linear_to_srgb(color);
            image.pixel(x, y) = color;
        }
    }
}

void k_means(sil::Image &image, int means = 3)
{
    if (means <= 0 || image.width() == 0 || image.height() == 0)
    {
        return;
    }

    std::vector<glm::vec3> colors(means);
    std::vector<int> labels(image.pixels().size());

    for (int i{0}; i < means; ++i)
    {
        int random_colorX = random_int(0, image.width() - 1);
        int random_colorY = random_int(0, image.height() - 1);

        colors[i] = image.pixel(random_colorX, random_colorY);
    }

    for (int k{0}; k < 10; ++k)
    {
        std::vector<glm::vec3> sums(means, glm::vec3{0.f});
        std::vector<int> counts(means, 0);

        for (int x{0}; x < image.width(); ++x)
        {
            for (int y{0}; y < image.height(); ++y)
            {
                float smallestDistance = std::numeric_limits<float>::max();
                int closestColor = 0;

                for (int i{0}; i < means; ++i)
                {
                    float distance = glm::distance(image.pixel(x, y), colors[i]);
                    if (distance < smallestDistance)
                    {
                        smallestDistance = distance;
                        closestColor = i;
                    }
                }

                int index = y * image.width() + x;
                labels[index] = closestColor;

                sums[closestColor] += image.pixel(x, y);
                counts[closestColor]++;
            }
        }

        for (int i{0}; i < means; ++i)
        {
            if (counts[i] > 0)
            {
                colors[i] = sums[i] / static_cast<float>(counts[i]);
            }
        }
    }

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            float smallestDistance = std::numeric_limits<float>::max();
            int closestColor = 0;

            for (int i{0}; i < means; ++i)
            {
                float distance = glm::distance(image.pixel(x, y), colors[i]);
                if (distance < smallestDistance)
                {
                    smallestDistance = distance;
                    closestColor = i;
                }
            }

            image.pixel(x, y) = colors[closestColor];
        }
    }
}

void kuwahara(sil::Image &image, int radius = 4)
{
    sil::Image copy = image;

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            int quadrants[4][4] = {
                {x - radius, x, y - radius, y},
                {x, x + radius, y - radius, y},
                {x - radius, x, y, y + radius},
                {x, x + radius, y, y + radius},
            };

            float min_variance = std::numeric_limits<float>::max();
            glm::vec3 best_color{0.f};

            for (int q{0}; q < 4; ++q)
            {
                glm::vec3 sum{0.f};
                int count{0};

                for (int qx{quadrants[q][0]}; qx <= quadrants[q][1]; ++qx)
                {
                    for (int qy{quadrants[q][2]}; qy <= quadrants[q][3]; ++qy)
                    {
                        if (qx < 0 || qx >= image.width() || qy < 0 || qy >= image.height())
                        {
                            continue;
                        }
                        sum += copy.pixel(qx, qy);
                        count++;
                    }
                }

                glm::vec3 mean = sum / static_cast<float>(count);
                float variance{0.f};

                for (int qx{quadrants[q][0]}; qx <= quadrants[q][1]; ++qx)
                {
                    for (int qy{quadrants[q][2]}; qy <= quadrants[q][3]; ++qy)
                    {
                        if (qx < 0 || qx >= image.width() || qy < 0 || qy >= image.height())
                        {
                            continue;
                        }
                        glm::vec3 diff = copy.pixel(qx, qy) - mean;
                        variance += diff.r * diff.r + diff.g * diff.g + diff.b * diff.b;
                    }
                }

                variance /= static_cast<float>(count);

                if (variance < min_variance)
                {
                    min_variance = variance;
                    best_color = mean;
                }
            }

            image.pixel(x, y) = best_color;
        }
    }
}

void normalize_histogram(sil::Image &image)
{
    if (image.pixels().empty())
    {
        return;
    }

    float min_b = std::numeric_limits<float>::max();
    float max_b = 0.f;

    for (const glm::vec3 &color : image.pixels())
    {
        float b = getLuminance(color);
        min_b = std::min(min_b, b);
        max_b = std::max(max_b, b);
    }

    if (max_b <= min_b)
    {
        return;
    }

    for (glm::vec3 &color : image.pixels())
    {
        color = (color - min_b) / (max_b - min_b);
    }
}

void dither(sil::Image &image)
{
    const float bayer[4][4] = {
        {0.f, 8.f, 2.f, 10.f},
        {12.f, 4.f, 14.f, 6.f},
        {3.f, 11.f, 1.f, 9.f},
        {15.f, 7.f, 13.f, 5.f}};

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            float luminance = getLuminance(image.pixel(x, y));
            float threshold = bayer[y % 4][x % 4] / 16.f;

            if (luminance > threshold)
            {
                image.pixel(x, y) = glm::vec3{1.f};
            }
            else
            {
                image.pixel(x, y) = glm::vec3{0.f};
            }
        }
    }
}

glm::vec2 rotated(glm::vec2 point, glm::vec2 center_of_rotation, float angle)
{
    return glm::vec2{glm::rotate(glm::mat3{1.f}, angle) * glm::vec3{point - center_of_rotation, 0.f}} + center_of_rotation;
}

void vortex(sil::Image &image)
{
    sil::Image copy = image;
    glm::vec2 center{image.width() / 2.f, image.height() / 2.f};

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            glm::vec2 point{x, y};
            float distance = glm::distance(point, center);
            float angle = distance * 0.03f;
            glm::vec2 source = rotated(point, center, angle);

            int src_x = static_cast<int>(source.x);
            int src_y = static_cast<int>(source.y);

            if (src_x >= 0 && src_x < image.width() && src_y >= 0 && src_y < image.height())
            {
                image.pixel(x, y) = copy.pixel(src_x, src_y);
            }
            else
            {
                image.pixel(x, y) = glm::vec3{0.f};
            }
        }
    }
}

void convolution(sil::Image &image, const std::vector<std::vector<float>> &kernel)
{
    sil::Image copy = image;
    int kh = static_cast<int>(kernel.size());
    int kw = static_cast<int>(kernel[0].size());
    int half_y = kh / 2;
    int half_x = kw / 2;

    for (int x{0}; x < image.width(); ++x)
    {
        for (int y{0}; y < image.height(); ++y)
        {
            glm::vec3 sum{0.f};

            for (int ky{0}; ky < kh; ++ky)
            {
                for (int kx{0}; kx < kw; ++kx)
                {
                    int nx = x + kx - half_x;
                    int ny = y + ky - half_y;

                    if (nx >= 0 && nx < image.width() && ny >= 0 && ny < image.height())
                    {
                        sum += copy.pixel(nx, ny) * kernel[ky][kx];
                    }
                }
            }

            image.pixel(x, y) = glm::clamp(sum, glm::vec3{0.f}, glm::vec3{1.f});
        }
    }
}

void box_blur(sil::Image &image, int size = 5)
{
    float weight = 1.f / static_cast<float>(size * size);
    std::vector<std::vector<float>> kernel(size, std::vector<float>(size, weight));
    convolution(image, kernel);
}

int main()
{
    {
        sil::Image image{"images/photo.jpg"};
        box_blur(image, 5);
        image.save("output/convolution/box_blur.png");
    }
    {
        sil::Image image{"images/photo.jpg"};
        std::vector<std::vector<float>> emboss = {
            {-2.f, -1.f, 0.f},
            {-1.f,  1.f, 1.f},
            { 0.f,  1.f, 2.f},
        };
        convolution(image, emboss);
        image.save("output/convolution/emboss.png");
    }
    {
        sil::Image image{"images/photo.jpg"};
        std::vector<std::vector<float>> outline = {
            {-1.f, -1.f, -1.f},
            {-1.f,  8.f, -1.f},
            {-1.f, -1.f, -1.f},
        };
        convolution(image, outline);
        image.save("output/convolution/outline.png");
    }
    {
        sil::Image image{"images/photo.jpg"};
        std::vector<std::vector<float>> sharpen = {
            { 0.f, -1.f,  0.f},
            {-1.f,  5.f, -1.f},
            { 0.f, -1.f,  0.f},
        };
        convolution(image, sharpen);
        image.save("output/convolution/sharpen.png");
    }
}
