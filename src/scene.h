#pragma once

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "utilities.h"
#include "sceneStructs.h"
#include "texture.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "gltf-loader.h"

using namespace std;
namespace fs = std::experimental::filesystem;

class Scene {
private:
    ifstream fp_in;
    int loadMaterial(string materialid);
    int loadGeom(string objectid);
    int loadCamera();

	void updateMaxMin(gvec3* maxVal, gvec3* minVal, gvec3 val);

public:
    Scene(string filename);
    ~Scene();

    Geom_v geoms;
	Triangle_v triangles;
    Material_v materials;
    RenderState state;
	Texture_v textures = Texture_v();

	Geom_v readObjFromMesh(string filename, int materialid, gmat4 transform = MAT4I);//default to the identity matrix

	/**
	Creates a Material object from the read-in material
	*/
	Material materialFromObj(tinyobj::material_t mat);
	/**
	Constructs a Geom object with all the relevant bounding-box parameters 
	*/
	Geom Scene::geomFromShape(tinyobj::shape_t shape, tinyobj::attrib_t attrib,
		std::vector<tinyobj::material_t> materials, int materialid,
		gmat4 transform);
	/**
	Pulls out the relevant information to make a triangle from the given face index
	*/
	Triangle triangleFromObjIndex(int index, vector<tinyobj::index_t> indices, vector<int> material_ids,
		tinyobj::attrib_t attrib,
		int defaultMaterialId, gmat4 transform);
	/**
	Constructs a set of Geom objects from a Gltf file
	*/
	Geom_v readGltfFromMesh(string filename, int materialid, gmat4 transform);
	/**
	Turns the mesh into a geom that refers to a list of triangles
	*/
	Geom geomFromGltfMesh(example::Mesh<float> mesh, int materialid, gmat4 transform);
	/**
	Pulls out the relevant information to make a triangle from the given face index
	*/
	Triangle triangleFromGltfIndex(example::Mesh<float> mesh, unsigned int i,
		int materialid, gmat4 transform, gmat4 normTransform, gvec3* maxVals, gvec3* minVals,
		bool hasTexture
	);
	Material materialFromTexture(uint8_t texturePresenceMask, int textureIndex, int copyMaterialId);
};
