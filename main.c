#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include "simp_GLerror.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdint.h>
#include "linmath.h"

#define WIDTH 480
#define HEIGHT 240 

static bool cast_ray(const vec3 start, const vec3 end, const uint32_t step_count,
		const vec3 sphere_pos, const float radius_sqr, vec3 intersect_vec);
static GLuint build_program(char* vertex_src, char* fragment_src);
static char* read_file(const char* file);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(void)
{
	if(!glfwInit()){	exit(1);	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = NULL;
#ifdef FULLSCREEN
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(mode->width, mode->height, "Ounga Bounga", monitor, NULL);
#else
	window = glfwCreateWindow(1080, 720, "Unga Bunga", NULL, NULL);
#endif

	glfwSetKeyCallback(window, key_callback);
	if(!window){	glfwTerminate(); exit(1);	}
	glfwMakeContextCurrent(window);
	glewInit();
	glEnable(GL_DEPTH_TEST);

	float ratio;
	int width, height;

	float t1, t2, elapsed_frame_time = 10.0f, dt = 1.5e-8;

	float quad_pos[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
	float quad_tex[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
	uint32_t quad_index[] = { 0, 2, 1, 2, 3, 1 };

	GLuint VAO, quad_pos_buffer, quad_index_buffer, quad_tex_buffer;

	GL(glGenVertexArrays(1, &VAO));
	GL(glGenBuffers(1, &quad_pos_buffer));
	GL(glGenBuffers(1, &quad_index_buffer));
	GL(glGenBuffers(1, &quad_tex_buffer));

	GL(glBindVertexArray(VAO));
	GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_index_buffer));
	GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof * quad_index, (void*)quad_index,	GL_STATIC_DRAW));

	GL(glBindBuffer(GL_ARRAY_BUFFER, quad_pos_buffer));
	GL(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof * quad_pos, (void*)quad_pos, GL_STATIC_DRAW));
	GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0));
	GL(glEnableVertexAttribArray(0));

	GL(glBindBuffer(GL_ARRAY_BUFFER, quad_tex_buffer));
	GL(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof * quad_tex, (void*)quad_tex, GL_STATIC_DRAW));
	GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0));
	GL(glEnableVertexAttribArray(1));

	GL(glBindVertexArray(0));

	uint8_t* buffer = calloc(WIDTH * HEIGHT * 4u, sizeof * buffer);

	vec3 sphere_pos = { 0.0f, 0.0f, 5.0f };
	float radius_sqr = 0.2f;

	vec3 light_pos = { -1.0f, -1.0f, 0.0f };

	uint32_t step_count = 1024;
	for(int i = 0; i < WIDTH; i++)
	{
		break;
		for(int j = 0; j < HEIGHT; j++)
		{
			vec3 start_vec = { -1.0f + 2.0f * (float)i / (float)WIDTH, -1.0f + 2.0f * (float)j / (float)HEIGHT,
				0.0f };
			vec3 end_vec;
			end_vec[0] = start_vec[0];
			end_vec[1] = start_vec[1];
			end_vec[2] = 10.0f;
			vec3 intersect_vec;
			if(cast_ray(start_vec, end_vec, step_count, sphere_pos, radius_sqr, intersect_vec))
			{
				vec3 normal;
				vec3 light_ray_normal;
				vec3_sub(light_ray_normal, intersect_vec, light_pos);
				vec3_norm(light_ray_normal, light_ray_normal);
				vec3_sub(normal, intersect_vec, sphere_pos);
				vec3_norm(normal, normal);
				buffer[3 * (i + j * WIDTH) + 0] = (uint8_t)(255.0f * 0.5f * (normal[0] + 1.0f));
				buffer[3 * (i + j * WIDTH) + 1] = (uint8_t)(255.0f * 0.5f * (normal[1] + 1.0f));
				buffer[3 * (i + j * WIDTH) + 2] = (uint8_t)(255.0f * 0.5f * (normal[2] + 1.0f));
			}
			else
			{
				buffer[3 * (i + j * WIDTH) + 0] = (uint8_t)(255.0f * 0.5f * (1.0f + end_vec[0]));
				buffer[3 * (i + j * WIDTH) + 1] = (uint8_t)(255.0f * 0.5f * (1.0f + end_vec[1]));
				buffer[3 * (i + j * WIDTH) + 2] = (uint8_t)(255.0f * end_vec[2] * 0.1f);
			}
		}
	}

	GLuint texture;
	GL(glGenTextures(1, &texture));
	GL(glBindTexture(GL_TEXTURE_2D, texture));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)buffer));
	GL(glGenerateMipmap(GL_TEXTURE_2D));
	GL(glBindTexture(GL_TEXTURE_2D, 0));

	GLuint program = build_program(read_file("vertex.glsl"), read_file("fragment.glsl"));

	while (!glfwWindowShouldClose(window))
	{
		t1 = glfwGetTime();
		glfwGetFramebufferSize(window, &width, &height);
		ratio = (float)width / (float)height;
		glViewport(0, 0, width, height);

		elapsed_frame_time += dt;
		if(elapsed_frame_time > 10.0f)
		{
			elapsed_frame_time *= 0.0f;
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

			GL(glUseProgram(program));
			GL(glUniform1f(glGetUniformLocation(program, "ratio"), ratio));
			GL(glUniform1f(glGetUniformLocation(program, "time"), t1));
			GL(glActiveTexture(GL_TEXTURE0));
			GL(glBindTexture(GL_TEXTURE_2D, texture));
			GL(glBindVertexArray(VAO));

			GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
			glfwSwapBuffers(window);
		}

		glfwPollEvents();
		t2 = glfwGetTime();
		dt = t2 - t1;
	}

	free(buffer);
	GL(glDeleteBuffers(1, &quad_pos_buffer));
	GL(glDeleteBuffers(1, &quad_tex_buffer));
	GL(glDeleteBuffers(1, &quad_index_buffer));
	GL(glDeleteVertexArrays(1, &VAO));
	glfwTerminate();
	return 0x45;
}

