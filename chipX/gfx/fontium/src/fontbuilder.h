#pragma once

#include "fontconfig.h"
#include "fontrenderer.h"
#include "layoutconfig.h"
//#include "layoutdata.h"
#include "layouterfactory.h"
#include "outputconfig.h"
#include "exporterfactory.h"
#include "imagewriterfactory.h"

class FontBuilder {
public:
    FontBuilder();
    ~FontBuilder();

protected:
private:
    FontConfig*     m_font_config;
    LayoutConfig*   m_layout_config;

    FontRenderer*   m_font_renderer;
//    LayoutData*     m_layout_data;
    AbstractLayouter* m_layouter;
    LayouterFactory*    m_layouter_factory;
    OutputConfig*   m_output_config;
    ExporterFactory* m_exporter_factory;
    ImageWriterFactory* m_image_writer_factory;
    AbstractImageWriter* m_image_writer;

    void doExport(bool x2);

};
