#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )
#pragma comment( lib, "SDL2_image.lib" )

/******************************************************************************
*                                                                             *
*                              Included Header Files                          *
*                                                                             *
******************************************************************************/
#include "Geometry.h"
#include <string>
#include <iostream>
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <SDL\SDL_image.h>
#include "tiny_obj_loader.h"

/******************************************************************************
*                                                                             *
*                           Macros and Static Variables                       *
*                                                                             *
******************************************************************************/
#define ARRAY_SIZE(a) sizeof(a) / sizeof(*a)

Shader* Geometry::shader = NULL;
const char* Geometry::ICO_OBJ = "res/meshes/icosohedron.obj";
const char* Geometry::TORUS_OBJ = "res/meshes/torus.obj";
const glm::vec3 Geometry::COLORS[] = { { +1.0f, +0.0f, +0.0f },   // Red.
                                       { +0.0f, +1.0f, +0.0f },   // Green.
                                       { +0.0f, +0.0f, +1.0f },   // Blue.
                                       { +1.0f, +1.0f, +0.0f },   // Yellow.
                                       { +1.0f, +0.0f, +1.0f },   // Magenta.
                                       { +0.0f, +1.0f, +1.0f } }; // Cyan.

/******************************************************************************
*                                                                             *
*                       Mesh::Mesh  (constructor - overloaded)                *
*                                                                             *
*******************************************************************************
* PARAMETERS (1)                                                              *
*  void                                                                       *
*                                                                             *
* PARAMETERS (2)                                                              *
*  @param rhs                                                                 *
*           An instance of Mesh to be copied to this mesh.                    *
*                                                                             *
*******************************************************************************
* RETURNS (1)                                                                 *
*  void                                                                       *
*                                                                             *
* RETURNS (2)                                                                 *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION (1)                                                             *
*  Default constructor which initializes all members of Mesh to 0 (NULL)      *
*  except for members with default values (numBuffers and drawMode).          *
*                                                                             *
* DESCRIPTION (2)                                                             *
*  Copy constructor which copis all of the data from one Mesh to a new one.   *
*                                                                             *
*******************************************************************************/
Mesh::Mesh() :
    /* Constructor Initialization. */
    vertices(0), numVertices(0),
    indices(0), numIndices(0),
	textureID(-1), changed(false),
	transform_MTW(glm::mat4()), translate_M(glm::mat4()),
	scale_M(glm::mat4()), rotate_M(glm::mat4()), revolve_M(glm::mat4()), 
    numBuffers(DEFAULT_NUM_BUFFERS), bufferIDs(0), vertexArrayID(0),
	drawMode(DEFAULT_DRAW_MODE), solid(DEFAULT_SOLID)
{
	/* Empty. */
}
Mesh::Mesh(const Mesh& rhs) :
    /* Constructor Initialization. */
	numVertices(rhs.getNumVertices()),
	numIndices(rhs.getNumIndices()),
	textureID(rhs.getTextureID()),
	numBuffers(rhs.getNumBuffers()),
	vertexArrayID(rhs.getVertexArrayID()),
	drawMode(rhs.getDrawMode())
{
	/* Allocate space for the vertices, indices, and buffers on the heap. */
	vertices = new Vertex[rhs.getNumVertices()];
	indices  = new GLushort[rhs.getNumIndices()];
	bufferIDs = new GLuint[rhs.getNumBuffers()];

	/* Copy the bytes over from the rhs mesh. */
	memcpy(vertices, rhs.getVertices(), rhs.getNumVertices() * sizeof(Vertex));
	memcpy(indices, rhs.getIndices(), rhs.getNumIndices() * sizeof(GLushort));
	memcpy(bufferIDs, rhs.getBufferIDs(), rhs.getNumBuffers() * sizeof(GLuint));
}

/******************************************************************************
*                                                                             *
*                            Mesh::translateModel                             *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param translate                                                           *
*           Translation vector to be applied to this mesh.                    *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the translation property for this mesh to the indicated vector.       *
*                                                                             *
*******************************************************************************/
void Mesh::translateModel(glm::vec3 translate)
{
	translate_M = glm::translate(translate);
	changed = true;
}

/******************************************************************************
*                                                                             *
*                               Mesh::rotateModel                             *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param theta                                                               *
*           Angle by which the mesh will be rotated.                          *
*  @param axis                                                                *
*           Axis in world space by which the mesh will be rotated.            *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the rotation property for this mesh to the indicated vector. This     *
*  property will be applied BEFORE translation, and therefore occurs in       *
*  model space.                                                               *
*                                                                             *
*******************************************************************************/
void Mesh::rotateModel(GLfloat theta, glm::vec3 axis)
{
	rotate_M = glm::rotate(theta, axis);
	changed = true;
}

/******************************************************************************
*                                                                             *
*                               Mesh::scaleModel                              *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param scale                                                               *
*           Scale vector to be applied to this mesh.                          *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the scale property for this mesh to the indicated vector.             *
*                                                                             *
*******************************************************************************/
void Mesh::scaleModel(glm::vec3 scale)
{
	scale_M = glm::scale(scale);
	changed = true;
}


