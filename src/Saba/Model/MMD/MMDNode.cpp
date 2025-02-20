//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <stack>
#include "MMDNode.h"
#include <Saba/Base/Log.h>
#include <glm/gtc/matrix_transform.hpp>

namespace saba
{
	MMDNode::MMDNode()
		: m_index(0)
		, m_enableIK(false)
		, m_parent(nullptr)
		, m_child(nullptr)
		, m_next(nullptr)
		, m_prev(nullptr)
		, m_translate(0)
		, m_rotate(1, 0, 0, 0)
		, m_scale(1)
		, m_animTranslate(0)
		, m_animRotate(1, 0, 0, 0)
		, m_baseAnimTranslate(0)
		, m_baseAnimRotate(1, 0, 0, 0)
		, m_ikRotate(1, 0, 0, 0)
		, m_local(1)
		, m_global(1)
		, m_inverseInit(1)
		, m_initTranslate(0)
		, m_initRotate(1, 0, 0, 0)
		, m_initScale(1)
	{
	}

	void MMDNode::AddChild(MMDNode * child)
	{
		SABA_ASSERT(child != nullptr);
		if (child == nullptr)
		{
			return;
		}

		SABA_ASSERT(child->m_parent == nullptr);
		SABA_ASSERT(child->m_next == nullptr);
		SABA_ASSERT(child->m_prev == nullptr);
		child->m_parent = this;
		if (m_child == nullptr)
		{
			m_child = child;
			m_child->m_next = nullptr;
			m_child->m_prev = m_child;
		}
		else
		{
			const auto lastNode = m_child->m_prev;
			lastNode->m_next = child;
			child->m_prev = lastNode;

			m_child->m_prev = child;
		}
	}

	void MMDNode::BeginUpdateTransform()
	{
		LoadInitialTRS();
		SetIKRotate(glm::quat(1, 0, 0, 0));
		OnBeginUpdateTransform();
	}

	void MMDNode::EndUpdateTransform()
	{
		OnEndUpdateTransfrom();
	}

	void MMDNode::UpdateLocalTransform()
	{
		OnUpdateLocalTransform();
	}

void MMDNode::UpdateGlobalTransform()
{
	std::stack<MMDNode*> nodeStack;
	nodeStack.push(this);

	while (!nodeStack.empty())
	{
		MMDNode* currentNode = nodeStack.top();
		nodeStack.pop();

		if (currentNode->m_parent == nullptr)
		{
			currentNode->m_global = currentNode->m_local;
		}
		else
		{
			currentNode->m_global = currentNode->m_parent->m_global * currentNode->m_local;
		}

		MMDNode* child = currentNode->m_child;
		while (child != nullptr)
		{
			nodeStack.push(child);
			child = child->m_next;
		}
	}
}

	void MMDNode::UpdateChildTransform() const
	{
		MMDNode* child = m_child;
		while (child != nullptr)
		{
			child->UpdateGlobalTransform();
			child = child->m_next;
		}
	}

	void MMDNode::CalculateInverseInitTransform()
	{
		m_inverseInit = inverse(m_global);
	}

	void MMDNode::OnBeginUpdateTransform()
	{
	}

	void MMDNode::OnEndUpdateTransfrom()
	{
	}

	void MMDNode::OnUpdateLocalTransform()
	{
		const auto s = scale(glm::mat4(1), GetScale());
		auto r = mat4_cast(AnimateRotate());
		const auto t = translate(glm::mat4(1), AnimateTranslate());
		if (m_enableIK)
		{
			r = mat4_cast(m_ikRotate) * r;
		}
		m_local = t * r * s;
	}

}
