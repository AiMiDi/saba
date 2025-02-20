//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMDMODEL_MMDPHYSICS_H_
#define SABA_MODEL_MMDMODEL_MMDPHYSICS_H_

#include "PMDFile.h"
#include "PMXFile.h"

#include <memory>
#include <cinttypes>

// Bullet Types
class btRigidBody;
class btCollisionShape;
class btTypedConstraint;
class btDiscreteDynamicsWorld;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btMotionState;
struct btOverlapFilterCallback;

namespace saba
{
	class MMDPhysics;
	class MMDModel;
	class MMDNode;

	class MMDMotionState;

	class MMDRigidBody
	{
	public:
		MMDRigidBody();
		~MMDRigidBody();
		MMDRigidBody(const MMDRigidBody& rhs) = delete;
		MMDRigidBody& operator = (const MMDRigidBody& rhs) = delete;

		bool Create(const PMDRigidBodyExt& pmdRigidBody, MMDModel* model, MMDNode* node);
		bool Create(const PMXRigidbody& pmxRigidBody, MMDModel* model, MMDNode* node);
		void Destroy();

		btRigidBody* GetRigidBody() const;
		uint16_t GetGroup() const;
		uint16_t GetGroupMask() const;

		void SetActivation(bool activation) const;
		void ResetTransform() const;
		void Reset(const MMDPhysics* physics) const;

		void ReflectGlobalTransform() const;
		void CalcLocalTransform() const;

		glm::mat4 GetTransform() const;

	private:
		enum class RigidBodyType
		{
			Kinematic,
			Dynamic,
			Aligned,
		};

		std::unique_ptr<btCollisionShape>	m_shape;
		std::unique_ptr<MMDMotionState>		m_activeMotionState;
		std::unique_ptr<MMDMotionState>		m_kinematicMotionState;
		std::unique_ptr<btRigidBody>		m_rigidBody;

		RigidBodyType	m_rigidBodyType;
		uint16_t		m_group;
		uint16_t		m_groupMask;

		MMDNode*	m_node;
		glm::mat4	m_offsetMat;

		std::string					m_name;
	};

	class MMDJoint
	{
	public:
		MMDJoint();
		~MMDJoint();
		MMDJoint(const MMDJoint& rhs) = delete;
		MMDJoint& operator = (const MMDJoint& rhs) = delete;

		bool CreateJoint(const PMDJointExt& pmdJoint, const MMDRigidBody* rigidBodyA, const MMDRigidBody* rigidBodyB);
		bool CreateJoint(const PMXJoint& pmxJoint, const MMDRigidBody* rigidBodyA, const MMDRigidBody* rigidBodyB);
		void Destroy();

		btTypedConstraint* GetConstraint() const;

	private:
		std::unique_ptr<btTypedConstraint>	m_constraint;
	};

	class MMDPhysics
	{
	public:
		MMDPhysics();
		~MMDPhysics();

		MMDPhysics(const MMDPhysics& rhs) = delete;
		MMDPhysics& operator = (const MMDPhysics& rhs) = delete;

		bool Create();
		void Destroy();

		void SetFPS(float fps);
		float GetFPS() const;
		void SetMaxSubStepCount(int numSteps);
		int GetMaxSubStepCount() const;
		void Update(float time) const;

		void AddRigidBody(const MMDRigidBody* mmdRB) const;
		void RemoveRigidBody(const MMDRigidBody* mmdRB) const;
		void AddJoint(const MMDJoint* mmdJoint) const;
		void RemoveJoint(const MMDJoint* mmdJoint) const;

		btDiscreteDynamicsWorld* GetDynamicsWorld() const;

	private:
		std::unique_ptr<btBroadphaseInterface>				m_broadphase;
		std::unique_ptr<btDefaultCollisionConfiguration>	m_collisionConfig;
		std::unique_ptr<btCollisionDispatcher>				m_dispatcher;
		std::unique_ptr<btSequentialImpulseConstraintSolver>	m_solver;
		std::unique_ptr<btDiscreteDynamicsWorld>			m_world;
		std::unique_ptr<btCollisionShape>					m_groundShape;
		std::unique_ptr<btMotionState>						m_groundMS;
		std::unique_ptr<btRigidBody>						m_groundRB;
		std::unique_ptr<btOverlapFilterCallback>			m_filterCB;

		double	m_fps;
		int		m_maxSubStepCount;
	};

}
#endif // SABA_MODEL_MMDMODEL_MMDPHYSICS_H_

