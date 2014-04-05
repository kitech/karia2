// 2010年 03月 18日 星期四 13:36:31 CST
#include <QHash>
#include <QString>

QHash<QString, QString> gMimeHash;

static int mimeHashInit() {
    if (!gMimeHash.contains("ez")) gMimeHash.insert("ez", "application-andrew-inset");
    char ** raw_mime = {
        
        NULL
    };
    return 0;
}
static int mimeHashInited = mimeHashInit();

