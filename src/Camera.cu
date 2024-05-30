#include "pch.h"
#include "Camera.h"
//#include "Raytracing/Materials/Material.h"

//__device__ Camera::Camera(glm::dvec3 _lookFrom, glm::dvec3 _lookAt, glm::dvec3 _vUp, double vFOV, double _focusDist, double _defocusAngle
//    , double aspectRat, int imgWidth, int samplesPerPx, int maxDepth)
//    : focusDistance(_focusDist), defocusAngle(_defocusAngle), verticalFov(vFOV), perPixelSamples(samplesPerPx),
//    aspectRatio(aspectRat), lookFrom(_lookFrom), lookAt(_lookAt), vUp(_vUp), center(lookFrom), maxRecursionDepth(maxDepth)
//{
//    pixelSampleScale = 1.0f / perPixelSamples;
//
//    imageSize.x = imgWidth;
//    imageSize.y = imageSize.x / aspectRatio;
//    imageSize.y = (imageSize.y < 1) ? 1 : imageSize.y;
//
//    double alpha = Utils::degToRad(verticalFov);
//    double h = tan(alpha / 2);
//    glm::dvec2 viewport(1.0f, 2 * h * focusDistance);
//    viewport.x = viewport.y * (double(imageSize.x) / imageSize.y);
//
//    //Basis vectors calculate
//    w = glm::normalize(lookFrom - lookAt);
//    u = glm::normalize(glm::cross(vUp, w));
//    v = glm::cross(w, u);
//
//    glm::dvec3 viewport_u = viewport.x * u;
//    glm::dvec3 viewport_v = viewport.y * -v;
//
//    pixelDelta_u = viewport_u / (double)imageSize.x;
//    pixelDelta_v = viewport_v / (double)imageSize.y;
//
//    glm::dvec3 viewportUpperLeft = center - (focusDistance * w) - viewport_u / 2.0 - viewport_v / 2.0;
//    pixel00_loc = viewportUpperLeft + 0.5 * (pixelDelta_u + pixelDelta_v);
//
//    //Calculate defocus disk vectors
//    double defocusRadius = focusDistance * tan(Utils::degToRad(defocusAngle / 2.0));
//    defocusDisk_u = u * defocusRadius;
//    defocusDisk_v = v * defocusRadius;
//}

//__device__ glm::dvec3 Camera::rayColor(const Ray& ray, int depth, const Hittable& world, curandState* localRandState) const
//{
//    if (depth <= 0)
//        return glm::dvec3(0, 0, 0);
//
//	hitData data;
//	if (world.hit(ray, Interval(0.001, Utils::infinity), data)) {
//        Ray scateredRay(glm::dvec3(0.0f), glm::dvec3(0.0f));
//        glm::dvec3 attenuation;
//        if (data.mat->scatter(ray, data, attenuation, scateredRay, localRandState))
//            return attenuation * rayColor(scateredRay, depth - 1, world, localRandState);
//        
//        return glm::vec3(0.0f, 0.0f, 0.0f);;
//	}
//
//	glm::dvec3 unitDir = glm::normalize(ray.direction());
//    double a = 0.5 * (unitDir.y + 1.0);
//	//Gradient betweend white and (0.5f, 0.7f, 1.0f) color
//    return (1.0 - a) * glm::dvec3(1.0f, 1.0f, 1.0f) + a * glm::dvec3(0.5, 0.7, 1.0);
//}

//__device__ Ray Camera::getRay(int i, int j, curandState* localRandState) const
//{
//    glm::dvec3 offset = sampleSquare(localRandState);
//    glm::dvec3 pixelCenter = pixel00_loc + (((double)i + offset.x) * pixelDelta_u)
//        + (((double)j + offset.y) * pixelDelta_v);
//
//    glm::dvec3 rayOrigin = (defocusAngle <= 0) ? center : sampleDefocusDisk(localRandState);
//    glm::dvec3 rayDir = pixelCenter - rayOrigin;
//    
//    return Ray(rayOrigin, rayDir);
//}
//
////Camera helper functions
//__device__ glm::dvec3 Camera::sampleSquare(curandState* localRandState) const {
//    return glm::dvec3(Utils::generateRandomNumber(-0.5, 0.5, localRandState), 
//        Utils::generateRandomNumber(-0.5, 0.5, localRandState), 0);
//}
//
//__device__ glm::dvec3 Camera::sampleDefocusDisk(curandState* localRandState) const {
//    glm::dvec2 p = Utils::Vector::randomInUnitDisk(localRandState);
//    return center + (p.x * defocusDisk_u) + (p.y * defocusDisk_v);
//}
//
//__device__ inline double linearToGamma(double linearComponent) {
//    if (linearComponent > 0)
//        return sqrt(linearComponent);
//
//    return 0;
//}
//
//__device__ dataPixels Camera::convertColor(const glm::dvec3& color) {
//    static const Interval intensity(0.000, 0.999);
//
//    glm::dvec3 newColor = glm::dvec3(linearToGamma(color.r), linearToGamma(color.g), linearToGamma(color.b));
//
//    int r = int(256.0f * intensity.clamp(newColor.r));
//    int g = int(256.0f * intensity.clamp(newColor.g));
//    int b = int(256.0f * intensity.clamp(newColor.b));
//
//    dataPixels ret = { r, g, b, 255 };
//    return ret;
//}