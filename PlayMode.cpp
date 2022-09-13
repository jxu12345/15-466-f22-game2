#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

GLuint arm_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > arm_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("arm.pnct"));
	arm_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load<Scene> arm_scene (LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("arm.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = arm_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = arm_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

// detect if reached goal cube
bool reached_goal(glm::vec3 const &gripper_head_pos, glm::vec3 const &cube_pos) {
	std::cout << "gripper_head_pos: " << std::to_string(gripper_head_pos.x) << ", " << std::to_string(gripper_head_pos.y) << ", " << std::to_string(gripper_head_pos.z) << std::endl;
	std::cout << "cube_pos: " << std::to_string(cube_pos.x) << ", " << std::to_string(cube_pos.y) << ", " << std::to_string(cube_pos.z) << std::endl;
	
	return (
		   gripper_head_pos.x >= cube_pos.x - 0.15f
		&& gripper_head_pos.x <= cube_pos.x + 0.15f
		&& gripper_head_pos.y >= cube_pos.y - 0.15f
		&& gripper_head_pos.y <= cube_pos.y + 0.15f
		&& gripper_head_pos.z >= cube_pos.z - 0.15f
		&& gripper_head_pos.z <= cube_pos.z + 0.15f
	);	
}

PlayMode::PlayMode() : scene(*arm_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		std::cout << transform.name << std::endl;
		if (transform.name == "Hip") hip = &transform;
		else if (transform.name == "UpperLeg") upper_leg = &transform;
		else if (transform.name == "LowerLeg") lower_leg = &transform;
		else if (transform.name == "Gripper") gripper = &transform;
		else if (transform.name == "GripperHead") gripper_head = &transform;
		else if (transform.name == "Cube") cube = &transform;


	}
	if (gripper_head == nullptr) {
		std::cout << "gripper_head is null" << std::endl;
	}
	else {
		std::cout << std::to_string(gripper_head->position.x) << " " << std::to_string(gripper_head->position.y) << " " << std::to_string(gripper_head->position.z) << std::endl;
	}

	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");
	if (gripper == nullptr) throw std::runtime_error("Gripper not found.");
	if (gripper_head == nullptr) throw std::runtime_error("Gripper head not found.");
	if (cube == nullptr) throw std::runtime_error("Goal cube not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;
	gripper_base_rotation = gripper->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			arrowUp.downs += 1;
			arrowUp.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrowDown.downs += 1;
			arrowDown.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrowLeft.downs += 1;
			arrowLeft.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrowRight.downs += 1;
			arrowRight.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}

	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			arrowUp.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrowDown.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrowLeft.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrowRight.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			space.released = true;
			return true;
		}

	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	if (arrowLeft.pressed) {
		mvnt_hip += speed * elapsed;
	}
	else if (arrowRight.pressed) {
		mvnt_hip -= speed * elapsed;
	}

	if (arrowUp.pressed) {
		*mvnt_curr_joint += speed * elapsed;
	}
	if (arrowDown.pressed) {
		*mvnt_curr_joint -= speed * elapsed;
	}
	if (space.released) {
		curr_joint = increment_joint(curr_joint);
		space.released = false;
	}

	hip->rotation = hip_base_rotation * glm::angleAxis(
			glm::radians(mvnt_hip),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
			glm::radians(mvnt_upper_leg),
			glm::vec3(-1.0f, 0.0f, 0.0f)
		);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
			glm::radians(mvnt_lower_leg),
			glm::vec3(-1.0f, 0.0f, 0.0f)
		);
	gripper->rotation = gripper_base_rotation * glm::angleAxis(
			glm::radians(mvnt_gripper),
			glm::vec3(-1.0f, 0.0f, 0.0f)
		);

	glm::vec4 gripper_head_pos_homo = glm::vec4(gripper_head->position, 1.0f);
	glm::uvec4 cube_pos_homo = glm::vec4(cube->position, 1.0f);

	glm::vec3 gripper_head_pos_global = gripper_head->make_local_to_world() * gripper_head_pos_homo;
	glm::vec3 cube_pos_global = cube->make_local_to_world() * cube_pos_homo;
	if (reached_goal(gripper_head_pos_global, cube->position)) {
		std::cout << "Reached goal!" << std::endl;
		place_cube();
	}

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	arrowUp.downs = 0;
	arrowDown.downs = 0;
	arrowLeft.downs = 0;
	arrowRight.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
