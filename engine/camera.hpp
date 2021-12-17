#pragma once
#include "common.hpp"
#include "components.hpp"

#ifdef _WIN32
// Windows defines these to nothing for some ancient compat reasons...
#undef near
#undef far
#endif


struct Camera
{
	void makePerspective(float fovy, float aspect, float near, float far)
	{
		projection = glm::perspective(fovy, aspect, near, far);
		this->fovy = fovy;
		this->aspect = aspect;
		this->near = near;
		this->far = far;
	}

	void makeOrtho(float left, float right, float bottom, float top, float near, float far)
	{
		projection = glm::ortho(left, right, bottom, top, near, far);
		this->near = near;
		this->far = far;
		fovy = 0;
		aspect = (right - left) / (top - bottom);
	}

	void updateViewMatrix(vec3 position, quat rotation) {
		view = glm::mat4_cast(glm::inverse(rotation));
		view = glm::translate(view, -position);
	}

	void updateViewMatrix(vec3 position) {
		view = glm::translate(mat4(1.f), -position);
	}

	vec3 position() const { return -view[3] * view; }
	vec3 forward() const { return vec4(forward_axis, 0) * view; }
	vec3 up() const { return vec4(up_axis, 0) * view; }
	vec3 left() const { return vec4(left_axis, 0) * view; }
	vec3 right() const { return vec4(right_axis, 0) * view; }

	mat4 projection = mat4();
	mat4 view = mat4();
	float near = 0;
	float far = 0;
	float fovy = 0;
	float aspect = 1.f;
};


struct Frustum {

	struct Plane {
		Plane() {}
		Plane(vec3 p, vec3 n): normal(glm::normalize(n)), distance(glm::dot(normal, p)) { }
		vec3 normal = up_axis;
		float distance = 0.f;

		float signedDistance(vec3 point) const { return glm::dot(normal, point) - distance; }
		bool inFront(vec3 point, float radius = 0.f) const { return signedDistance(point) > -radius; }
	};

	Frustum(const Camera& cam) {
		float halfVert = cam.far * tanf(cam.fovy * 0.5f);
		float halfHoriz = halfVert * cam.aspect;

		vec3 pos = cam.position();
		vec3 forward = cam.forward();
		vec3 forwardFar = cam.far * forward;
		vec3 up = cam.up();
		vec3 right = cam.right();
		nearPlane = { pos + cam.near * forward, forward };
		farPlane = { pos + forwardFar, -forward };
		rightPlane = { pos, glm::cross(up, forwardFar + right * halfHoriz) };
		leftPlane = { pos, glm::cross(forwardFar - right * halfHoriz, up) };
		topPlane = { pos, glm::cross(right, forwardFar - up * halfVert) };
		bottomPlane = { pos, glm::cross(forwardFar + up * halfVert, right) };
	}

	bool visible(const Transform& transform, const Bounds& bounds) const {
		return visible(transform.position, bounds.radius * glm::compMax(transform.scale));
	}

	bool visible(vec3 position, float radius) const {
		return nearPlane.inFront(position, radius) &&
			leftPlane.inFront(position, radius) &&
			rightPlane.inFront(position, radius) &&
			topPlane.inFront(position, radius) &&
			bottomPlane.inFront(position, radius) &&
			farPlane.inFront(position, radius);
	}

	Plane topPlane;
	Plane bottomPlane;
	Plane rightPlane;
	Plane leftPlane;
	Plane farPlane;
	Plane nearPlane;
};