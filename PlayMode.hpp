#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <iostream>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
		uint8_t released = 0;
	} left, right, down, up, arrowLeft, arrowRight, arrowUp, arrowDown, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;

	// manipulation speed
	float speed = 2.0f;
	float mvnt = 0.0f;
	float wobble = 0.0f;

	float mvnt_hip = 0.0f;
	float mvnt_upper_leg = 0.0f;
	float mvnt_lower_leg = 0.0f;
	float mvnt_gripper = 0.0f;

	float *mvnt_curr_joint = &mvnt_upper_leg;
	
	enum Joint {UPPER_LEG, LOWER_LEG, GRIPPER};
	Joint curr_joint = UPPER_LEG;
	Joint increment_joint (Joint j) {
		switch (j) {
			case UPPER_LEG:
				mvnt_curr_joint = &mvnt_lower_leg;
				return LOWER_LEG;
			case LOWER_LEG:
				mvnt_curr_joint = &mvnt_gripper;
				return GRIPPER;
			case GRIPPER:
				mvnt_curr_joint = &mvnt_upper_leg;
				return UPPER_LEG;
			default:
				std::cout << "ERROR: invalid joint" << std::endl;
				return UPPER_LEG;
		}
	}
	
	//camera:
	Scene::Camera *camera = nullptr;

};
