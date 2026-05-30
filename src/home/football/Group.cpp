#include "ui_Group.h"

#include "Group.h"

using namespace HomeCompa::Football;

namespace
{

}

class Group::Impl
{
public:
	Impl(Group& self, std::shared_ptr<ModelGroup> model)
		: m_self { self }
		, m_model { std::move(model) }
	{
		m_ui.setupUi(&m_self);
	}

	void Init(const int idChamp)
	{
		m_model->setData({}, idChamp, ModelGroup::Role::ChampId);
	}

private:
	Group& m_self;

	PropagateConstPtr<ModelGroup, std::shared_ptr> m_model;

	Ui::Group m_ui {};
};

Group::Group(std::shared_ptr<ModelGroup> model, QWidget* parent)
	: QWidget(parent)
	, m_impl(*this, std::move(model))
{
}

Group::~Group() = default;

void Group::Init(const int idChamp)
{
	m_impl->Init(idChamp);
}
