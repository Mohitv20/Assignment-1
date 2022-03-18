#include "Gameplay/Components/CharacterMovement.h"
#include "Gameplay/InputEngine.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void CharacterMovement::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void CharacterMovement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "move", &_move, 1.0f);
}

nlohmann::json CharacterMovement::ToJson() const {
	return {
		{ "move", _move }
	};
}
CharacterMovement::CharacterMovement() :
	IComponent(),
	_move(5.0f)
{ }

CharacterMovement::~CharacterMovement() = default;

CharacterMovement::Sptr CharacterMovement::FromJson(const nlohmann::json & blob)
{
	CharacterMovement::Sptr result = std::make_shared<CharacterMovement>();
	result->_move = blob["move"];
	return result;
}
void CharacterMovement::Update(float x) //Character movement WASD
{
	if (InputEngine::GetKeyState(GLFW_KEY_W) == ButtonState::Down) {
		_body->SetLinearVelocity(glm::vec3(6.0f, 0.0f, 0.0f));
		}
	if (InputEngine::GetKeyState(GLFW_KEY_A) == ButtonState::Down) {
		_body->SetLinearVelocity(glm::vec3(0.0f, 6.0f, 0.0f));
	}
	if (InputEngine::GetKeyState(GLFW_KEY_S) == ButtonState::Down) {
		_body->SetLinearVelocity(glm::vec3(-6.0f, 0.0f, 0.0f));
	}
	if (InputEngine::GetKeyState(GLFW_KEY_D) == ButtonState::Down) {
		_body->SetLinearVelocity(glm::vec3(0.0f, -6.0f, 0.0f));
	}
}