/******************************************************************************
*                                                                             *
*                              Mesh::revolveModel                             *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param theta                                                               *
*           Angle by which the mesh will be revolved.                         *
*  @param axis                                                                *
*           Axis in world space by which the mesh will be revolved.           *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the revolution property for this mesh. This property will be applied  *
*  AFTER the mesh has been translated, and therefore occurs in world space.   *
*                                                                             *
*******************************************************************************/
void Mesh::revolveModel(GLfloat theta, glm::vec3 axis)
{
	revolve_M = glm::rotate(theta, axis);
	changed = true;
}

/******************************************************************************
*                                                                             *
*                               Mesh::getTransform                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The combined transformation matrix for this mesh.                          *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Calculates and returns the combined transformation matrix for this mesh.   *
*                                                                             *
*******************************************************************************/
glm::mat4 Mesh::getTransform()
{
	if (changed)
	{
		transform_MTW = revolve_M * translate_M * rotate_M * scale_M;
		changed = false;
	}
	return transform_MTW;
}

/******************************************************************************
*                                                                             *
*                             Mesh::clearTransform                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Clears the transformation properties for this mesh.                        *
*                                                                             *
*******************************************************************************/
void Mesh::clearTransform()
{
	revolve_M = translate_M = rotate_M = scale_M = transform_MTW = glm::mat4();
	changed = false;
}


