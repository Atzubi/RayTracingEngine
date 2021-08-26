//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_HITSHADER_H
#define RAYTRACECORE_HITSHADER_H

//#include "RayEngine.h"

class BasicHitShader : public HitShader{
public:
    BasicHitShader(){

    }
    BasicHitShader(const BasicHitShader& copy){

    }

    Shader* clone() override{
        return new BasicHitShader(*this);
    }

    ShaderOutput shade(int id, PipelineInfo *pipelineInfo, IntersectionInfo *shaderInput, void* dataInput, RayGeneratorOutput *newRays) override{
        Vector3D Ka = {1, 1, 1};
        Vector3D Kd = {1, 1, 1};
        Vector3D Ks = {1, 1, 1};
        uint8_t pix[3];
        unsigned char *image = shaderInput->material->map_Kd.image;

        double_t diffuse = 1, specular = 0.5, exponent = 8, ambient = 0.1;

        if (image == nullptr) {
            if (shaderInput->material->illum == 2) {
                exponent = shaderInput->material->Ns;
                Ka = shaderInput->material->Ka;
                Kd = shaderInput->material->Kd;
                Ks = shaderInput->material->Ks;
            }
            pix[0] = 255;
            pix[1] = 255;
            pix[2] = 255;
        } else {
            int w = shaderInput->material->map_Kd.w;
            int h = shaderInput->material->map_Kd.h;
            double x = fmod(shaderInput->texture.x, 1.0);
            double y = -fmod(shaderInput->texture.y, 1.0);
            if (x < 0) x = 1 + x;
            if (y < 0) y = 1 + y;

            uint64_t pixelCoordinate = ((uint64_t) ((w - 1) * x)) + w * ((uint64_t) ((h - 1) * y));

            pix[0] = image[pixelCoordinate * 3];
            pix[1] = image[pixelCoordinate * 3 + 1];
            pix[2] = image[pixelCoordinate * 3 + 2];

            Ka = shaderInput->material->Ka;
            Kd = shaderInput->material->Kd;
            Ks = shaderInput->material->Ks;
        }

        Vector3D light{};
        light.x = -0.707107;
        light.y = 0.707107;
        light.z = 0;

        ShaderOutput shaderOutput;

        if (shaderInput->distance == std::numeric_limits<double_t>::max()) return shaderOutput;

        Vector3D n{}, l{}, v{}, r{};
        double_t nl;

        l.x = light.x;
        l.y = light.y;
        l.z = light.z;

        n = shaderInput->normal;

        v.x = -1.0 * (shaderInput->rayOrigin.x - pipelineInfo->cameraPosition.x);
        v.y = -1.0 * (shaderInput->rayOrigin.y - pipelineInfo->cameraPosition.y);
        v.z = -1.0 * (shaderInput->rayOrigin.z - pipelineInfo->cameraPosition.z);


        double_t length = sqrt(l.x * l.x + l.y * l.y + l.z * l.z);
        l.x /= length;
        l.y /= length;
        l.z /= length;

        length = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        n.x /= length;
        n.y /= length;
        n.z /= length;

        length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        v.x /= length;
        v.y /= length;
        v.z /= length;

        nl = fmax(n.x * l.x + n.y * l.y + n.z * l.z, 0);

        r.x = (2 * nl * n.x) - l.x;
        r.y = (2 * nl * n.y) - l.y;
        r.z = (2 * nl * n.z) - l.z;

        length = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
        r.x /= length;
        r.y /= length;
        r.z /= length;

        double_t dot = fmax(v.x * r.x + v.y * r.y + v.z * r.z, 0);

        shaderOutput.color[0] = (uint8_t) fmin((ambient * Ka.x + diffuse * nl * Kd.x + specular * powf(dot, exponent) * Ks.x) * pix[0],
                                  255);
        shaderOutput.color[1] = (uint8_t) fmin((ambient * Ka.y + diffuse * nl * Kd.y + specular * powf(dot, exponent) * Ks.y) * pix[1],
                                  255);
        shaderOutput.color[2] = (uint8_t) fmin((ambient * Ka.z + diffuse * nl * Kd.z + specular * powf(dot, exponent) * Ks.z) * pix[2],
                                  255);

        return shaderOutput;
    }

    void *getAssociatedData() {
        return nullptr;
    }
};

#endif //RAYTRACECORE_HITSHADER_H
