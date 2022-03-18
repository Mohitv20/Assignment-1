#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

//Controller for Player 1 using WASD
class CharacterMovement : public Gameplay::IComponent 
{
public:
	typedef std::shared_ptr<CharacterMovement> Sptr;

	CharacterMovement(); //Controls
	virtual ~CharacterMovement();

	virtual void Awake() override;
	virtual void Update(float x) override; //update for WASD

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(CharacterMovement);
	virtual nlohmann::json ToJson() const override;
	static CharacterMovement::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _move;
	Gameplay::Physics::RigidBody::Sptr _body;
};