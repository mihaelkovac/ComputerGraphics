#include "bspline.h"

#include <cstdio>

BSpline BSpline::load(const char* pathFile, float tStep)
{
    BSpline result(tStep);
    load(result, pathFile);
    return result;
}

glm::vec3 evalPoint(float t, int i, const std::vector<BSplineVertex>& points) noexcept
{
    glm::mat4x3 R{points[i - 1].pos, points[i].pos, points[i + 1].pos, points[i + 2].pos};
    float t_2 = t*t;
    glm::vec4 T{ t_2 * t, t_2, t, 1};

    return R * BSpline::B * T;
}
 
void BSpline::load(BSpline& obj, const char* pathFile)
{
    FILE* f = fopen(pathFile, "r");

    if(!f)
    {
        fprintf(stderr, "Error while trying to open path file!");
        abort();
    }

    int read = 0;
    do
    {
        auto& point = obj.points.emplace_back();
        read = fscanf(f, "%f %f %f\n", &point.pos.x, &point.pos.y, &point.pos.z);
        
    } while(read == 3 && !feof(f));


    for(size_t i = 1; i < obj.points.size() - 2; ++i)
    {
        for(float t = 0.f; t <= 1.0f; t+=obj.tStep_)
        {
            auto& pathPoint = obj.path.emplace_back();
            pathPoint = evalPoint(t, i, obj.points);
            // printf("%f %f %f\n", pathPoint.x, pathPoint.y, pathPoint.z);
        }
    }


}

// void BSpline::load(BSpline& obj, const char* pathFile)
// {
//     FILE* f = fopen(pathFile, "r");

//     if(!f)
//     {
//         fprintf(stderr, "Error while trying to open path file!");
//         abort();
//     }

//     int read = 0;
//     do
//     {
//         auto& point = obj.points.emplace_back();
//         read = fscanf(f, "%f %f %f\n", &point.posx, &point.pos.y, &point.pos.z);
        
//     } while(read == 3 && !feof(f));

// }

size_t pointsInSegment(float tStep)
{
    return static_cast<size_t>(1.0f / tStep);
}

glm::vec3 BSpline::point(float t, int i) const noexcept
{
    // glm::mat4x3 R{points[i - 1], points[i], points[i + 1], points[i + 2]};
    // float t_2 = t*t;
    // glm::vec4 T{ t_2 * t, t_2, t, 1};

    // return R * B * T;

    return path[(i - 1) * pointsInSegment(tStep_) + (t / tStep_)];
}

glm::vec3 BSpline::tangent(float t, int i) const noexcept
{
    glm::mat4x3 R{points[i - 1].pos, points[i].pos, points[i + 1].pos, points[i + 2].pos};
    glm::vec3 d_T{t*t, t, 1};
    return R * d_B * d_T;
}

glm::vec3 BSpline::bitangent(float t, int i) const noexcept
{
    glm::mat4x3 R{points[i - 1].pos, points[i].pos, points[i + 1].pos, points[i + 2].pos};
    glm::vec2 dd_T{t, 1};
    return R * dd_B * dd_T;
}

BSpline::Animation BSpline::animate() const noexcept
{
    return Animation{this};
}

void BSpline::Animation::setSpline(const BSpline* spline) noexcept
{
    spline_ = spline;
}

BSpline::Animation::AnimationTuple BSpline::Animation::update() noexcept
{

    t += spline_->tStep_;
    if(t >= 1.0f)
    {
        t = 0.f;
        ++segment;

        if(segment >= spline_->points.size() - 2)
        {
            segment = 1;
        }

    }

    pos    = spline_->point(t, segment);
    tang   = spline_->tangent(t, segment);
    bitang = spline_->bitangent(t, segment); 

    return {pos, tang, bitang};
}