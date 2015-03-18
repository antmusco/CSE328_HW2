/*******************************************************************************
 *                                                                             *
 *                              Included Header Files                          *
 *                                                                             *
 ******************************************************************************/
#include <gl\glew.h>
#include <SDL\SDL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include "Display.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include "EventManager.h"

/*******************************************************************************
 *                                                                             *
 *                        Defined Constants and Macros                         *
 *                                                                             *
 ******************************************************************************/
#define  FULLSCREEN_ENABLED   true
#define  DEFAULT_HEIGHT		  600
#define  DEFAULT_WIDTH        800
#define  MILLIS_PER_SECOND    1000
#define  MAX_ROT              2 * M_PI
#define  MESHES_PATH          "res/meshes/";
#define  TEXUTRES_PATH        "res/textures/";
#define  SHADERS_PATH         "res/shaders/";
#define  FRAMES_PER_SECOND    100
#define  PROJECT_TITLE        "CSE 328 Homework 2"
#define  PRINT(a)             std::cout << a << std::endl;

/*******************************************************************************
 *                                                                             *
 *                                Global Variables                             *
 *                                                                             *
 ******************************************************************************/

/* Unit vectors for the 3-D space. */
glm::vec3  bases[] =
{
    {+1.0f, +0.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +0.0f, +1.0f}
};

/*******************************************************************************
 *                                                                             *
 *                                     main                                    *
 *                                                                             *
 *******************************************************************************
 * PARAMETERS                                                                  *
 *  argc                                                                       *
 *        The number of command line strings.                                  *
 *  argv                                                                       *
 *        The array of command line stirngs.                                   *
 *                                                                             *
 *******************************************************************************
 * RETURNS                                                                     *
 *  0 on success, any non-zero value on failure.                               *
 *                                                                             *
 *******************************************************************************
 * DESCRIPTION                                                                 *
 *  Beginning point of the program.                                            *
 *                                                                             *
 *******************************************************************************/
int main(int argc, char* argv[])
{
	/* Initialize SDL with all subsystems. */
	SDL_Init(SDL_INIT_EVERYTHING);

	/* Create the display, shader, camera, and event manager. */
	Display      display(PROJECT_TITLE, DEFAULT_WIDTH, DEFAULT_HEIGHT);
	Shader       shader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
	Camera*      camera = display.getCamera();

	/* Apply the shaders and maximize the display. */
	Geometry::shader = &shader;
	display.setShader(shader);
	display.maximize();
	GLfloat speed = 1.0f;
	EventManager eventManager(camera, &speed);

	// Create mesh/transform vectors.
	std::vector<Mesh*> meshes;
	std::vector<glm::mat4*> transforms;
	
	// Create geometries.
	meshes.push_back(Geometry::makeSphere(1, 0));        // Icosohedron.
	meshes.push_back(Geometry::makeSphere(1, 1));        // 80-Triangle Sphere.
	meshes.push_back(Geometry::makeSphere(1, 2));        // 320-Triangle Sphere.
	meshes.push_back(Geometry::makeEllipse(1, 2, 1.5, 3)); // Ellipse.
	meshes.push_back(Geometry::makeCylinder(1, 4));      // Cylinder.
	meshes.push_back(Geometry::makeCube(1));
	meshes.push_back(Geometry::makeTetrahedron(1));      // Tetrahedron.
	meshes.push_back(Geometry::makeCone(1, 4));          // Cone.
	meshes.push_back(Geometry::makeTorus());             // Torus.

	/* Place meshes in the world space. */
	GLfloat s = (2 * M_PI) / meshes.size();
	GLfloat radius = 6.0f;
	for (GLuint i = 0; i < meshes.size(); i++)
	{
		meshes[i]->translateModel(glm::vec3{ cosf(i * s) * radius, +0.0f, sinf(i * s) * radius });
	}
	
	meshes[1]->setIsSolid(false);

	/* Instantiate the event reference. */
	SDL_Event event;
	SDL_PollEvent(&event);	

	/* Begin the milliseconds counter. */
	GLuint startMillis = 0, tempMillis = 0, currentMillis = 0, 
		millisPerFrame = (GLuint)((1.0 / FRAMES_PER_SECOND) * MILLIS_PER_SECOND);
	startMillis = tempMillis = currentMillis = SDL_GetTicks();	
	GLfloat t = 0;

	/* Main loop. */
	while (event.type != SDL_QUIT)
	{
		/* Handle the new event. */
		eventManager.handleSDLEvent(&event);

		/* Get the new number of milliseconds. */
		currentMillis = SDL_GetTicks();
		
		/* If a new frame is to be drawn, update the display. */
		if ((currentMillis - startMillis) >= millisPerFrame)
		{
			startMillis = currentMillis;
			display.repaint(meshes);
			for (Mesh* m : meshes)
			{
				m->rotateModel(t, glm::vec3{ +0.0f, +1.0f, +0.0f });
				m->revolveModel(t, glm::vec3{ +0.0f, +1.0f, +1.0f });
			}
			t += 0.003f;
		}

		/* Update the temporary millisecond counter. */
		tempMillis = currentMillis;
		
		/* Get the next event. */
		SDL_PollEvent(&event);
	}

	/* Free the shapes. */
	for (Mesh* m : meshes)
	{
		m->cleanUp();
		delete m;
	}

	/* Quit using SDL. */
	SDL_Quit();

	/* Exit Success. */
	return 0;
}