/******************************************************************************
*                                                                             *
*                          Geometry::makeCube (static)                        *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D cube.                                *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D cube. The data for this 3-D cube is stored on the heap, so the  *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeCube(GLfloat side)
{
	/* Define return mesh. */
	Mesh* cube = new Mesh();

	cube->setTextureID(-1);
	cube->setDrawMode(GL_TRIANGLES);

	/* Deine vertices. */
	Vertex localVertices[] =
	{
		/* Vertex 0. */
		glm::vec3(-1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+1.0f, +0.0f, +0.0f), // Color.
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 1. */
		glm::vec3(+1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+0.0f, +1.0f, +0.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 2. */
		glm::vec3(+1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+0.0f, +0.0f, +1.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 3. */
		glm::vec3(-1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+1.0f, +1.0f, +1.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 4. */
		glm::vec3(-1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+1.0f, +0.0f, +1.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 5. */
		glm::vec3(+1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+0.0f, +0.5f, +0.2f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 6. */
		glm::vec3(+1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.8f, +0.6f, +0.4f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 7. */
		glm::vec3(-1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.3f, +1.0f, +0.5f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 8. */
		glm::vec3(+1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+0.2f, +0.5f, +0.2f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 9. */
		glm::vec3(+1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+0.9f, +0.3f, +0.7f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 10. */
		glm::vec3(+1.0f, -1.0f, +1.0f), // Position.
		glm::vec3(+0.3f, +0.7f, +0.5f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 11. */
		glm::vec3(+1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.5f, +0.7f, +0.5f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 12. */
		glm::vec3(-1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+0.7f, +0.8f, +0.2f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 13. */
		glm::vec3(-1.0f, +1.0f, -1.0f), // Position.
		glm::vec3(+0.5f, +0.7f, +0.3f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 14. */
		glm::vec3(-1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.4f, +0.7f, +0.7f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 15. */
		glm::vec3(-1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.2f, +0.5f, +1.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 16. */
		glm::vec3(+1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+0.6f, +1.0f, +0.7f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 17. */
		glm::vec3(-1.0f, +1.0f, +1.0f), // Position.
		glm::vec3(+0.6f, +0.4f, +0.8f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 18. */
		glm::vec3(-1.0f, -1.0f, +1.0f), // Position.
		glm::vec3(+0.2f, +0.8f, +0.7f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 19. */
		glm::vec3(+1.0f, -1.0f, +1.0f), // Position.
		glm::vec3(+0.2f, +0.7f, +1.0f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 21. */
		glm::vec3(+1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.8f, +0.3f, +0.7f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 22. */
		glm::vec3(-1.0f, -1.0f, -1.0f), // Position.
		glm::vec3(+0.8f, +0.9f, +0.5f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 23. */
		glm::vec3(-1.0f, -1.0f, +1.0f), // Position.
		glm::vec3(+0.5f, +0.8f, +0.5f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
		/* Vertex 24. */
		glm::vec3(+1.0f, -1.0f, +1.0f), // Position.
		glm::vec3(+0.9f, +1.0f, +0.2f), // Color
		glm::vec3(+0.0f, +0.0f, +0.0f), // Vertex Normal
		glm::vec2(+0.0f, +0.0f),        // Texture Coordinate.
	};

	/* Define indices. */
	GLushort localIndices[] = {
		0, 1, 2, 0, 2, 3, // Top
		4, 5, 6, 4, 6, 7, // Front
		8, 9, 10, 8, 10, 11, // Right
		12, 14, 13, 12, 14, 15, // Left
		16, 17, 18, 16, 18, 19, // Back
		20, 22, 21, 20, 23, 22, // Bottom
	};

	// Apply the scale transformation.
	glm::mat3 scale = glm::mat3{ glm::scale(glm::vec3{ side, side, side }) };
	for (int i = 0; i < ARRAY_SIZE(localVertices); i++)
		localVertices[i].position = scale * localVertices[i].position;

	/* Set the vertices and the indices for the mesh.. */
	cube->setVertices(ARRAY_SIZE(localVertices), localVertices);
	cube->setIndices(ARRAY_SIZE(localIndices), localIndices);

	/* Generate buffer and vertex arrays. */
	cube->genBufferArrayID();
	cube->genVertexArrayID();

	/* Return mesh. */
	return cube;
}

/******************************************************************************
*                                                                             *
*                     Geometry::makeTetrahedron (static)                      *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D tetraheron.                          *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D tetrahedron. The data for this tetrahedron is stored on the     *
*  heap, so the caller must be sure to free the memory once the mesh is no    *
*  caller must clean up after use.                                            *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeTetrahedron(GLfloat radius)
{
	/* Define return mesh. */
	Mesh* tetra = new Mesh();

	tetra->setTextureID(-1);
	tetra->setDrawMode(GL_TRIANGLES);

	glm::vec2 texture{0.0f, 0.0f};
	glm::vec3 a = glm::normalize(glm::vec3{ +1.0f, +1.0f, +1.0f });
	glm::vec3 b = glm::normalize(glm::vec3{ +1.0f, -1.0f, -1.0f });
	glm::vec3 c = glm::normalize(glm::vec3{ -1.0f, +1.0f, -1.0f });
	glm::vec3 d = glm::normalize(glm::vec3{ -1.0f, -1.0f, +1.0f });

	/* Deine vertices. */
	Vertex localVertices[] =
	{
		// Vertex 0. 
		a, COLORS[0], -c, texture,  
		// Vertex 1.
		d, COLORS[0], -c, texture,
		// Vertex 2. 
		b, COLORS[0], -c, texture, 
		// Vertex 3.
		a, COLORS[1], -b, texture, 
		// Vertex 4. 
		c, COLORS[1], -b, texture, 
		// Vertex 5.
		d, COLORS[1], -b, texture,    
		// Vertex 6.
		a, COLORS[2], -d, texture, 
		// Vertex 7. 
		b, COLORS[2], -d, texture,  
		// Vertex 8. 
		c, COLORS[2], -d ,texture,
		// Vertex 9.
		b, COLORS[3], -a, texture,
		// Vertex 10.
		d, COLORS[3], -a, texture,
		// Vertex 11.
		c, COLORS[3], -a, texture,
	};

	/* Define indices. */
	GLushort localIndices[] = {
		0, 1, 2, 
		3, 4, 5,
		6, 7, 8, 
		9, 10, 11,
	};

	// Apply the scale transformation.
	//glm::mat3 scale = glm::mat3{ 
	//	glm::scale(glm::vec3{ radius, radius, radius}) 
	//};
	//for (int i = 0; i < ARRAY_SIZE(localVertices); i++)
	//	localVertices[i].position = scale * localVertices[i].position;

	/* Set the vertices and the indices for the mesh.. */
	tetra->setVertices(ARRAY_SIZE(localVertices), localVertices);
	tetra->setIndices(ARRAY_SIZE(localIndices), localIndices);

	/* Generate buffer and vertex arrays. */
	tetra->genBufferArrayID();
	tetra->genVertexArrayID();

	/* Return mesh. */
	return tetra;
}

/******************************************************************************
*                                                                             *
*                        Geometry::makeSphere (static)                        *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  radius                                                                     *
*           Radius of the sphere to generate.                                 *
*  tesselation                                                                *
*           Level of approximation from the base sphere (icosohedron).        *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D sphere.                              *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D sphere. The data for this 3-D sphere is stored on the heap, so  *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeSphere(GLfloat radius, GLuint tesselation)
{
	// Create return mesh and cache for tessellating the sphere.
	Mesh* sphere = Geometry::loadObj(ICO_OBJ);
	std::map<GLuint, GLushort> middlePointIndexCache;

	// Get the vertices from icosohedron.
	std::vector<Vertex> localVerts;
	for (GLuint i = 0; i < sphere->getNumVertices(); i++)
	{
		localVerts.push_back(sphere->getVertex(i));
		localVerts.at(i).position = glm::normalize(localVerts.at(i).position) *
			radius;
	}

	// Get the indices from the icosohedron.
	std::vector<GLushort> localIndices;
	for (GLuint i = 0; i < sphere->getNumIndices(); i++)
		localIndices.push_back(sphere->getIndex(i));

	// Index variables for tesselation.
	GLushort a, b, c;

	// Tesselate.
	for (GLuint i = 0; i < tesselation; i++)
	{
		// Buffer for adding indices.
		std::vector<GLushort> newIndices;

		// Split each triangle into 4 new triangles.
		for (GLuint j = 0; j < localIndices.size(); j += 3)
		{
			// Vertices of originial triangle.
			GLushort v_0 = localIndices.at(j + 0);
			GLushort v_1 = localIndices.at(j + 1);
			GLushort v_2 = localIndices.at(j + 2);

			// Get the middle index of each side of the triangle.
			a = middlePointIndex(v_0, v_1, &localVerts,
				&middlePointIndexCache);
			b = middlePointIndex(v_1, v_2, &localVerts,
				&middlePointIndexCache);
			c = middlePointIndex(v_2, v_0, &localVerts,
				&middlePointIndexCache);

			// Make sure vertex sits on sphere radius.
			localVerts[a].position *= radius;
			localVerts[b].position *= radius;
			localVerts[c].position *= radius;
			
			// Triangle 1.
			newIndices.push_back(v_0); 
			newIndices.push_back(a); 
			newIndices.push_back(c);

			// Triangle 2.
			newIndices.push_back(v_1);
			newIndices.push_back(b);
			newIndices.push_back(a);

			// Triangle 3.
			newIndices.push_back(v_2);
			newIndices.push_back(c);
			newIndices.push_back(b);

			// Triangle 4.
			newIndices.push_back(a);
			newIndices.push_back(b);
			newIndices.push_back(c);

		}

		// Update local indices.
		localIndices = newIndices;
	}

	// Copy over the local vertex data.
	sphere->setVertices(&localVerts);

	// Copy over the local index data.
	sphere->setIndices(&localIndices);

	// Delete the buffers on the graphics hardware.
	glDeleteBuffers(sphere->getNumBuffers(), sphere->getBufferIDs());
	const GLuint vaid = sphere->getVertexArrayID();
	glDeleteBuffers(1, &vaid);

	// Generate new buffers.
	sphere->genBufferArrayID();
	sphere->genVertexArrayID();

	// Return the mesh.
	return sphere;
}

/******************************************************************************
*                                                                             *
*                        Geometry::makeEllipse (static)                       *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  r_x                                                                        *
*           Radius in the x-coordinate of the ellipse.                        *
*  r_y                                                                        *
*           Radius in the y-coordinate of the ellipse.                        *
*  r_z                                                                        *
*           Radius in the z-coordinate of the ellipse.                        *
*  tesselation                                                                *
*           Level of approximation from the base sphere (icosohedron).        *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D ellipse.                             *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D ellipse. The data for this ellipse is stored on the heap, so    *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeEllipse(GLfloat r_x, GLfloat r_y, GLfloat r_z,
	GLuint tesselation)
{
	// Generate a unit sphere.
	Mesh* ellipse = makeSphere(1, tesselation);

	// Create the scale transformation.
	glm::mat3 scale = glm::mat3(glm::scale(glm::vec3{ r_x, r_y, r_z }));

	// Apply transformation to all vertices in the ellipse.
	Vertex* vertices = ellipse->getVertices();
	GLuint count = ellipse->getNumVertices();
	for (int i = 0; i < count; i++)
		vertices[i].position = scale * vertices[i].position;

	// Delete the buffers on the graphics hardware.
	glDeleteBuffers(ellipse->getNumBuffers(), ellipse->getBufferIDs());
	const GLuint vaid = ellipse->getVertexArrayID();
	glDeleteBuffers(1, &vaid);

	// Generate new buffers.
	ellipse->genBufferArrayID();
	ellipse->genVertexArrayID();

	// Return transformed sphere.
	return ellipse;

}

/******************************************************************************
*                                                                             *
*                        Geometry::makeTorus (static)                         *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D torus.                               *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D torus. The data for this 3-D torus is stored on the heap, so    *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeTorus()
{
	return Geometry::loadObj(TORUS_OBJ);
}

/******************************************************************************
*                                                                             *
*                     Geometry::makeClyinder (static)                         *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  radius                                                                     *
*           Radius of the base of the cylinder.                               *
*  length                                                                     *
*           Length of the cylinder.                                           *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D cylinder.                            *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D cylinder. The data for this 3-D cylinder is stored on the heap, *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeCylinder(GLfloat radius, GLfloat length)
{
	// Create return mesh.
	Mesh* cylinder = new Mesh();
	// Define the number of segments for the cylinder.
	GLuint NUM_SEGMENTS = 20;
	// Calculate the arc of each segment.
	GLfloat theta = (2 * M_PI) / NUM_SEGMENTS;
	
	// Temp variables for vertex members.
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 textureCoordinates{ +0.0f, +0.0f };
	glm::vec3 v_0, v_1, v_2, v_3;

	// Local Vertex and Index vectors.
	std::vector<Vertex> localVerts;
	std::vector<GLushort> localIndices;

	// Local variables.
	GLuint index = 0;
	GLuint color_size = ARRAY_SIZE(COLORS);
	GLfloat base = -(length / 2);

	v_0 = glm::vec3{ +0.0f, +0.0f, base };
	v_1 = glm::vec3{ +0.0f, +0.0f, base + length };

	for (GLuint i = 0; i < NUM_SEGMENTS; i++)
	{

		// Update color.
		GLuint c = (i + 1) % color_size;
		color = COLORS[c];

		// Calculate first vertex of circle.
		v_2.x = cosf(i * theta) * radius;
		v_2.y = sinf(i * theta) * radius;
		v_2.z = base;

		// Calculate second vertex of circle.
		v_3.x = cosf((i + 1) * theta) * radius;
		v_3.y = sinf((i + 1) * theta) * radius;
		v_3.z = base;

		// Add bottom circle.
		localVerts.push_back({
			v_0, color, {+0.0f, -1.0f, +0.0f}, textureCoordinates
		});
		localVerts.push_back({
			v_2, color, { +0.0f, -1.0f, +0.0f }, textureCoordinates
		});
		localVerts.push_back({
			v_3, color, { +0.0f, -1.0f, +0.0f }, textureCoordinates
		});

		// Add indices
		localIndices.push_back(index + 0);
		localIndices.push_back(index + 1);
		localIndices.push_back(index + 2);

		v_2.z = base + length;
		v_3.z = base + length;

		// Add top circle.
		localVerts.push_back({
			v_2, color, { +0.0f, +1.0f, +0.0f }, textureCoordinates
		});
		localVerts.push_back({
			v_1, color, { +0.0f, +1.0f, +0.0f }, textureCoordinates
		});
		localVerts.push_back({
			v_3, color, { +0.0f, +1.0f, +0.0f }, textureCoordinates
		});

		// Add indices.
		localIndices.push_back(index + 3);
		localIndices.push_back(index + 4);
		localIndices.push_back(index + 5);

		// Increment Index
		index += 6;

	}

	// Build cylinder up from the bottom.
	for (GLfloat z = 0; z < length; z += 1.0f)
	{
		// Loop around a circle.
		for (GLuint i = 0; i < NUM_SEGMENTS; i++)
		{
			// Vertex 0 coordinates.
			v_0.x = cosf(i * theta) * radius;
			v_0.y = sinf(i * theta) * radius;
			v_0.z = base + z;

			// Vertex 1 coordinates.
			v_1.x = cosf((i + 1) * theta) * radius;
			v_1.y = sinf((i + 1) * theta) * radius;
			v_1.z = base + z;

			// Vertex 2 coordinates.
			v_2.x = v_0.x;
			v_2.y = v_0.y;
			v_2.z = base + z + 1;

			// Vertex 3 coordinates.
			v_3.x = v_1.x;
			v_3.y = v_1.y;
			v_3.z = base + z + 1;
			
			// Update color.
			GLuint c = (i + (GLuint)z) % color_size;
			color = COLORS[c];

			// Calculate normal.
			normal = -glm::normalize(glm::cross((v_2 - v_0), (v_1 - v_0)));

			// Add the 4 vertices.
			localVerts.push_back({ 
				v_0, color, normal, textureCoordinates
			});
			localVerts.push_back({
				v_1, color, normal, textureCoordinates
			});
			localVerts.push_back({
				v_2, color, normal, textureCoordinates
			});
			localVerts.push_back({
				v_3, color, normal, textureCoordinates
			});

			// Bottom triangle.
			localIndices.push_back(index + 0);
			localIndices.push_back(index + 1);
			localIndices.push_back(index + 2);
			
			// Top triangle.
			localIndices.push_back(index + 3);
			localIndices.push_back(index + 2);
			localIndices.push_back(index + 1);

			// Increment index counter.
			index += 4;

		}
	}

	// Copy over the local vertex data.
	cylinder->setVertices(&localVerts);

	// Copy over the local index data.
	cylinder->setIndices(&localIndices);

	// Generate new buffers.
	cylinder->genBufferArrayID();
	cylinder->genVertexArrayID();

	// Return cylinder.
	return cylinder;
}


/******************************************************************************
*                                                                             *
*                     Geometry::makeClyinder (static)                         *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  radius                                                                     *
*           Radius of the base of the cylinder.                               *
*  length                                                                     *
*           Length of the cylinder.                                           *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  A mesh object describing a simple 3-D cylinder.                            *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which creates a new Mesh struct containing the data for a  *
*  simple 3-D cylinder. The data for this 3-D cylinder is stored on the heap, *
*  caller must be sure to free the memory once the mesh is no longer needed.  *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::makeCone(GLfloat radius, GLfloat length)
{
	// Create return mesh.
	Mesh* cone = new Mesh();
	// Define the number of segments for the cone.
	GLuint NUM_SEGMENTS = 20;
	// Calculate the arc of each segment.
	GLfloat theta = (2 * M_PI) / NUM_SEGMENTS;

	// Temp variables for vertex members.
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 textureCoordinates{ +0.0f, +0.0f };
	glm::vec3 v_0, v_1, top{ +0.0f, +0.0f, length };

	// Local Vertex and Index vectors.
	std::vector<Vertex> localVerts;
	std::vector<GLushort> localIndices;

	// Local variables.
	GLuint index = 0;
	GLuint color_size = ARRAY_SIZE(COLORS);
	GLfloat  base = -(length / 2);

	// Loop around a circle.
	for (GLuint i = 0; i < NUM_SEGMENTS; i++)
	{
		// Vertex 0 coordinates.
		v_0.x = cosf(i * theta) * radius;
		v_0.y = sinf(i * theta) * radius;
		v_0.z = 0;

		// Vertex 1 coordinates.
		v_1.x = cosf((i + 1) * theta) * radius;
		v_1.y = sinf((i + 1) * theta) * radius;
		v_1.z = 0;

		// Update color.
		GLuint c = i % color_size;
		color = COLORS[c];

		// Calculate normal.
		normal = -glm::normalize(glm::cross((top - v_0), (top - v_0)));

		// Add the 3 vertices.
		localVerts.push_back({
			v_0, color, normal, textureCoordinates
		});
		localVerts.push_back({
			v_1, color, normal, textureCoordinates
		});
		localVerts.push_back({
			top, color, normal, textureCoordinates
		});

		// Add triangle.
		localIndices.push_back(index + 0);
		localIndices.push_back(index + 1);
		localIndices.push_back(index + 2);

		// Change color.
		c = (i + 1) % color_size;
		color = COLORS[c];

		// Add 3 new vertices.
		localVerts.push_back({
			v_0, color, { +0.0f, -1.0f, +0.0f }, textureCoordinates
		});
		localVerts.push_back({
			v_1, color, { +0.0f, -1.0f, +0.0f }, textureCoordinates
		});
		localVerts.push_back({
			{+0.0f, +0.0f, +0.0f}, color, { +0.0f, -1.0f, +0.0f }, 
			textureCoordinates
		});

		// Add triangle.
		localIndices.push_back(index + 3);
		localIndices.push_back(index + 4);
		localIndices.push_back(index + 5);

		// Increment index counter.
		index += 6;

	}

	// Copy over the local vertex data.
	cone->setVertices(&localVerts);

	// Copy over the local index data.
	cone->setIndices(&localIndices);

	// Generate new buffers.
	cone->genBufferArrayID();
	cone->genVertexArrayID();

	// Return cone.
	return cone;
}

/******************************************************************************
*                                                                             *
*                       Geometry::middlePointIndex (static)                   *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  i1                                                                         *
*           Index of the first point.                                         *
*  i2                                                                         *
*           Index of the second point.                                        *
*  verts                                                                      *
*           Pointer to the vector containing all of the vertices.             *
*  cache                                                                      *
*           Pointer to the cache of already-generated index points.           *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The index of the point between the indicated indices.                      *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static function which takes in two indices and returns the index of the    *
*  point between them, located on the unit shpere. The points are stored in   *
*  cache as they are generated, so that future calls to this function will    *
*  remember the previous values.                                              *
*                                                                             *
*******************************************************************************/
GLushort Geometry::middlePointIndex(GLushort i1, GLushort i2,
	std::vector<Vertex> *verts, std::map<GLuint, GLushort>* cache)
{
	// Generate the key for mapping the index.
	GLushort smaller = (i1 < i2) ? i1 : i2;
	GLushort larger = (i1 < i2) ? i2 : i1;

	// key = smaller * 2^10 + larger
	GLuint key = (smaller << 10) + larger;

	// If the middle value exists, return the index value.
	if (cache->count(key) != 0)
	{
		return cache->at(key);
	}
	// Otherwise, the value is not in cache and must be calculated.
	else
	{
		// Grab the two vertices.
		Vertex v1 = verts->at(i1);
		Vertex v2 = verts->at(i2);

		// Position of the middle vertex.
		glm::vec3 position = glm::normalize(v1.position + v2.position);
		
		// Average color of the two vertices.
		glm::vec3 color = 0.5f * (v1.color + v2.color);
		
		// Average normal of the two vertices.
		glm::vec3 normal = glm::normalize(v1.normal + v2.normal);
		
		// Average texture coordinate of the two vertices.
		glm::vec2 textureCoordinate = (v1.textureCoordinate 
			+ v2.textureCoordinate) * 0.5f;

		// New middle vertex.
		Vertex middle = { position, color, normal, textureCoordinate };

		// Add the new vertex to verts and map the index to the cache.
		GLushort index = verts->size();
		verts->push_back(middle);
		(*cache)[key] = index;

		// Return the index of the vertex.
		return index;
	}
}

/******************************************************************************
*                                                                             *
*                          Mesh::vertexBufferSize()  (const)                  *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The number of bytes required for this Mesh's vertex buffer.                *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Calculates the number of bytes required for this Mesh's vertex buffer.     *
*                                                                             *
*******************************************************************************/
GLsizeiptr Mesh::vertexBufferSize() const
{
	return numVertices * sizeof(Vertex);
}

/******************************************************************************
*                                                                             *
*                          Mesh::indexBufferSize()  (const)                   *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The number of bytes required for this Mesh's index buffer.                 *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Calculates the number of bytes required for this Mesh's index buffer.      *
*                                                                             *
*******************************************************************************/
GLsizeiptr Mesh::indexBufferSize() const
{
	return numIndices * sizeof(GLushort);
}

/******************************************************************************
*                                                                             *
*                         Mesh::setVertices()  (overloaded)                   *
*                                                                             *
*******************************************************************************
* PARAMETERS (1)                                                              *
*  n                                                                          *
*           The number of elements in the array to copy.                      *
*  a                                                                          *
*           A pointer to the array of values to be set as the vertices.       *
*                                                                             *
* PARAMETERS (2)                                                              *
*  v                                                                          *
*           A pointer to a vector containing the values to be set as the      *
*           vertices.                                                         *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the number of vertices, as well as their values, for this Mesh.       *
*                                                                             *
*******************************************************************************/
void Mesh::setVertices(GLuint n, Vertex* a)
{
	// Set number of vertices.
	numVertices = n;
	// Allocate space on the heap.
	vertices = new Vertex[n];
	// Copy the data over to the allocated space.
	memcpy(vertices, a, sizeof(Vertex) * n);
}
void Mesh::setVertices(std::vector<Vertex>* v)
{
	// Set number of vertices.
	numVertices = v->size();
	// Allocate space on the heap.
	delete[] vertices;
	vertices = new Vertex[v->size()];
	// Copy the data over to the allocated space.
	memcpy(vertices, v->data(), sizeof(Vertex) * v->size());
}

/******************************************************************************
*                                                                             *
*                         Mesh::setIndices()   (overloaded)                   *
*                                                                             *
*******************************************************************************
* PARAMETERS (1)                                                              *
*  n                                                                          *
*           The number of elements in the array to copy.                      *
*  a                                                                          *
*           A pointer to the array of values to be set as the indices.        *
*                                                                             *
* PARAMETERS (2)                                                              *
*  v                                                                          *
*           A pointer to a vector containing the values to be set as the      *
*           indices.                                                          *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Sets the number of indices, as well as their values, for this Mesh.        *
*                                                                             *
*******************************************************************************/
void Mesh::setIndices(GLuint n, GLushort* a)
{
	// Set number of indices.
	numIndices = n;
	// Allocate space on the heap.
	indices = new GLushort[n];
	// Copy the data over to the allocated space.
	memcpy(indices, a, sizeof(GLushort) * n);
}
void Mesh::setIndices(std::vector<GLushort>* i)
{
	// Set number of vertices.
	numIndices = i->size();
	// Allocate space on the heap.
	delete[] indices;
	indices = new GLushort[i->size()];
	// Copy the data over to the allocated space.
	memcpy(indices, i->data(), sizeof(GLushort) * i->size());
}

/******************************************************************************
*                                                                             *
*                                 Mesh::loadObj                               *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param objFile                                                             *
*        The path to the OBJ file that is to be loaded. Must point to a valid *
*        OBJ file.                                                            *
*  @param textureFile (optional)                                              *
*        The path to the texture file to be loaded. Can point to any valid    *
*        image format. By default this parameter is NULL.                     *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Loads an OBJ file and generates a Mesh object based on the Vertex and      *
*  Index data. This function also allows the option for the caller to apply a *
*  texture to the newly loaded Mesh via the textureFile parameter.            *
*                                                                             *
*******************************************************************************/
Mesh* Geometry::loadObj(const char* objFile, const char* textureFile)
{
	// Create a new Mesh object on the heap.
	Mesh* obj = new Mesh();

	// Declare the Vectors for shape and material data.
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	glm::vec3 triangleColor{1.0f, 1.0f, 1.0f};

	// Load the indicated OBJ file and get the error message. 
	std::string errMsg = tinyobj::LoadObj(shapes, materials, objFile);

	// If there was an error message, prompt the user and exit function. */
	if (!errMsg.empty())
	{
		std::cerr << "Error loading obj: " << errMsg << std::endl;
		return nullptr;
	}

	// Get the shape from the file.
	tinyobj::shape_t  s = shapes.at(0);

	// Copy over the Vertex data.
	std::vector<Vertex> localVertices;
	for (GLuint i = 0; i < (s.mesh.positions.size() / 3); i++)
	{
		// Generate random color.
		if ((i % 3) == 0)
		{
			GLuint index = i / 3;
			GLuint size = ARRAY_SIZE(COLORS);
			GLuint u = index % size;
			triangleColor = COLORS[u];
		}

		// Add new vertex.
		localVertices.push_back({

			// Vertex Position.
			{ s.mesh.positions.at((3 * i) + 0),
			s.mesh.positions.at((3 * i) + 1),
			s.mesh.positions.at((3 * i) + 2) },

			// Vertex Color.
			triangleColor,

			// Vertex Normal.
			{ s.mesh.normals.at((3 * i) + 0),
			s.mesh.normals.at((3 * i) + 1),
			s.mesh.normals.at((3 * i) + 2) },

			// U, V Coordinate.
			{ s.mesh.texcoords.at((2 * i) + 0),
			1 - s.mesh.texcoords.at((2 * i) + 1) }

		});
	}

	// Copy the Index data.
	std::vector<GLushort> localIndices;
	for(GLuint i = 0; i < s.mesh.indices.size(); i++) 
		localIndices.push_back(
			(GLushort) s.mesh.indices.at(i)
		);

	// Set the vertices and indices of this mesh.
	obj->setVertices(&localVertices);
	obj->setIndices(&localIndices);
	
	// Generate buffer and vertex arrays.
	obj->genBufferArrayID();
	obj->genVertexArrayID();

	// If the texture file was provided, generate the texture.
	if (textureFile != NULL)
		obj->genTextureID(textureFile);

	// Return the mesh.
	return obj;
}

/******************************************************************************
*                                                                             *
*                               Mesh::genTextureID                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  filename                                                                   *
*        The path to the texture that is to be loaded. Can be of any valid    *
*        image format.                                                        *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Generates the texture buffer and sends the data from the indicated file    *
*  down to the graphics hardware. The texture ID is saved to the indicated    *
*  parameter for this Mesh.                                                   *
*                                                                             *
*******************************************************************************/
void Mesh::genTextureID(const char* filename)
{
	/* Enable Texture 2D. */
	glEnable(GL_TEXTURE_2D);

	/* If the filename is null, do nothing. */
	if(filename != NULL) 
	{
		/* Load the SDL_Surface from the file. */
		SDL_Surface* textureSurface = IMG_Load(filename);

		/* If the image was not loaded correctly, do nothing. */
		if(textureSurface != NULL) 
		{
			/* The default color scheme is RGB. */
			GLenum colorScheme = GL_RGB;

			/* If the file is a bitmap, change the color scheme to BGR. */
			std::string file(filename);
			std::string ext = file.substr(file.find('.'), file.length() - 1);
			if(ext == ".bmp") 
				colorScheme = GL_BGR;

			/* Generate the texture buffer. */
			glGenTextures(1, &textureID);

			/* Bind the texture ID to the appropriate binding point. */
			glBindTexture(GL_TEXTURE_2D, textureID);

			/* Send the image data down to the graphics card. */
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSurface->w, 
				textureSurface->h, 0, colorScheme, GL_UNSIGNED_BYTE, 
				textureSurface->pixels);

			/* Set the desred texture parameters. */
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
}

/******************************************************************************
*                                                                             *
*                            Mesh::genBufferArrayID                           *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Generates the graphics hardware buffers for data regarding this Mesh. The  *
*  two specific buffers for this class are the vertex buffer and the index    *
*  buffer. The IDs of these buffers are stored in the bufferIDs array.        *
*                                                                             *
*******************************************************************************/
void Mesh::genBufferArrayID()
{
	// Generate the buffer space.
	bufferIDs = new GLuint[numBuffers];
	glGenBuffers(numBuffers, bufferIDs);

	// Create vertex buffer.
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize(), vertices,
		GL_STATIC_DRAW);

	// Create index buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize(), indices, 
		GL_STATIC_DRAW);
}

/******************************************************************************
*                                                                             *
*                            Mesh::genVertexArrayID                           *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Generates the vertex array object buffer for this mesh. The vertex array   *
*  object keeps track of the vertex attribute locations for this specific     *
*  mesh. To draw this mesh, the vertex array object must be bound before      *
*  telling OpenGL to draw its elements. The vertex array ID is stored in the  *
*  vertexArrayID value.                                                       *
*                                                                             *
*******************************************************************************/
void Mesh::genVertexArrayID()
{
	// Generate Vertex Array Object.
	glGenVertexArrays(1, &vertexArrayID);

	// Bind this vertex array ID.
	glBindVertexArray(vertexArrayID);

	// Bind the vertex buffer.
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);

	// Enable the vertex attributes.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Vertex position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
		(void*) ATTRIBUTE_0_OFFSET);

	// Vertex color attribute.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,	sizeof(Vertex), 
		(void*) ATTRIBUTE_1_OFFSET);

	// Vertex normal attribute.
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(void*)ATTRIBUTE_2_OFFSET);

	// Vertex texture coordinate attribute.
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(void*)ATTRIBUTE_3_OFFSET);
}

/******************************************************************************
*                                                                             *
*                             Mesh::cleanUp (Destructor)                      *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Ensures that any allocated space is freed before the Mesh is discarded.    *
*  This call can be made explicitly or implicitly by the descructor.          *
*                                                                             *
*******************************************************************************/
void Mesh::cleanUp()
{
	// Delete the buffers on the graphics hardware. 
	glDeleteBuffers(numBuffers, bufferIDs);
	glDeleteBuffers(1, &vertexArrayID);

	// Free the space allocated on the heap for vertex/index data.
	delete[] vertices;
	delete[] indices;
	delete[] bufferIDs;

	// Remove any dangling pointers.
	vertices = NULL;
	indices = NULL;
	bufferIDs = NULL;

	// Set the number of vertices/indices to 0.
	numVertices = numIndices = 0;
}
