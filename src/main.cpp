// Core libraries
#include <iostream>
#include <cmath>

// Third party libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Local headers
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "Camera.h"
#include "bone.h"


using namespace std;
using namespace glm;

double get_last_elapsed_time() {
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

class Application : public EventCallbacks {
public:
	WindowManager *windowManager = nullptr;
    Camera *camera = nullptr;

    std::shared_ptr<Shape> shape;
	std::shared_ptr<Program> phongShader;

    GLuint VAO;
    GLuint VBO, imat_VBO;

    bool walk_next = false;
    bool run_next = false;
    bool slowmo = false;

    mat4 animmat[200];
    int animmatsize=0;
    
    double gametime = 0;
    bool wireframeEnabled = false;
    bool mousePressed = false;
    bool mouseCaptured = false;
    glm::vec2 mouseMoveOrigin = glm::vec2(0);
    glm::vec3 mouseMoveInitialCameraRot;

    Application() {
        camera = new Camera();
    }
    
    ~Application() {
        delete camera;
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		// Movement
        if (key == GLFW_KEY_1 && action == GLFW_PRESS) {walk_next = true;}
        if (key == GLFW_KEY_2 && action == GLFW_PRESS) {run_next = true;}
        if (key == GLFW_KEY_3 && action == GLFW_PRESS) {slowmo = !slowmo;}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
        mousePressed = (action != GLFW_RELEASE);
        if (action == GLFW_PRESS) {
            resetMouseMoveInitialValues(window);
        }
    }
    
    void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
        if (mousePressed || mouseCaptured) {
            float yAngle = (xpos - mouseMoveOrigin.x) / windowManager->getWidth() * 3.14159f;
            float xAngle = (ypos - mouseMoveOrigin.y) / windowManager->getHeight() * 3.14159f;
            camera->setRotation(mouseMoveInitialCameraRot + glm::vec3(-xAngle, -yAngle, 0));
        }
    }

	void resizeCallback(GLFWwindow *window, int in_width, int in_height) { }
    
    // Reset mouse move initial position and rotation
    void resetMouseMoveInitialValues(GLFWwindow *window) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseMoveOrigin = glm::vec2(mouseX, mouseY);
        mouseMoveInitialCameraRot = camera->rot;
    }

	bone *root = NULL;
	int size_stick = 0;
    all_animations all_animation;

	void initGeom(const std::string& resourceDirectory) {
		readtobone(resourceDirectory + "/walk.fbx", &root, &all_animation);
        readtobone(resourceDirectory + "/run.fbx", NULL, &all_animation);

        root->set_animations(&all_animation, animmat, animmatsize);

        //generate the VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VBO);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        vector<vec3> pos;
        vector<unsigned int> imat;

        root->write_to_VBOs(vec3(0, 0, 0), pos, imat);
        size_stick = pos.size();
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*pos.size(), pos.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        //indices of matrix:
        glGenBuffers(1, &imat_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, imat_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint)*imat.size(), imat.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)0);

        glBindVertexArray(0);
	}
	
	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
        
		// Initialize the GLSL programs
        phongShader = std::make_shared<Program>();
        phongShader->setShaderNames(resourceDirectory + "/phong.vert", resourceDirectory + "/phong.frag");
        phongShader->init();

        phongShader->addUniform("Manim");
	}
    
    glm::mat4 getPerspectiveMatrix() {
        float fov = 3.14159f / 4.0f;
        float aspect = windowManager->getAspect();
        return glm::perspective(fov, aspect, 0.01f, 10000.0f);
    }

	void render() {
		glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        double frametime = get_last_elapsed_time();
        static double totaltime_ms=0;
        totaltime_ms += frametime*1000.0;
        static double totaltime_untilframe_ms = 0;
        totaltime_untilframe_ms += frametime*1000.0;

        for (int ii = 0; ii < 200; ii++)
            animmat[ii] = mat4(1);

        static int current_animation = 1;
        static int next_animation = 1;


        //animation frame system
        int anim_step_width_ms = root->getDuration(next_animation) / root->getKeyFrameCount(next_animation);
        static int frame = 0;
        if (totaltime_untilframe_ms >= anim_step_width_ms) {
            totaltime_untilframe_ms = 0;
            frame++;
        }

        if (slowmo) {
        	if (frame >= (root->animation[next_animation]->frames - 2) * 10) {
        		frame = 0;
        		current_animation = next_animation;
        		if (walk_next) {
        			next_animation = 0;
        		}
        		else if (run_next) {
        			next_animation = 1;
        		}
        		walk_next = run_next = false;
        	}
        	root->play_slowmo_animation(frame, current_animation, next_animation);
        }
        else {
	        if (frame >= root->animation[next_animation]->frames - 1) {
	        	// We're at the end of our animation - Set frames to 0 and see if we should swap
	        	frame = 0;
	        	current_animation = next_animation;
	        	if (walk_next) {
	        		next_animation = 0;
	        	}
	        	else if (run_next) {
	        		next_animation = 1;
	        	}
	        	walk_next = run_next = false;
	        }
	        root->play_animation(frame, current_animation, next_animation);
	    }

		// Clear framebuffer.
		glClearColor(0.3f, 0.7f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks.
		glm::mat4 V, M, P;
        P = getPerspectiveMatrix();
        V = camera->getViewMatrix();
        M = glm::mat4(1);
        
        /**************/
        /* DRAW SHAPE */
        /**************/
        M = glm::translate(glm::mat4(1), glm::vec3(0, 0, -3));
        phongShader->bind();
        phongShader->setMVP(&M[0][0], &V[0][0], &P[0][0]);


        glBindVertexArray(VAO);

        glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f));
        M = TransZ * S;
        glUniformMatrix4fv(phongShader->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniformMatrix4fv(phongShader->getUniform("Manim"), 200, GL_FALSE, &animmat[0][0][0]);
        glDrawArrays(GL_LINES, 4, size_stick-4);
        glBindVertexArray(0);   


        phongShader->unbind();
	}
};

int main(int argc, char **argv) {
	std::string resourceDir = "../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

    // Initialize window.
	WindowManager * windowManager = new WindowManager();
	windowManager->init(800, 600);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);
    
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle())) {
        // Update camera position.
        application->camera->update();
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
