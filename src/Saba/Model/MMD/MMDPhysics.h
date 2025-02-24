//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMDMODEL_MMDPHYSICS_H_
#define SABA_MODEL_MMDMODEL_MMDPHYSICS_H_

#include "PMDFile.h"
#include "PMXFile.h"

#include <memory>

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
class btConstraintSolver;
class btThreads;

namespace saba
{
	class MMDPhysics;
	class MMDModel;
	class MMDNode;

	class MMDMotionState;

	/**
	 * @brief Class representing a MMD Rigid Body.
	 */
	class MMDRigidBody
	{
	public:
		MMDRigidBody();
		~MMDRigidBody();
		MMDRigidBody(const MMDRigidBody& rhs) = delete;
		MMDRigidBody& operator = (const MMDRigidBody& rhs) = delete;

		/**
		 * @brief Create a rigid body from PMD data.
		 * @param pmdRigidBody PMD rigid body data.
		 * @param model Pointer to the MMD model.
		 * @param node Pointer to the MMD node.
		 * @return True if creation is successful, false otherwise.
		 */
		bool Create(const PMDRigidBodyExt& pmdRigidBody, MMDModel* model, MMDNode* node);
		/**
		 * @brief Create a rigid body from PMX data.
		 * @param pmxRigidBody PMX rigid body data.
		 * @param model Pointer to the MMD model.
		 * @param node Pointer to the MMD node.
		 * @return True if creation is successful, false otherwise.
		 */
		bool Create(const PMXRigidbody& pmxRigidBody, MMDModel* model, MMDNode* node);
		/**
		 * @brief Destroy the rigid body.
		 */
		void Destroy();

		/**
		 * @brief Get the Bullet rigid body.
		 * @return Pointer to the Bullet rigid body.
		 */
		btRigidBody* GetRigidBody() const;
		/**
		 * @brief Get the collision group.
		 * @return Collision group.
		 */
		uint16_t GetGroup() const;
		/**
		 * @brief Get the collision group mask.
		 * @return Collision group mask.
		 */
		uint16_t GetGroupMask() const;

		/**
		 * @brief Set the activation state of the rigid body.
		 * @param activation Activation state.
		 */
		void SetActivation(bool activation) const;
		/**
		 * @brief Reset the transform of the rigid body.
		 */
		void ResetTransform() const;
		/**
		 * @brief Reset the rigid body with the given physics.
		 * @param physics Pointer to the MMD physics.
		 */
		void Reset(const MMDPhysics* physics) const;

		/**
		 * @brief Reflect the global transform to the rigid body.
		 */
		void ReflectGlobalTransform() const;
		/**
		 * @brief Calculate the local transform of the rigid body.
		 */
		void CalcLocalTransform() const;

		/**
		 * @brief Get the transform matrix of the rigid body.
		 * @return Transform matrix.
		 */
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

	/**
	 * @brief Class representing a MMD Joint.
	 */
	class MMDJoint
	{
	public:
		MMDJoint();
		~MMDJoint();
		MMDJoint(const MMDJoint& rhs) = delete;
		MMDJoint& operator = (const MMDJoint& rhs) = delete;

		/**
		 * @brief Create a joint from PMD data.
		 * @param pmdJoint PMD joint data.
		 * @param rigidBodyA Pointer to the first rigid body.
		 * @param rigidBodyB Pointer to the second rigid body.
		 * @return True if creation is successful, false otherwise.
		 */
		bool CreateJoint(const PMDJointExt& pmdJoint, const MMDRigidBody* rigidBodyA, const MMDRigidBody* rigidBodyB);
		/**
		 * @brief Create a joint from PMX data.
		 * @param pmxJoint PMX joint data.
		 * @param rigidBodyA Pointer to the first rigid body.
		 * @param rigidBodyB Pointer to the second rigid body.
		 * @return True if creation is successful, false otherwise.
		 */
		bool CreateJoint(const PMXJoint& pmxJoint, const MMDRigidBody* rigidBodyA, const MMDRigidBody* rigidBodyB);
		/**
		 * @brief Destroy the joint.
		 */
		void Destroy();

		/**
		 * @brief Get the Bullet constraint.
		 * @return Pointer to the Bullet constraint.
		 */
		btTypedConstraint* GetConstraint() const;

	private:
		std::unique_ptr<btTypedConstraint>	m_constraint;
	};

	/**
	 * @brief Class representing MMD Physics.
	 */
	class MMDPhysics
	{
	public:
		MMDPhysics();
		~MMDPhysics();

		MMDPhysics(const MMDPhysics& rhs) = delete;
		MMDPhysics& operator = (const MMDPhysics& rhs) = delete;

		/**
		 * @brief Create the physics world.
		 * @return True if creation is successful, false otherwise.
		 */
		bool Create();
		/**
		 * @brief Destroy the physics world.
		 */
		void Destroy();

		/**
		 * @brief Set the frames per second (FPS) for the physics simulation.
		 * @param fps Frames per second.
		 */
		void SetFPS(float fps);
		/**
		 * @brief Get the frames per second (FPS) for the physics simulation.
		 * @return Frames per second.
		 */
		float GetFPS() const;
		/**
		 * @brief Set the maximum number of sub-steps for the physics simulation.
		 * @param numSteps Maximum number of sub-steps.
		 */
		void SetMaxSubStepCount(int numSteps);
		/**
		 * @brief Get the maximum number of sub-steps for the physics simulation.
		 * @return Maximum number of sub-steps.
		 */
		int GetMaxSubStepCount() const;
		/**
		 * @brief Update the physics simulation.
		 * @param time Time step for the update.
		 */
		void Update(float time) const;

		/**
		 * @brief Add a rigid body to the physics world.
		 * @param mmdRB Pointer to the MMD rigid body.
		 */
		void AddRigidBody(const MMDRigidBody* mmdRB) const;
		/**
		 * @brief Remove a rigid body from the physics world.
		 * @param mmdRB Pointer to the MMD rigid body.
		 */
		void RemoveRigidBody(const MMDRigidBody* mmdRB) const;
		/**
		 * @brief Add a joint to the physics world.
		 * @param mmdJoint Pointer to the MMD joint.
		 */
		void AddJoint(const MMDJoint* mmdJoint) const;
		/**
		 * @brief Remove a joint from the physics world.
		 * @param mmdJoint Pointer to the MMD joint.
		 */
		void RemoveJoint(const MMDJoint* mmdJoint) const;

		/**
		 * @brief Get the Bullet dynamics world.
		 * @return Pointer to the Bullet dynamics world.
		 */
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

		int                                                  m_maxThreadCount;
		double	m_fps;
		int		m_maxSubStepCount;
	};

}
#endif // SABA_MODEL_MMDMODEL_MMDPHYSICS_H_

