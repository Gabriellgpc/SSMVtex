/* 
 *  Copyright (c) 2017  Rafael Pagés
 *
 *  This file is part of SSMVtex
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef MULTITEXTURER_H
#define MULTITEXTURER_H

#include <stdlib.h>
#include <map>
#include <list>

// #include <opencv/cv.h>
// #include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "camera.h"
#include "image.h"
#include "unwrapper.h"
#include "packer.h"

typedef enum {TEXTURE, VERTEX, FLAT} MappingMode;
typedef enum {NORMAL_VERTEX, NORMAL_BARICENTER, AREA, AREA_OCCL} CamAssignMode;
typedef enum {LIGHT, SHADOW, DARK} VtxMode;
typedef enum {MESH, SPLAT} InputMode;
typedef enum {VRML, OBJ, PLY} OutputExtension;

class Multitexturer {

public:

    Multitexturer();
    virtual ~Multitexturer();

    // Parses the command line and sets the options
    void parseCommandLine(int argc, char *argv[]);

    // Loads the needed 
    void loadInputData();

    // Evaluate how each cameras sees each triangle
    void evaluateCameraRatings();

    // Turns a 3D mesh into a set of 2D charts that
    // are stored in the std::vector charts_
    void meshUnwrap();

    // Rearranges the 2D charts stored in charts_
    // so they are efficently packed
    void chartPacking();

    // Creates a texture atlas and colors it
    void chartColoring();

    // Colors the mesh using a color-per-vertex approach
    void colorVertices(std::vector<Color>& _meshcolors);

    // Exports a textured 3D model using the selected format
    void exportTexturedModel();

    // Exports a model colored using a color per vertex approach
    void exportColorPerVertexModel(const std::vector<Color>& _vertexcolors);

    // Returns the mapping mode
    MappingMode getMappingMode() const;

    // TEST chart exporter: the packed charts are exported
    // as a flat 3D mesh in OBJ format
    void exportOBJcharts(const std::string& _fileName);




private:

    void printHelp() const;
    
    // Access input data
    void readInputMesh();
    void readImageList();   
    // Access camera data
    void loadCameraInfo();
    void readCameraFile();
    void readBundlerFile(); // Only for the camera information


    // Geometry stuff
    //
    // According to camera c, is vertex v eclipsed by triangle t ?
    bool isVertexEclipsed(int _v, int _t, int _c) const;
    bool isVertexEclipsed(const Vector3f& _v, int _t, int _c) const;
    // Line-triangle intersection (returns false if not intersecting)
    // Line: _a, _b
    // Triangle: _v0, _v1, v2
    bool lineTriangleIntersection(const Vector3f& _a, const Vector3f& _b, const Vector3f& _v0, const Vector3f& _v1, const Vector3f& _v2, Vector3f& _intersection) const;
    // Samples x with respect to the given resolution
    unsigned int findPosGrid (float _x, float _min, float _max, unsigned int _resolution) const;
    // Checks if the point p is included in the triangle defined by vertices a, b and c
    bool isPinsideTri (const Vector2f& _p, const Vector2f& _a, const Vector2f& _b, const Vector2f& _c) const; 
    // This function returns a 2D Vector containing the u,v, image coordinates
    // of a point (_p) in relation to a triangle (_a, _b, _c)
    Vector2f uvPtri (const Vector2f& _p, const Vector2f& _a, const Vector2f& _b, const Vector2f& _c) const;
    // Returns the intersecting point of two lines defined by point _a and vector _va and _b and _vb
    Vector2f lineIntersect(const Vector2f& _a, const Vector2f& _va, const Vector2f& _b, const Vector2f& _vb) const;
 


    // Different ways to estimate camera weights:
    // They all fill the vector tri_ratings_ of each camera
    // 
    // Uses the normal of the triangle
    void evaluateNormal(std::vector<std::vector<float> >& _cam_tri_ratings);
    // Uses the projected area of the triangle
    void evaluateArea(std::vector<std::vector<float> >& _cam_tri_ratings);
    // Uses the projected area taking into account occlusions
    void evaluateAreaWithOcclusions(std::vector<std::vector<float> >& _cam_tri_ratings);
    // Smooths the values estimated so transitions are seamless
    void smoothRatings(std::vector<std::list<int> > & _tri2tri, std::vector<std::vector<float> >& _cam_tri_ratings);
    // Weights the normals with respect to a function with
    // curvature beta_ and cutoff value alpha_
    void evaluateWeightNormal(std::vector<std::vector<float> >& _cam_tri_ratings);
    // Finds a face in an image and increases the camera ratings
    // for that camera in the corresponding facial triangles
    void improveFaceRatings(std::vector<std::vector<float> >& _cam_tri_ratings);


    // Finds a camera in the list and returns its position
    // if the camera is not found, -1 is returned
    int findCameraInList(const std::string& _fileName) const;

    // Loads the selected image to the cache
    void loadImageToCache(const std::string& _fileName);

    // Finds a face in the determined image
    // Returns true if found or false if not
    // Stores the corners of the box where the face is contained
    bool findFaceInImage(float& _face_min_x, float& _face_max_x, float& _face_min_y, float& _face_max_y) const;

    // Chart coloring functions:
    // This functions calculates the output image size
    void calculateImageSize();

    // Fills arrays _pix_frontier and _pix_triangle, which represent the final image
    void rasterizeTriangles(ArrayXXi& _pix_frontier, ArrayXXi& _pix_triangle);

    // Finds the borders of every chart and fills in the corresponding arrays
    void findChartBorders(Chart& _chart, ArrayXXi& _pix_frontier, ArrayXXi& _pix_triangle);

    // Subdivides the triangles using a mid-point subdivission approach
    // so the photoconsistency check is more accurate
    void subdivideCharts(unsigned int _iterations = 1);

    // Updates the number of vtx and tri after a subdivision stage
    void updateNumbers();

    // Checks if there is an occlusion not produced by the geometry
    // and solves it.
    void checkPhotoconsistency();
    void checkPhotoconsistencyPerPhoto();

    // Performs the multi-texturing and returns a texture image
    Image colorTextureAtlas(const ArrayXXi& _pix_triangle);

    // This method colors each path on a different flat color... just for illustration purposes...
    Image colorFlatCharts(const ArrayXXi& _pix_triangle);

    // This method colors each vertex depending on how it seen by a certain camera... again, just for illustration purposes
    void exportCamColorMesh(unsigned int _camIndex);

    // Dilates the charts by inpainting the background using OpenCV
    void dilateAtlasCV(const ArrayXXi& _pix_triangle, Image& _image) const;
    void dilateAtlasCV2(const ArrayXXi& _pix_triangle, Image& _image) const;
    
    // Dilate the charts by extending their border color
    void dilateAtlas(ArrayXXi& _pix_frontier, Image& _image, unsigned int _nIter) const;



    // Input 3D mesh
    Mesh3D mesh_;
    Mesh3D origMesh_;

    unsigned int nVtx_, nTri_;

    // Input files
    std::vector<Camera> cameras_;
    std::vector<std::string> imageList_;
    unsigned int nCam_;

    // Images are stored in a cache
    // so there are no memory issues
    std::map<std::string, Image> imageCache_;

    // Group of 2D charts created by unwrapping the 3D mesh
    std::vector<Chart> charts_;

    // Dimensions of the final image:
    // Real-scale floating point coordinates
    float realWidth_, realHeight_;
    // Image dimensions
    unsigned int imWidth_, imHeight_;

    // Options - Default values
    CamAssignMode ca_mode_; //  AREA_OCCL
    MappingMode m_mode_; //  TEXTURE
    InputMode in_mode_; //  MESH
    OutputExtension out_extension_; // VRML
    int num_cam_mix_; //  1
    float alpha_; // 0.5
    float beta_; // 1.0
    unsigned int dimension_; // 10,000,000
    unsigned int imageCacheSize_; // 75
    bool highlightOcclusions_; // false
    bool powerOfTwoImSize_; // false
    bool photoconsistency_; // true

    // File names
    std::string fileNameIn_;
    std::string fileNameCam_;
    std::string fileNameImageList_;
    std::string fileNameOut_;
    std::string fileNameTexOut_;
    std::string fileFaceCam_;

    // Out timing file
    std::ofstream times_;

};

#endif // MULTITEXTURER_H
