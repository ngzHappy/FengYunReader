#ifndef CHAPTERMODEL_HPP
#define CHAPTERMODEL_HPP

#include "ChapterHeaderData.hpp"
#include <QAbstractListModel>
#include <vector>
#include <cstddef>
#include <cstdbool>
#include <cstdint>

class ChapterModel :
        public QAbstractListModel
{
    Q_OBJECT
private:
	std::vector< ChapterHeaderData > datas_;
	ChapterHeaderData valid;
public:

	enum {
		ChapterNameRole = Qt::UserRole+1,
		ChapterIndexRole,
	};

    ChapterModel();
    ~ChapterModel();

	ChapterHeaderData get( std::size_t i ) {
		if (i < datas_.size()) { return datas_[i]; }
		return valid;
	}

	const ChapterHeaderData get(std::size_t i) const {
		if (i < datas_.size()) { return datas_[i]; }
		return valid;
	}

	virtual int rowCount(const QModelIndex & ) const override {
		return int( datas_.size() );
	}
    int size() const { return int( datas_.size() ) ; }
	QVariant data(const QModelIndex &index, int role) const override;

	template<typename It_b >
	void resetModelData(It_b b,It_b e) {
		this->beginResetModel();
		datas_.clear();
		datas_.assign( b ,e );
		this->endResetModel();
	}

	template<typename It_b >
	void resetModelData(std::size_t size_,It_b b, It_b e) {
		this->beginResetModel();
		datas_.clear();
		datas_.reserve(size_);
		for (; b != e; ++b) { datas_.push_back(*b); }
		this->endResetModel();
	}

private:
	ChapterModel & operator=(const ChapterModel &) =delete;
	ChapterModel & operator=(ChapterModel &&) =delete;

	ChapterModel(const ChapterModel &) = delete;
	ChapterModel(ChapterModel &&) = delete;

};

#endif // CHAPTERMODEL_HPP
