/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.net>
    Based on the Jigsaw slicer by:
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slicer-goldberg.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include "goldberg-engine.h"
#include "grid.h"
#include "utilities.h"

K_PLUGIN_CLASS_WITH_JSON(GoldbergSlicer, "palapeli_goldbergslicer.json")

GoldbergSlicer::GoldbergSlicer(QObject* parent, const QVariantList& args)
            : Pala::Slicer(parent, args) {

    Pala::IntegerProperty* prop;
    Pala::StringProperty* sprop;
    Pala::BooleanProperty* bprop;

    m_qvoronoi_available = IrregularMode::checkForQVoronoi();

    Pala::SlicerMode* presetMode = nullptr;
    addMode(presetMode = new PresetMode);
    addMode(new RectMode);
    addMode(new CairoMode);
    addMode(new HexMode);
    addMode(new RotrexMode);
    Pala::SlicerMode* irregularMode = nullptr;
    if (m_qvoronoi_available)
        addMode(irregularMode = new IrregularMode);

    prop = new Pala::IntegerProperty(i18n("Approx. piece count"));
    prop->setRange(2, 2000);
    prop->setDefaultValue(30);
    prop->setRepresentation(Pala::IntegerProperty::SpinBox);
    addProperty("020_PieceCount", prop);
    
    sprop = new Pala::StringProperty(i18n("Quick preset"));
    QVariantList choices;
    choices << i18nc("Puzzle shape preset", "Ordinary");
    choices << i18nc("Puzzle shape preset", "Very regular");
    choices << i18nc("Puzzle shape preset", "Very diverse");
    choices << i18nc("Puzzle shape preset", "Large plugs");
    sprop->setChoices(choices);
    sprop->setDefaultValue(QLatin1String(""));
    sprop->setEnabled(false);
    presetMode->setPropertyEnabled("025_QuickPreset", true);
    addProperty("025_QuickPreset", sprop);

    prop = new Pala::IntegerProperty(i18n("Flipped edge percentage"));
    prop->setRange(0, 100);
    prop->setDefaultValue(10);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    presetMode->setPropertyEnabled("030_FlipThreshold", false);
    addProperty("030_FlipThreshold", prop);

    prop = new Pala::IntegerProperty(i18n("Edge curviness"));
    prop->setRange(-100, 100);
    prop->setDefaultValue(-50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("040_EdgeCurviness", prop);

    prop = new Pala::IntegerProperty(i18n("Plug size"));
    prop->setRange(-50, 50);
    prop->setDefaultValue(0);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    presetMode->setPropertyEnabled("050_PlugSize", false);
    addProperty("050_PlugSize", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of curviness"));
    prop->setRange(25, 100);
    prop->setDefaultValue(50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    presetMode->setPropertyEnabled("055_SigmaCurviness", false);
    addProperty("055_SigmaCurviness", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of plug position"));
    prop->setRange(20, 100);
    prop->setDefaultValue(35);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    presetMode->setPropertyEnabled("056_SigmaBasepos", false);
    addProperty("056_SigmaBasepos", prop);
 
    prop = new Pala::IntegerProperty(i18n("Diversity of plugs"));
    prop->setRange(25, 100);
    prop->setDefaultValue(50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    presetMode->setPropertyEnabled("057_SigmaPlugs", false);
    addProperty("057_SigmaPlugs", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of piece size"));
    prop->setRange(0, 30);
    prop->setDefaultValue(15);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    prop->setEnabled(false);
    if (irregularMode)
        irregularMode->setPropertyEnabled("058_IrrPieceSizeDiversity", true);
    addProperty("058_IrrPieceSizeDiversity", prop);

    bprop = new Pala::BooleanProperty(i18n("Dump grid image"));
    bprop->setDefaultValue(false);
    presetMode->setPropertyEnabled("070_DumpGrid", false);
    addProperty("070_DumpGrid", bprop);

 }

bool GoldbergSlicer::run(Pala::SlicerJob* job) {
    //read job

    GoldbergEngine engine = GoldbergEngine(job);

    int piece_count = job->argument("020_PieceCount").toInt();
    engine.m_quickpreset = 0;
    // FIXME: this is not pretty :-/
    QString qptext = job->argument("025_QuickPreset").toString();
    // ordinary == 0 == default
    if (qptext == i18nc("Puzzle shape preset", "Very regular")) engine.m_quickpreset = 1;
    if (qptext == i18nc("Puzzle shape preset", "Very diverse")) engine.m_quickpreset = 2;
    if (qptext == i18nc("Puzzle shape preset", "Large plugs")) engine.m_quickpreset = 3;
    
    engine.m_flip_threshold = job->argument("030_FlipThreshold").toInt();
    engine.m_edge_curviness = job->argument("040_EdgeCurviness").toInt();
    engine.m_plug_size = 1 + 0.01*job->argument("050_PlugSize").toInt();
    engine.m_sigma_curviness = 0.01*job->argument("055_SigmaCurviness").toInt();
    engine.m_sigma_basepos = 0.01*job->argument("056_SigmaBasepos").toInt();
    engine.m_sigma_plugs = 0.01*job->argument("057_SigmaPlugs").toInt();
    engine.m_irregular_relaxation_steps = 30 - job->argument("058_IrrPieceSizeDiversity").toInt();

    // square the sigmas, so that lower values are more stretched out on the slider
    engine.m_sigma_curviness *= engine.m_sigma_curviness;
    engine.m_sigma_basepos *= engine.m_sigma_basepos;
    engine.m_sigma_plugs *= engine.m_sigma_plugs;

    // outline rendering now done by palapeli itself. Outline option was removed.
    engine.m_outlines = false;

    engine.set_dump_grid(job->argument("070_DumpGrid").toBool());

    engine.m_alternate_flip = (engine.m_flip_threshold > 50);
    if (engine.m_alternate_flip) engine.m_flip_threshold = 100-engine.m_flip_threshold;

    //determine selected mode, and call grid generation algorithm
    const GoldbergMode* mode = static_cast<const GoldbergMode*>(job->mode());
    if (!mode)
        return false;
    mode->generateGrid(&engine, piece_count);

    engine.dump_grid_image();

    return true;
}



#include "slicer-goldberg.moc"
