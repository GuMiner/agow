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

	static const int INVULNERABLE = -1;

private:
	static std::map<Shape, unsigned int> models;

	std::string name;
	std::string description;
	Shape shape;
	vec::vec4 color;

	int health;
	bool isSelected;

	PhysicalModel physicalModel;

	bool CanKill()
	{
		return health != INVULNERABLE;
	}

	static BasicPhysics::CShape GetPhysicalShape(Shape shape);

public:
	static bool LoadNpcModels(ModelManager* modelManager);

	NPC(std::string name, std::string description, Shape shape, vec::vec4 color, int startingHealth);
	
	void LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass);

	void Render(ModelManager* modelManager, const vec::mat4& projectionMatrix);

	void UnloadNpcPhysics(BasicPhysics physics);
	virtual ~NPC();
};

