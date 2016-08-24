#pragma once
#include <string>
#include <map>
#include "Data\RenderableSentence.h"
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
    RenderableSentence nameString;

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
	
    void LoadGraphics(FontManager* fontManager);
	void LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass);
    
    void Update(float gameTime, float elapsedTime);
	void Render(FontManager* fontManager, ModelManager* modelManager, const vec::mat4& projectionMatrix);

	void UnloadNpcPhysics(BasicPhysics physics);
	virtual ~NPC();
};

