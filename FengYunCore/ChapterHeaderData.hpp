#ifndef _Chapter_HEADER_DATA_HPP
#define _Chapter_HEADER_DATA_HPP

#include <QString>
#include <memory>
#include <atomic>

class ChapterHeaderDataCore{
public:
    int chapterIndex    = -1;
    QString chapterName ;/* 章节名   */
	QString fileData    ;/* 章节全文 */
	QString chapterUrl  ;/* 章节网址 */
	std::atomic<bool> isSetData{false};
};

class ChapterHeaderData  {
    typedef std::shared_ptr< ChapterHeaderDataCore > PT;
    PT core_data;
public:

    class Undefine;

    ChapterHeaderData():core_data( PT(new ChapterHeaderDataCore) ){}
    explicit ChapterHeaderData(Undefine * /*never use this*/){  }
    ~ChapterHeaderData(){}

	const std::atomic<bool> & isSetData()const {return core_data->isSetData; }
	std::atomic<bool> & isSetData() { return core_data->isSetData; }

	const QString & getChapterURL()const { return core_data->chapterUrl; }
	QString & getChapterURL() { return core_data->chapterUrl; }

	const QString & getChapterData()const { return core_data->fileData; }
	QString & getChapterData()  { return core_data->fileData; }

    const QString & getChapterName()const{return core_data->chapterName;}
    QString & getChapterName() {return core_data->chapterName;}

    int getChapterIndex()const{return core_data->chapterIndex;}
    int & getChapterIndex() {return core_data->chapterIndex;}

    bool isValid() const{ 
		if ( bool( core_data ) == false) { return false; }
		return getChapterIndex()>=0;
	}
    explicit operator bool() const { return isValid(); }

    friend bool operator<(const ChapterHeaderData & l,const ChapterHeaderData &r){
        return l.getChapterIndex() < r.getChapterIndex();
    }

    friend bool operator==(const ChapterHeaderData & l,const ChapterHeaderData &r){
        return l.getChapterIndex() == r.getChapterIndex();
    }

};

#endif

