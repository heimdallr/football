#include "ViewDelegateStateFocus.h"

#include <QStyledItemDelegate>

using namespace HomeCompa::Football;

class ViewDelegateStateFocus::Impl final : public QStyledItemDelegate
{
private: // QStyledItemDelegate
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
	{
		QStyledItemDelegate::initStyleOption(option, index);

		if (option->state & QStyle::State_Selected)
		{
			option->state |= QStyle::State_HasFocus;

			option->palette.setColor(QPalette::Inactive, QPalette::Highlight, option->palette.color(QPalette::Active, QPalette::Highlight).darker(120));
			option->palette.setColor(QPalette::Inactive, QPalette::HighlightedText, option->palette.color(QPalette::Active, QPalette::HighlightedText));
		}
	}
};

ViewDelegateStateFocus::ViewDelegateStateFocus()  = default;
ViewDelegateStateFocus::~ViewDelegateStateFocus() = default;

QAbstractItemDelegate* ViewDelegateStateFocus::GetDelegate() noexcept
{
	return m_impl.get();
}
