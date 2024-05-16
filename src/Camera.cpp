#include "pch.h"
#include "Camera.h"

Camera::Camera(double aspectRat, int imgWidth, int samplesPerPx, int maxDepth)
{
    perPixelSamples = samplesPerPx;
    pixelSampleScale = 1.0f / perPixelSamples;

    aspectRatio = aspectRat;
    imageSize.x = imgWidth;
    imageSize.y = imageSize.x / aspectRatio;
    imageSize.y = (imageSize.y < 1) ? 1 : imageSize.y;

    //Camera
    center = glm::vec3(0.0f, 0.0f, 0.0f);

    float focal_length = 1.0f; //Distance between viewport and camera center point
    glm::vec2 viewport(1.0f, 2.0f);
    viewport.x = viewport.y * (double(imageSize.x) / imageSize.y);

    glm::vec3 viewport_u = glm::vec3(viewport.x, 0.0f, 0.0f);
    glm::vec3 viewport_v = glm::vec3(0.0f, -viewport.y, 0.0f);

    pixelDelta_u = viewport_u / (float)imageSize.x;
    pixelDelta_v = viewport_v / (float)imageSize.y;

    glm::vec3 viewportUpperLeft = center - glm::vec3(0.0f, 0.0f, focal_length) -
        viewport_u / 2.0f - viewport_v / 2.0f;

    pixel00_loc = viewportUpperLeft + 0.5f * (pixelDelta_u + pixelDelta_v);

    uint32_t numOfChannels = 4; //RGBA
    uint32_t dataSize = imageSize.x * imageSize.y * numOfChannels;
    data.reserve(dataSize);
}

glm::vec3 Camera::rayColor(const Ray& ray, int depth, const Hittable& world) const
{
    if (depth <= 0)
        return glm::vec3(0, 0, 0);

	hitData data;
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
	if (world.hit(ray, Interval(0.001, Utils::infinity), data)) {
        glm::vec3 direction = data.normal + Utils::randomVectorOnHemisphere(data.normal);
        color = 0.5f * rayColor(Ray(data.p, direction), depth - 1, world);
        return glm::vec4(color, 1.0f);
	}

	glm::vec3 unitDir = glm::normalize(ray.direction());
    float a = 0.5f * (unitDir.y + 1.0f);
	//Gradient betweend white and (0.5f, 0.7f, 1.0f) color
    color = (1.0f - a) * glm::vec3(1.0f, 1.0f, 0.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f);
    return glm::vec4(color, 1.0f);
}

void Camera::render(const Hittable& world)
{
    data.clear();
    glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
    for (int j = 0; j < imageSize.y; j++) {
        std::clog << "\rScanlines remaining: " << (imageSize.y - j) << ' ' << std::flush;
        for (int i = 0; i < imageSize.x; i++) {
            pixelColor = glm::vec3(0.0f, 0.0f, 0.0f);
            for (int sampleIdx = 0; sampleIdx < perPixelSamples; sampleIdx++) {
                Ray r = getRay(i, j);
                pixelColor += rayColor(r, maxRecursionDepth, world);
            }

            data.emplace_back(convertColor((float)pixelSampleScale * pixelColor));
        }
    }
    std::clog << "\ndone\n";
}

Ray Camera::getRay(int i, int j) const
{
    glm::vec3 offset = sampleSquare();
    glm::vec3 pixelCenter = pixel00_loc + (((float)i + offset.x) * pixelDelta_u)
        + (((float)j + offset.y) * pixelDelta_v);
    glm::vec3 rayDir = pixelCenter - center;
    
    return Ray(center, rayDir);
}

//Camera helper functions
glm::vec3 Camera::sampleSquare() const {
    return glm::vec3(Utils::generateRandomNumber(0.0, 1.0) - 0.5, Utils::generateRandomNumber(0.0, 1.0) - 0.5, 0);
}

inline double linearToGamma(double linearComponent)
{
    if (linearComponent > 0)
        return sqrt(linearComponent);

    return 0;
}

dataPixels Camera::convertColor(const glm::vec3& color) {
    static const Interval intensity(0.000, 0.999);

    glm::vec3 newColor = glm::vec3(linearToGamma(color.r), linearToGamma(color.g), linearToGamma(color.b));

    int r = int(256.0f * intensity.clamp(newColor.r));
    int g = int(256.0f * intensity.clamp(newColor.g));
    int b = int(256.0f * intensity.clamp(newColor.b));

    dataPixels ret = { r, g, b, 255 };
    return ret;
}