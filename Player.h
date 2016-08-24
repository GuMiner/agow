#pragma once
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <Bullet\btBulletDynamicsCommon.h>
#include "Config\KeyBindingConfig.h"
#include "Managers\RegionManager.h"
#include "Managers\ModelManager.h"
#include "Math\Vec.h"
#include "BasicPhysics.h"
#include "Camera.h"

class Player
{
    // Where the viewer is currently oriented on-screen.
    Camera camera;
    PhysicalModel physicalModel;

    // Used for view rotation.
    sf::Vector2i lastMousePos;

public:
    Player();
    bool LoadPlayerModel(ModelManager* modelManager);
    void LoadPlayerPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass);
    
    void Update(float frameTime);
    void Render(ModelManager* modelManager, const vec::mat4& projectionMatrix);

    const vec::vec3 GetViewPosition() const;
    const vec::quaternion GetViewOrientation() const;

    void Warp(RegionManager* regionManager, btDynamicsWorld* world, const vec::vec2 mapPos);      

    void UnloadPlayerPhysics(BasicPhysics physics);
};
