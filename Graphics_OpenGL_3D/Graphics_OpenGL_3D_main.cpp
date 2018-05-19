#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.

#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f
#define NUMBER_OF_CAMERAS 8
#define MAIN_CAM 0
#define FRONT_CAM 1
#define SIDE_CAM 2
#define TOP_CAM 3
#define CCTV_1 4
#define CCTV_2 5
#define CCTV_3 6
#define CCTV_DYN 7

typedef struct _CAMERA {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
	float fov_y, aspect_ratio, near_clip, far_clip;
	int move_status;
} CAMERA;
CAMERA camera[NUMBER_OF_CAMERAS];
int camera_selected;

typedef struct _VIEWPORT {
	int x, y, w, h;
} VIEWPORT;
VIEWPORT viewport[NUMBER_OF_CAMERAS];

//glm::mat4 ModelViewMatrix, ViewMatrix, ProjectionMatrix;
glm::mat4 ViewMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ProjectionMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewProjectionMatrix; // Mp * Mv * Mm

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

void set_ViewMatrix(int camera_id) {
	ViewMatrix[camera_id] = glm::mat4(1.0f);
	ViewMatrix[camera_id][0].x = camera[camera_id].uaxis.x;
	ViewMatrix[camera_id][0].y = camera[camera_id].vaxis.x;
	ViewMatrix[camera_id][0].z = camera[camera_id].naxis.x;

	ViewMatrix[camera_id][1].x = camera[camera_id].uaxis.y;
	ViewMatrix[camera_id][1].y = camera[camera_id].vaxis.y;
	ViewMatrix[camera_id][1].z = camera[camera_id].naxis.y;

	ViewMatrix[camera_id][2].x = camera[camera_id].uaxis.z;
	ViewMatrix[camera_id][2].y = camera[camera_id].vaxis.z;
	ViewMatrix[camera_id][2].z = camera[camera_id].naxis.z;

	ViewMatrix[camera_id] = glm::translate(ViewMatrix[camera_id], -camera[camera_id].pos);
}

#include "Object_Definitions.h"

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLineWidth(2.0f);
	draw_axes(MAIN_CAM);
	glLineWidth(1.0f);
 
    draw_static_object(&(static_objects[OBJ_BUILDING]), 0, MAIN_CAM);

	draw_static_object(&(static_objects[OBJ_TABLE]), 0, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_TABLE]), 1, MAIN_CAM);

	draw_static_object(&(static_objects[OBJ_LIGHT]), 0, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 1, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 2, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 3, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 4, MAIN_CAM);

	draw_static_object(&(static_objects[OBJ_TEAPOT]), 0, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0, MAIN_CAM);
 	draw_static_object(&(static_objects[OBJ_FRAME]), 0, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0, MAIN_CAM);
	draw_static_object(&(static_objects[OBJ_COW]), 0, MAIN_CAM);

	draw_animated_tiger(MAIN_CAM);
	glutSwapBuffers();
}

void motion(int x, int y) {
}

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0, polygon_fill_on = 0, depth_test_on = 0;

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;
		}
		break;
	case 'f':
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;
	case 'd':
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;
	aspect_ratio = (float)width / height;
	glViewport(0, 0, width, height);
	camera[MAIN_CAM].aspect_ratio = aspect_ratio;
	viewport[MAIN_CAM].x = viewport[MAIN_CAM].y = 0;
	viewport[MAIN_CAM].w = (int)(0.70f*width);
	viewport[MAIN_CAM].h = (int)(0.70f*height);
	ProjectionMatrix[MAIN_CAM] = glm::perspective(camera[MAIN_CAM].fov_y*TO_RADIAN, camera[MAIN_CAM].aspect_ratio, camera[MAIN_CAM].near_clip, camera[MAIN_CAM].far_clip);
	ViewProjectionMatrix[MAIN_CAM] = ProjectionMatrix[MAIN_CAM] * ViewMatrix[MAIN_CAM];

	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	tiger_data.rotation_angle = (timestamp_scene % 360)*TO_RADIAN;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup_OpenGL_stuffs);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_camera(void) {
	/*
	MAIN_CAM 0
	FRONT_CAM 1
	SIDE_CAM 2
	TOP_CAM 3
	CCTV_1 4
	CCTV_2 5
	CCTV_3 6
	CCTV_DYN 7
	*/

	camera[MAIN_CAM].pos = glm::vec3(600.0f, 600.0f, 200.0f);
	camera[MAIN_CAM].uaxis = glm::vec3(600.0f, 600.0f, 200.0f); // right
	camera[MAIN_CAM].vaxis = glm::vec3(600.0f, 600.0f, 200.0f); // up
	camera[MAIN_CAM].naxis = glm::vec3(600.0f, 600.0f, 200.0f); // back

	camera[MAIN_CAM].move_status = 0;
	camera[MAIN_CAM].fov_y = 15.0f;
	camera[MAIN_CAM].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[MAIN_CAM].near_clip = 1.0f;
	camera[MAIN_CAM].far_clip = 10000.0f;
	
	//set_ViewMatrix(MAIN_CAM);
	ViewMatrix[MAIN_CAM] = glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST); // Default state
	 
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);

	initialize_camera();

	/*
	if (0) {
		ViewMatrix = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
			glm::vec3(-10.0f, 0.0f, 0.0f));
	}
	if (0) {
		ViewMatrix = glm::lookAt(glm::vec3(800.0f, 90.0f, 25.0f), glm::vec3(0.0f, 90.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	}

	if (1) {
		ViewMatrix = glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	}
	*/
}

void prepare_scene(void) {
	define_axes();
	define_static_objects();
	define_animated_tiger();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
