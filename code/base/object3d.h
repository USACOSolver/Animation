#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
class Object3D
{
public:
	glm::vec3 position = {0.0f, 0.0f, 0.0f};
	glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
	glm::vec3 scale = {1.0f, 1.0f, 1.0f};

	glm::vec3 getFront() const;

	glm::vec3 getUp() const;

	glm::vec3 getRight() const;

	glm::mat4 getModelMatrix() const;
};

class Plane : public Object3D
{
public:
	Plane() {
		initPlaneVertices();
		initGLResources();
	}
	Plane(glm::vec3 p, glm::vec3 n)
	{
		position = p;
		normal = n;
		initPlaneVertices();
		initGLResources();
	}
	~Plane()
	{
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vbo);
	}
	void draw()
	{
		glBindVertexArray(_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	GLuint _vao = 0;
	GLuint _vbo = 0;

private:
	std::vector<float> vertices;
	void initPlaneVertices()
	{
		glm::vec3 d1, d2, v(1, 0, 0);
		if (glm::dot(normal, v) == 0)
		{
			v = glm::vec3(0, 1, 0);
		}
		d1 = glm::cross(v, normal);
		d2 = glm::cross(normal, d1);
		glm::normalize(d1);
		glm::normalize(d2);
		float dist = position.length();
		glm::vec3 e[] = {d1 + d2, -d1 + d2, -d1 - d2, d1 - d2};
		for (int i = 0; i < 3; i++)
		{
			e[i] = e[i] + position;
		}
		int ord[] = {0, 1, 2};
		for (int j = 0; j < 3; j++)
		{
			int i = ord[j];
			vertices.push_back(e[i][0]);
			vertices.push_back(e[i][1]);
			vertices.push_back(e[i][2]);
			vertices.push_back(normal[0]);
			vertices.push_back(normal[0]);
			vertices.push_back(normal[0]);
		}
		int ord1[] = {0, 2, 3};
		for (int j = 0; j < 3; j++)
		{
			int i = ord[j];
			vertices.push_back(e[i][0]);
			vertices.push_back(e[i][1]);
			vertices.push_back(e[i][2]);
			vertices.push_back(normal[0]);
			vertices.push_back(normal[0]);
			vertices.push_back(normal[0]);
		}
	}
	
	void initGLResources()
	{
		// plane VAO
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);
		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		// glEnableVertexAttribArray(2);
		// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
		glBindVertexArray(0);
	};
	glm::vec3 normal;
};