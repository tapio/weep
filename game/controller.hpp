#include "common.hpp"

class Controller
{
public:
	Controller(vec3& pos, quat& rot);

	void update(float dt);

	vec3& position;
	quat& rotation;
	float speed = 2.0f;
	float fast = 4.0f;
};
