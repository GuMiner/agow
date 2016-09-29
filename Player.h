#pragma once
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <Bullet\btBulletDynamicsCommon.h>
#include "Config\KeyBindingConfig.h"
#include "Data\UserPhysics.h"
#include "Managers\RegionManager.h"
#include "Managers\ModelManager.h"
#include "Math\Vec.h"
#include "Utils\TypedCallback.h"
#include "BasicPhysics.h"
#include "Camera.h"

class Player : public ICallback<UserPhysics::ObjectType>
{
    // TODO configurable
    const float SpeedLimit = 20.0f;
    const float RoadSpeedLimit = 200.0f;

    // Where the viewer is currently oriented on-screen.
    Camera camera;
    PhysicalModel physicalModel;

    // Used for view rotation.
    sf::Vector2i lastMousePos;
    
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

public:
    Player();
    bool LoadPlayerModel(ModelManager* modelManager);
    void LoadPlayerPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass);
    
    void Update(float frameTime, int terrainTypeOn);
    void Render(ModelManager* modelManager, const vec::mat4& projectionMatrix);

    const vec::vec2 GetTerrainPosition() const;
    const vec::vec3 GetViewPosition() const;
    const vec::quaternion GetViewOrientation() const;
    const vec::mat4 GetViewMatrix() const;

    void Warp(RegionManager* regionManager, btDynamicsWorld* world, const vec::vec2 mapPos);      

    void UnloadPlayerPhysics(BasicPhysics physics);

    // Used for collision callbacks.
    virtual void Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData) override;
};
