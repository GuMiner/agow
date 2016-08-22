#pragma once
#include <string>
#include <map>
#include "Managers\ModelManager.h"
#include "Math/Vec.h"
#include "BasicPhysics.h"

class NPC
{
public:
	enum Shape
	{
		DIAMOND,
		CAPSULE,
		CUBOID
	};

private:
	std::map<Shape, unsigned int> models;

	std::string name;
	std::string description;
	Shape shape;
	vec::vec4 color;

	int health;

	PhysicalModel physicalModel;

	bool CanKill()
	{
		return health != -1;
	}

	static BasicPhysics::CShape GetPhysicalShape(Shape shape);

public:
	static bool LoadNpcModels(ModelManager* modelManager);

	NPC(std::string name, std::string description, Shape shape, vec::vec4 color, int startingHealth);
	
	void LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass);

	void Render(const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix);

	void UnloadNpcPhysics(BasicPhysics physics);
	virtual ~NPC();
};

