#pragma once
#include "common.hpp"

struct Tween
{
	Tween(float duration_ = 1.f, bool start = true) {
		t = start ? 0.f : 1.f;
		duration = duration_;
		speed = 1.f / duration_;
	}

	void reset() {
		t = 0.f;
	}

	float update(float dt) {
		t += dt * speed;
		return t;
	}

	bool active() const {
		return t < 1.f;
	}

	float t = 0.f;
	float duration = 1.f;
	float speed = 1.f;
};


/*
  Easing functions
  by Kristoffer Gronlund, 2014
  https://github.com/krig/easing
  Public domain
  This work is a spiritual descendent (not to say derivative work) of works done by the following individuals:
  Warren Moore (https://github.com/warrenm)
  Robert Penner (http://www.robertpenner.com/easing/)
  George McGinley Smith (http://gsgd.co.uk/sandbox/jquery/easing/)
  James Padolsey (http://james.padolsey.com/demos/jquery/easing/)
  Authors of jQuery (http://plugins.jquery.com/project/Easing)
  Matt Gallagher (http://cocoawithlove.com/2008/09/parametric-acceleration-curves-in-core.html)
  Jesse Crossen (http://stackoverflow.com/questions/5161465/how-to-create-custom-easing-function-with-core-animation)
*/

namespace easing {

	template <typename T> T ease(T a, T b, T t) {
		return a + (b - a) * t;
	}

	template <typename T> T linear(T t) {
		return t;
	}

	template <typename T> T quadraticIn(T t) {
		return t * t;
	}

	template <typename T> T quadraticOut(T t) {
		return -(t * (t - 2.));
	}

	template <typename T> T quadraticInOut(T t) {
		return (t < 0.5) ? 2. * t * t : (-2. * t * t) + (4. * t) - 1.;
	}

	template <typename T> T cubicIn(T t) {
		return t * t * t;
	}

	template <typename T> T cubicOut(T t) {
		const T f = t - 1.; return f * f * f + 1.;
	}

	template <typename T> T cubicInOut(T t) {
		if (t < T(0.5)) {
			return 4. * t * t * t;
		} else {
			const T f = (2. * t) - 2.;
			return 0.5 * f * f * f + 1.;
		}
	}

	template <typename T> T quarticIn(T t) {
		return t * t * t * t;
	}

	template <typename T> T quarticOut(T t) {
		const T f = t - 1.;
		return f * f * f * (1. - t) + 1.;
	}

	template <typename T> T quarticInOut(T t) {
		if(t < 0.5) {
			return 8. * t * t * t * t;
		} else {
			T f = (t - 1.);
			return -8. * f * f * f * f + 1.;
		}
	}

	template <typename T> T quinticIn(T t) {
		return t * t * t * t * t;
	}

	template <typename T> T quinticOut(T t) {
		T f = (t - 1.);
		return f * f * f * f * f + 1.;
	}

	template <typename T> T quinticInOut(T t) {
		if (t < 0.5) {
			return 16. * t * t * t * t * t;
		} else {
			T f = ((2. * t) - 2.);
			return  0.5 * f * f * f * f * f + 1.;
		}
	}

	template <typename T> T sineIn(T t) {
		return sin((t - 1.) * M_PI_2) + 1.;
	}

	template <typename T> T sineOut(T t) {
		return sin(t * M_PI_2);
	}

	template <typename T> T sineInOut(T t) {
		return 0.5 * (1. - cos(t * M_PI));
	}

	template <typename T> T circularIn(T t) {
		return 1. - sqrt(1. - (t * t));
	}

	template <typename T> T circularOut(T t) {
		return sqrt((2. - t) * t);
	}

	template <typename T> T circularInOut(T t) {
		if (t < 0.5) {
			return 0.5 * (1 - sqrt(1 - 4. * (t * t)));
		} else {
			return 0.5 * (sqrt(-((2. * t) - 3.) * ((2. * t) - 1.)) + 1.);
		}
	}

	template <typename T> T exponentialIn(T t) {
		return (t <= 0) ? t : pow(2., 10. * (t - 1.));
	}

	template <typename T> T exponentialOut(T t) {
		return (t >= 1.) ? t : 1. - pow(2., -10. * t);
	}

	template <typename T> T exponentialInOut(T t) {
		if (t <= 0. || t >= 1.)
			return t;

		if (t < 0.5) {
			return 0.5 * pow(2., (20. * t) - 10.);
		} else {
			return -0.5 * pow(2., (-20. * t) + 10.) + 1.;
		}
	}

	template <typename T> T elasticIn(T t) {
		return sin(13. * M_PI_2 * t) * pow(2., 10. * (t - 1.));
	}

	template <typename T> T elasticOut(T t) {
		return sin(-13. * M_PI_2 * (t + 1.)) * pow(2., -10. * t) + 1.;
	}

	template <typename T> T elasticInOut(T t) {
		if (t < 0.5) {
			return 0.5 * sin(13. * M_PI_2 * (2. * t)) * pow(2., 10. * ((2. * t) - 1.));
		} else {
			return 0.5 * (sin(-13. * M_PI_2 * ((2. * t - 1) + 1)) * pow(2., -10. * (2. * t - 1.)) + 2.);
		}
	}

	template <typename T> T backIn(T t) {
		return t * t * t - t * sin(t * M_PI);
	}

	template <typename T> T backOut(T t) {
		const T f = 1. - t;
		return 1. - (f * f * f - f * sin(f * M_PI));
	}

	template <typename T> T backInOut(T t) {
		if (t < 0.5) {
			const T f = 2. * t;
			return 0.5 * (f * f * f - f * sin(f * M_PI));
		} else {
			const T f = (1. - (2.*t - 1.));
			return 0.5 * (1. - (f * f * f - f * sin(f * M_PI))) + 0.5;
		}
	}

	template <typename T> T bounceOut(T t) {
		if (t < 4. / 11.) {
			return (121. * t * t) / 16.;
		} else if (t < 8. / 11.) {
			return (363. / 40. * t * t) - (99 / 10. * t) + 17 / 5.;
		} else if (t < 9. / 10.) {
			return (4356. / 361. * t * t) - (35442. / 1805. * t) + 16061. / 1805.;
		} else {
			return (54. / 5. * t * t) - (513. / 25. * t) + 268. / 25.;
		}
	}

	template <typename T> T bounceIn(T t) {
		return 1. - bounceOut(1. - t);
	}

	template <typename T> T bounceInOut(T t) {
		if (t < 0.5) {
			return 0.5 * bounceIn(t * 2.);
		} else {
			return 0.5 * bounceOut(t * 2. - 1.) + 0.5;
		}
	}

	template <typename T> T perlinInOut(T t) {
		T t3 = t * t * t;
		T t4 = t3 * t;
		T t5 = t4 * t;
		return 6. * t5 - 15. * t4 + 10. * t3;
	}
}
