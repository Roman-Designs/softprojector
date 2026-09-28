#include <QApplication>
#include <cassert>

#include "../src/headers/imagegenerator.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    BibleSettings settings;
    settings.useShadow = false;
    settings.versions.primaryBible = "1";
    settings.versions.secondaryBible = "none";
    settings.versions.trinaryBible = "none";

    Verse verse;
    verse.primary_caption = QString::fromUtf8("Луки 12:20");
    verse.primary_text = QString::fromUtf8("А Бог сказав йому: Нерозумний! Цієї ночі душу твою зажадають від тебе. ")
            + QString::fromUtf8("приготувавкомувонобуде").repeated(8);
    Verse changed = verse;
    changed.primary_text.chop(1);
    changed.primary_text += "X";

    ImageGenerator generator;
    generator.setScreenSize(QSize(1280, 720));
    for (int count = 1; count <= 3; ++count) {
        if (count == 2) {
            settings.versions.secondaryBible = "2";
            verse.secondary_text = verse.primary_text;
            verse.secondary_caption = verse.primary_caption;
            changed.secondary_text = changed.primary_text;
            changed.secondary_caption = changed.primary_caption;
        } else if (count == 3) {
            settings.versions.trinaryBible = "3";
            verse.trinary_text = verse.primary_text;
            verse.trinary_caption = verse.primary_caption;
            changed.trinary_text = changed.primary_text;
            changed.trinary_caption = changed.primary_caption;
        }

        // The final character must be visible, even after a long unbreakable run.
        assert(generator.generateBibleImage(verse, settings).toImage() !=
               generator.generateBibleImage(changed, settings).toImage());
    }
}
