
#include "fontbuilder.h"



FontBuilder::FontBuilder() : m_image_writer(0)
{
    m_font_config = new FontConfig();
    m_font_renderer = new FontRenderer(m_font_config);
    m_layout_config = new LayoutConfig();
    m_layout_data = new LayoutData();
    m_layouter = 0;
    m_layouter_factory = new LayouterFactory();
    m_output_config = new OutputConfig();
    m_exporter_factory = new ExporterFactory();
    m_image_writer_factory = new ImageWriterFactory();

//    m_font_config->emmitChange();

//    m_font_loader = new FontLoader(this);
}

FontBuilder::~FontBuilder()
{
}


//void FontBuilder::on_comboBoxLayouter_currentIndexChanged(QString name)
//{
//    if (name.isEmpty()) return;
//    if (m_layouter) {
//        delete m_layouter;
//        m_layouter = 0;
//    }
//    m_layouter = m_layouter_factory->build(name,this);
//    if (m_layouter) {
//        m_layouter->setConfig(m_layout_config);
//        m_layouter->setData(m_layout_data);
//        m_layouter->on_ReplaceImages(m_font_renderer->rendered());
//        m_layout_config->setLayouter(name);
//    }
//}

void FontBuilder::doExport(bool x2) {
//    QDir dir(m_output_config->path());
//    QString texture_filename;
//    setLayoutImage(m_layout_data->image());
//    if (m_output_config->writeImage()) {
//        delete m_image_writer;
//        m_image_writer = 0;
//        AbstractImageWriter* exporter = m_image_writer_factory->build(m_output_config->imageFormat(),this);
//        if (!exporter) {
//            QMessageBox msgBox;
//            msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
//            msgBox.exec();
//            return;
//        }
//
//        // image writer takes a set of rendered fonts and their layout data positions and creates an image
//        exporter->setData(m_layout_data,m_layout_config,m_font_renderer->data());
//        texture_filename = m_output_config->imageName();
//        if (x2) {
//            texture_filename += "_x2";
//        }
//        texture_filename+="."+exporter->extension();
//        QString filename = dir.filePath(texture_filename);
//
//        QFile file(this);
//        file.setFileName(filename);
//        if (!file.open(QIODevice::WriteOnly)) {
//            delete exporter;
//            QMessageBox msgBox;
//            msgBox.setText(tr("Error opening file :")+filename);
//            msgBox.exec();
//            return;
//        }
//        if (!exporter->Write(file)) {
//            QMessageBox msgBox;
//            msgBox.setText(tr("Error on save image :\n")+exporter->errorString()+"\nFile not writed.");
//            msgBox.exec();
//        }
//        file.close();
//        m_image_writer = exporter;
//        m_image_writer->watch(filename);
//        connect(m_image_writer,SIGNAL(imageChanged(QString)),this,SLOT(onExternalImageChanged(QString)));
//    }
//    if (m_output_config->writeDescription()) {
//        AbstractExporter* exporter = m_exporter_factory->build(m_output_config->descriptionFormat(),this);
//        if (!exporter) {
//            QMessageBox msgBox;
//            msgBox.setText(tr("Unknown exporter :")+m_output_config->descriptionFormat());
//            msgBox.exec();
//            return;
//        }
//        exporter->setFace(m_font_renderer->face());
//        exporter->setFontConfig(m_font_config,m_layout_config);
//        exporter->setData(m_layout_data,m_font_renderer->data());
//        exporter->setTextureFilename(texture_filename);
//        exporter->setScale(m_font_renderer->scale());
//        QString filename = dir.filePath(m_output_config->descriptionName());
//        if (x2) {
//            filename += "_x2";
//        }
//        filename+="."+exporter->getExtension();
//        QByteArray data;
//        if (!exporter->Write(data)) {
//             QMessageBox msgBox;
//             msgBox.setText(tr("Error on save description :\n")+exporter->getErrorString()+"\nFile not writed.");
//             msgBox.exec();
//         } else {
//             QFile file(filename);
//             if (file.open(QIODevice::WriteOnly)) {
//                 file.write(data);
//             } else {
//
//             }
//         }
//        delete exporter;
    }


