#include "ChapterModel.hpp"
#include <QSize>

ChapterModel::ChapterModel(){

}

ChapterModel::~ChapterModel(){

}

QVariant ChapterModel::data(
	const QModelIndex & index, 
	int role) const
{
	if (index.isValid()) {
		if (index.row() < int(datas_.size()) ) {
			auto item = get( index.row() );
			switch (role)
			{
			case ChapterNameRole:
			case Qt::DisplayRole:  return item.getChapterName() ; break;
			case ChapterIndexRole: return item.getChapterIndex(); break;
			case Qt::SizeHintRole: return QSize(32,32); break;
			}

		}
	}
	return QVariant();
}
