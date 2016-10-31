#pragma once
#include <string>
#include <vector>
#include <SFML\System.hpp>
#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>
#include "Config\KeyBindingConfig.h"
#include "Data\UserPhysics.h"
#include "Managers\RegionManager.h"
#include "Managers\ModelManager.h"
#include "Utils\TypedCallback.h"
#include "Weapons\GravityWeapon.h"
#include "Weapons\PressureWeapon.h"
#include "Weapons\RockWeapon.h"
#include "Weapons\SunbeamWeapon.h"
#include "Weapons\WeaponBase.h"
#include "Physics.h"
#include "Camera.h"

class Player : public ICallback<UserPhysics::ObjectType>
{
    // TODO configurable
    const float SpeedLimit = 20.0f;
    const float RoadSpeedLimit = 200.0f;

    // Where the viewer is currently oriented on-screen.
    Camera camera;
    Model model;

    // Used for view rotation.
    glm::ivec2 lastMousePos;
    
    enum MotionType
    {
        ON_FOOT,
        MOTORCYCLE,
        DRONES
    };

    MotionType motionType;
    bool isOnGround;

    int enemyKos;
    int civilianKos;
    int allyKos;

    GravityWeapon gravityWeapon;
    PressureWeapon pressureWeapon;
    RockWeapon rockWeapon;
    SunbeamWeapon sunbeamWeapon;

    WeaponBase* selectedWeapon;
    std::vector<std::string> inventory;

public:
    Player(ModelManager* modelManager, Physics* physics);
    bool LoadPlayerModel(ModelManager* modelManager);
    void LoadPlayerPhysics(Physics* physics, glm::vec3 startingPosition, float mass);
    
    void Update(float frameTime, int terrainTypeOn);
    void Render(ModelManager* modelManager, const glm::mat4& projectionMatrix);

    const glm::vec2 GetTerrainPosition() const;
    const glm::vec3 GetPosition() const;
    const glm::quat GetOrientation() const;
    const glm::vec2 Get2DOrientation() const;
    const glm::quat GetViewOrientation() const;
    const glm::mat4 GetViewMatrix() const;

    void Warp(RegionManager* regionManager, btDynamicsWorld* world, const glm::vec2 mapPos);      

    void UnloadPlayerPhysics(Physics* physics);

    // Used for collision callbacks.
    virtual void Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData) override;
};
