#pragma once
#define _USE_MATH_DEFINES
#define STRMAX 100
#include<glm/glm.hpp>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Math.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

enum class FigureType
{
	Torus, Point, MiddlePoint, Cursor, BezierCurveC0, PointsLine, BezierCurveC2, InterpolationCurveC2, BezierPatchC0, BezierPatchC0Cylinder, BezierPatchC2, BezierPatchC2Cylinder
};

class Program;



class Figure
{
public:
	Figure();
	~Figure() = default;
	static int idx;
	bool GetGui(int i, Figure* par);
	std::vector<float> GetVertices();
	std::vector<unsigned int> GetIndices();
	glm::mat4 GetModel();
	bool GetSelected() { return selected; };
	void Select() { selected = true; };
	void Unselect() { selected = false; };
	void Scale(float change);
	void Rotate(glm::vec3 axis, float angle);
	void Rotate(glm::quat new_q);
	void RotateAround(glm::vec3 point, glm::vec3 axis, float angle);
	void ScaleAround(glm::vec3 point, float change);
	void Move(float x, float y, float z);
	void MoveTo(float x, float y, float z);
	void MoveVec(float a, glm::vec3 v);
	glm::vec3 GetPos();
	virtual void Draw();
	void virtual Initialize(Program* _program);
	void virtual RecalcFigure();
	bool CanMove() { return canMove; };
	bool HasParent() { return parents.size() > 0; };

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	int from;
	FigureType figureType;
	bool keepOpen = false;
	char name[STRMAX] = "";
	char newName[STRMAX] = "";
	bool isCurve = false;
	bool isPatch = false;
	void virtual CleanUp();
protected:
	virtual bool Create();
	void RecalcModel();
	glm::mat4 translation;
	glm::quat rotation_q;
	glm::mat4 scale;
	glm::mat4 model;
	float scale_f;
	bool selected = false;
	bool canMove = true;
	bool virtual GetGuiInternal(Figure* par) = 0;
	std::vector<Figure*> parents;
	std::vector<int> parents_cnt;
	Program* program = NULL;
	Shader shader;
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
private:
	bool selected_old = false;
};
