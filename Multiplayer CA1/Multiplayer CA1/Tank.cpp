#include "Tank.hpp"

#include <iostream>
#include <SFML/Graphics/RenderTarget.hpp>

#include "DataTables.hpp"
#include "Projectile.hpp"
#include "ProjectileType.hpp"
#include "ResourceHolder.hpp"

namespace
{
	const std::vector<TankData> Table = InitializeTankData();
}

Tank::Tank(TankType type, const TextureHolder& textures)
: Entity(Table[static_cast<int>(type)].m_hitpoints)
, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture))
, m_type(type)
, m_fire_interval(Table[static_cast<int>(type)].m_fire_interval)
{
	setScale(5, 5);
	Utility::CentreOrigin(m_sprite);
	if (type == TankType::kPlayer1Tank) FaceDirection(Utility::Down);
	else FaceDirection(Utility::Up);

	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time time)
	{
		CreateProjectile(node, ProjectileType::kAlliedBullet, textures);
	};
	m_fire_command.category = static_cast<int>(Category::Type::kScene);
}

unsigned Tank::GetCategory() const
{
	return m_type == TankType::kPlayer1Tank ? Category::kPlayer1Tank : Category::kPlayer2Tank;
}

void Tank::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

//Tank will not keep firing when held down
void Tank::Fire()
{
	m_is_firing = true;
}

void Tank::CreateProjectile(SceneNode& node, ProjectileType type, const TextureHolder& textures) const
{
	std::cout << "Creating projectile " << static_cast<int>(type) << std::endl;
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));
	sf::Vector2f offset = GetWorldTransform().transformPoint(5.f,-1.f);
	offset -= getPosition();
	sf::Vector2f velocity(
		std::cosf(Utility::ToRadians(getRotation())) * projectile->GetMaxSpeed(), 
		std::sinf(Utility::ToRadians(getRotation())) * projectile->GetMaxSpeed());
	
	projectile->setPosition(GetWorldPosition() + offset);
	projectile->SetVelocity(velocity);
	projectile->setRotation(getRotation() + 90);
	node.AttachChild(std::move(projectile));
}

float Tank::GetMaxSpeed()
{
	return Table[static_cast<int>(m_type)].m_speed;
}

void Tank::Repair(int health)
{
	Entity::Repair(health);
}

void Tank::FaceDirection(Utility::Direction dir)
{
	switch (dir)
	{
	case Utility::Right:
		setRotation(0);
		break;
	case Utility::Up:
		setRotation(-90.f);
		break;
	case Utility::Left:
		setRotation(180.f);
		break;
	case Utility::Down:
		setRotation(90.f);
		break;
	}
}

void Tank::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	Entity::UpdateCurrent(dt, commands);
	if(m_is_firing)
	{
		if (m_fire_cooldown.asSeconds() <= 0.f) {
			commands.Push(m_fire_command);
			m_fire_cooldown = m_fire_interval;
		}
		m_is_firing = false;
	}
	if (m_fire_cooldown.asSeconds() > 0.f) m_fire_cooldown -= dt;
}
