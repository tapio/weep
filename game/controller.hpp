#include "common.hpp"

class Controller
{
public:
	Controller(vec3 pos, quat rot);

	void update(float dt);

	vec3 position;
	quat rotation;

	vec3 angles = vec3(0, 0, 0);

	float speed = 2.0f;
	float fast = 4.0f;

	float moveForce = 3000.f;
	float brakeForce = 1000.f;
	float jumpForce = 500.f;
	class btRigidBody* body = nullptr;

	bool onGround = true;
	bool fly = false;
};
