#ifndef JSONFONTEXPORTER_H
#define JSONFONTEXPORTER_H

#include "src/AbstractExporter.h"

class JSONFontExporter : public AbstractExporter
{
    Q_OBJECT
public:
    explicit JSONFontExporter(QObject *parent = 0);

    virtual bool Export(QByteArray& out);
signals:

public slots:

};

#endif // JSONFONTEXPORTER_H