static bool cast_ray(const vec3 start, const vec3 end, const uint32_t step_count,
		const vec3 sphere_pos, const float radius_sqr, vec3 intersect_vec)
{
	float t_step = 1.0f / (float)step_count;
	vec3 step_vec;
	vec3_sub(step_vec, end, start);
	vec3_scale(step_vec, step_vec, t_step);
	vec3 curr_vec;
	vec3_dup(curr_vec, start);
	for(int i = 0; i < step_count; i++)
	{
		vec3 diff;
		vec3_sub(diff, curr_vec, sphere_pos);
		if(vec3_mul_inner(diff, diff) < radius_sqr)
		{
			vec3_dup(intersect_vec, curr_vec);
			return true;
		}
		vec3_add(curr_vec, curr_vec, step_vec);
	}
	return false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	    glfwSetWindowShouldClose(window, true);
}

static GLuint build_program(char* vertex_src, char* fragment_src)
{
	GLuint program;
	if(!vertex_src || !fragment_src)
	{
		fprintf(stderr, "Pointer to vertex or fragment shader source was NULL\n");
		return -1;
	}

	GLuint vertex_shader, fragment_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GL(glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_src, NULL));
	GL(glCompileShader(vertex_shader));

	GLint vertex_compiled;
	GL(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled));
	if (vertex_compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		GL(glGetShaderInfoLog(vertex_shader, 1024, &log_length, message));
		fprintf(stderr, "%s\n", message);
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GL(glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_src, NULL));
	GL(glCompileShader(fragment_shader));

	GLint fragment_compiled;
	GL(glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled));
	if (fragment_compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		GL(glGetShaderInfoLog(fragment_shader, 1024, &log_length, message));
		fprintf(stderr, "%s\n", message);
	}

	program = glCreateProgram();
	GL(glAttachShader(program, vertex_shader));
	GL(glAttachShader(program, fragment_shader));
	GL(glLinkProgram(program));

	GLint program_linked;
	GL(glGetProgramiv(program, GL_LINK_STATUS, &program_linked));
	if (program_linked != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		GL(glGetProgramInfoLog(program, 1024, &log_length, message));
		fprintf(stderr, "%s\n", message);
	}
	GL(glDeleteShader(vertex_shader));
	GL(glDeleteShader(fragment_shader));
	free(vertex_src);
	free(fragment_src);
	return program;
}

static char* read_file(const char* file)
{
	FILE* f = fopen(file, "r");
	if(!f)
		return NULL;
	unsigned long file_size;
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char* res = calloc(file_size + 1, sizeof * res);
	if(!res)
	{
		fclose(f);
		return NULL;
	}
	
	char* curr = res;
	char c;
	while((c = fgetc(f)) != EOF)
	{
		*curr = c;
		curr++;
	}
	*curr = '\0';
	fclose(f);
	return res;
}
