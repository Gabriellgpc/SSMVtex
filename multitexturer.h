#ifndef MULTITEXTURER_H
#define MULTITEXTURER_H

#include <stdlib.h>

#include "camera.h"

typedef enum {NONE, COLOR, TEXTURE, POINT} MappingMode;
typedef enum {NORMAL_VERTEX, NORMAL_BARICENTER, AREA, AREA_OCCL} CamAssignMode;
//typedef enum {OBJ, TXT, WRL, OBJCAM, PNG, UNKNOWN} Extension;
typedef enum {MESH, SPLAT} InputMode;

class Multitexturer
{

public:

    Multitexturer();
    ~Multitexturer();

    // Parses the command line and sets the options
    void parseCommandLine(int argc, char *argv[]);

    // Loads the needed 
    void loadInputData();

    // Evaluate how each cameras sees each triangle
    void evaluateCameraRatings();


private:

    void printHelp(); // DUDA: tiene sentido una función void sin parámetros ser definida const?
    
    // Access input data
    void readInputMesh();
    void readCameraFile();
    void readImageList();


    // Different ways to estimate camera weights:
    // Uses the normal of the triangle
    void evaluateNormal(); 
    // Uses the projected area of the triangle
    void evaluateArea();
    // Uses the projected area taking into account occlusions
    void evaluateAreaWithOcclusions(unsigned int _resolution);
    



    Mesh3D mesh_;

    std::vector<Camera> cameras_;
    std::vector<std::string> imageList_;
    unsigned int nCam_;

    // Options - Default values
    CamAssignMode ca_mode_; //  AREA_OCCL
    MappingMode m_mode_; //  TEXTURE
    InputMode in_mode_; //  MESH
    int num_cam_mix_; //  1
    float alpha_; // 0.5
    float beta_; // 1.0
    unsigned int dimension_; // 10000000
    unsigned int imageCacheSize_; // 75

    // File names
    std::string fileNameIn_;
    std::string fileNameCam_;
    std::string fileNameImageList_;
    std::string fileNameOut_;
    std::string fileNameTexOut_;
    std::string fileFaceCam_;

};

#endif // MULTITEXTURER_H
