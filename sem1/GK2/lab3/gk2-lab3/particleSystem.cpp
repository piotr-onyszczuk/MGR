#include "particleSystem.h"

#include <iterator>

#include "dxDevice.h"
#include "exceptions.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const XMFLOAT3 ParticleSystem::EMITTER_DIR = XMFLOAT3(0.0f, 1.0f, 0.0f);
const float ParticleSystem::TIME_TO_LIVE = 4.0f;
const float ParticleSystem::EMISSION_RATE = 10.0f;
const float ParticleSystem::MAX_ANGLE = XM_PIDIV2 / 9.0f;
const float ParticleSystem::MIN_VELOCITY = 0.2f;
const float ParticleSystem::MAX_VELOCITY = 0.33f;
const float ParticleSystem::PARTICLE_SIZE = 0.08f;
const float ParticleSystem::PARTICLE_SCALE = 1.0f;
const float ParticleSystem::MIN_ANGLE_VEL = -XM_PI;
const float ParticleSystem::MAX_ANGLE_VEL = XM_PI;
const int ParticleSystem::MAX_PARTICLES = 500;

ParticleSystem::ParticleSystem(DirectX::XMFLOAT3 emmiterPosition)
	: m_emitterPos(emmiterPosition), m_particlesToCreate(0.0f), m_random(random_device{}())
{ }

vector<ParticleVertex> ParticleSystem::Update(float dt, DirectX::XMFLOAT4 cameraPosition)
{
	size_t removeCount = 0;
	for (auto& p : m_particles)
	{
		UpdateParticle(p, dt);
		if (p.Vertex.Age >= TIME_TO_LIVE)
			++removeCount;
	}
	m_particles.erase(m_particles.begin(), m_particles.begin() + removeCount);

	m_particlesToCreate += dt * EMISSION_RATE;
	while (m_particlesToCreate >= 1.0f)
	{
		--m_particlesToCreate;
		if (m_particles.size() < MAX_PARTICLES)
			m_particles.push_back(RandomParticle());
	}
	return GetParticleVerts(cameraPosition);
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
	static const uniform_real_distribution<float> angleDist(0, XM_2PI);
	static const uniform_real_distribution<float> magnitudeDist(0, tan(MAX_ANGLE));
	static const uniform_real_distribution<float> velDist(MIN_VELOCITY, MAX_VELOCITY);
	float angle = angleDist(m_random);
	float magnitude = magnitudeDist(m_random);
	XMFLOAT3 v{ cos(angle)*magnitude, 1.0f, sin(angle)*magnitude };

	auto velocity = XMLoadFloat3(&v);
	auto len = velDist(m_random);
	velocity = len * XMVector3Normalize(velocity);
	XMStoreFloat3(&v, velocity);
	return v;
}

Particle ParticleSystem::RandomParticle()
{
	static const uniform_real_distribution<float> anglularVelDist(MIN_ANGLE_VEL, MAX_ANGLE_VEL);
	Particle p;
	p.Vertex = ParticleVertex();
	p.Vertex.Age = 0;
	p.Vertex.Size = PARTICLE_SIZE;
	p.Vertex.Pos = m_emitterPos;
	p.Vertex.Angle = 0;

	p.Velocities = ParticleVelocities();
	p.Velocities.Velocity = RandomVelocity();
	p.Velocities.AngularVelocity = anglularVelDist(m_random);
	// TODO : 1.27 Setup initial particle properties

	return p;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
	// TODO : 1.28 Update particle properties
	p.Vertex.Age += dt;

	XMVECTOR v1 = XMLoadFloat3(&p.Vertex.Pos);
	XMVECTOR v2 = XMLoadFloat3(&p.Velocities.Velocity);
	XMStoreFloat3(&p.Vertex.Pos, v1 + dt * v2);

	p.Vertex.Angle += dt * p.Velocities.AngularVelocity;
	p.Vertex.Size += PARTICLE_SCALE * PARTICLE_SIZE * dt;
}

struct myclass {
	DirectX::XMFLOAT4 camPos;
	bool operator() (ParticleVertex a, ParticleVertex b) 
	{
		return dist(camPos,a.Pos) < dist(camPos, a.Pos);
	}
	float dist(XMFLOAT4 v1, XMFLOAT3 v2) {
		return sqrt((v1.x-v2.x)* (v1.x - v2.x)+
			(v1.y - v2.y) * (v1.y - v2.y)+
			(v1.z - v2.z) * (v1.z - v2.z)
		);
	}
} myobject;

vector<ParticleVertex> ParticleSystem::GetParticleVerts(DirectX::XMFLOAT4 cameraPosition)
{
	XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);

	vector<ParticleVertex> vertices;
	for (int i = 0; i < m_particles.size(); ++i) vertices.push_back(m_particles[i].Vertex);
	
	myobject.camPos = cameraPosition;
	std::sort(vertices.begin(), vertices.end(), myobject);
	// TODO : 1.29 Copy particles' vertex data to a vector and sort them

	return vertices;
}