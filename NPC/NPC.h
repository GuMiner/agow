#pragma once
#include <string>
#include <map>
#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\vec4.hpp>
#include "Data\RenderableSentence.h"
#include "Data\UserPhysics.h"
#include "Managers\ModelManager.h"
#include "Utils\TypedCallback.h"
#include "Physics.h"
#include "DialogPane.h"

class NPC : public ICallback<UserPhysics::ObjectType>
{
public:
    enum Shape
    {
        DIAMOND,
        CAPSULE,
        CUBOID,
        COUNT
    };

    static const int INVULNERABLE = -1;

protected:
    static std::map<Shape, unsigned int> models;

    std::string name;
    RenderableSentence nameString;
    RenderableSentence interactionString;

    std::string description;
    Shape shape;

    int health;
    int startingHealth;
    bool showInteractionKeys;

    bool nearFieldCollisionLastFrame;

    btRigidBody* nearFieldBubble;
    
    bool selectionChange;
    Model model;

    bool CanKill() const;
    static Physics::CShape GetPhysicalShape(Shape shape);

public:
    static bool LoadNpcModels(ModelManager* modelManager);

    NPC(std::string name, std::string description, Shape shape, glm::vec4 color, int startingHealth);
    
    void LoadGraphics(FontManager* fontManager);
    void LoadNpcPhysics(Physics* physics, glm::vec3 startingPosition, float mass);
    
    bool Converse(DialogPane* dialogPane);
    std::string GetName() const;
    glm::vec3 GetPosition() const;
    virtual std::string GetDescription() const;
    bool IsAlive() const;

    virtual void Update(float gameTime, float elapsedTime);
    virtual void Render(FontManager* fontManager, ModelManager* modelManager, const glm::mat4& projectionMatrix);

    void UnloadNpcPhysics(Physics* physics);
    virtual ~NPC();

    // Used for NPC collision callbacks.
    virtual void Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData) override;
};

