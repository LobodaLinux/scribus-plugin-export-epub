#include <QDebug>

#include "epubexport.h"
#include "scribusdoc.h"

#include "plugins/scribusAPI/scribusAPIDocument.h"
#include "plugins/scribusAPI/scribusAPI.h"
#include "module/epubexportEpubfile.h"
#include "module/epubexportStructure.h"
#include "module/epubexportContent.h"

EpubExport::EpubExport()
{
    // progressDialog = 0;
    // itemNumber = 0;

    // qDebug() << "marksList" << this->doc->get()->marksList();
    // qDebug() << "notesList" << this->doc->get()->notesList();
}

EpubExport::~EpubExport()
{
}

/**
 * - Create the Epub file
 * - Setup `ScribusAPIDocument` with the current document
 * - Fill the document information
 * - Use `ScribusAPIDocument` to read the styles and add them to the Epub's css file.
 * - Use `EpubExportContent` to
 *   - fill the Epub with the content and
 *   - update the structure
 * - Set the Epub's cover
 * - Add the "structure" files.
 */
void EpubExport::doExport()
{
    // qDebug() << "options" << options;

    // if it's my computer, always put the epub file in /tmp
    // TODO: remove before releasing the plugin
    if (QDir::homePath() == "/home/ale")
    {
        options.targetFilename = "/tmp/"+options.targetFilename;
        qDebug() << "forcing the output of the .epub file to /tmp";
    }

    EpubExportEpubfile* epub = new EpubExportEpubfile();
    epub->setFilename(options.targetFilename);
    epub->create();


    // TODO: pass scribusDoc as parameter to ScribusAPIDocument
    ScribusAPIDocument* scribusDocument = new ScribusAPIDocument();
    scribusDocument->set(this->scribusDoc);
    scribusDocument->setPageRange(this->options.pageRange);
    scribusDocument->readItems();
    scribusDocument->readSections();

    EpubExportStructure* structure = new EpubExportStructure();
    structure->setFilename(options.targetFilename);
    structure->readMetadata(scribusDocument->getMetadata());

    epub->add("OEBPS/Styles/style.css", scribusDocument->getStylesAsCss());
    structure->addToManifest("stylesheet", "Styles/style.css", "text/css");

    // TODO: add scribusDocument in the constructor
    EpubExportContent* content = new EpubExportContent();
    content->setDocument(scribusDocument);

    // TODO: check if there is a better way to fill the epub (more flexible and which better
    // shows in here what is going on
    content->fillEpub(epub, structure);

    if (structure->hasCover())
    {
        // TODO: implement a way to set the cover
        epub->addUncompressed("OEBPS/Images/cover.png", structure->getCover());
    } else {
        // epub->addUncompressed("OEBPS/Images/cover.png", scribusDocument->getFirstPageAsCoverImage());
        epub->addUncompressed("OEBPS/Images/cover.png", structure->getFirstPageAsCoverImage(scribusDocument));
    }
    structure->addToManifest("cover.png", "Images/cover.png", "image/png");

	epub->add("META-INF/container.xml", structure->getContainer());

	epub->add("OEBPS/toc.ncx", structure->getNCX());

	epub->add("OEBPS/content.opf", structure->getOPF());

    epub->close();
}

QDebug operator<<(QDebug dbg, const EpubExportOptions options)
{
    dbg.nospace() << "(targetFilename:" << options.targetFilename;
    QStringList output;
    foreach (int pageRange, options.pageRange) {
        output << QString::number(pageRange);
    }
    dbg.nospace() << ", (" << output.join(", ") << ")" << ", imageMaxWidth:" << options.imageMaxWidth  << ", imageMaxWidthThreshold:" << options.imageMaxWidthThreshold << ")";
    return dbg.space();
}
