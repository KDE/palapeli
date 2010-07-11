/***************************************************************************
 *   Copyright  2010 Johannes Loehnert <loehnert.kde@gmx.net>
 * based on the Jigsaw slicer (c) 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************************/

#include "slicer-goldberg.h"

#include <QMessageBox>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

#include "goldberg-engine.h"
#include "grid.h"
#include "utilities.h"

K_PLUGIN_FACTORY(SvgSlicerFactory, registerPlugin<GoldbergSlicer>();)
K_EXPORT_PLUGIN(SvgSlicerFactory("palapeli_goldbergslicer"))

GoldbergSlicer::GoldbergSlicer(QObject* parent, const QVariantList& args)
            : Pala::Slicer(parent, args) {

    Pala::IntegerProperty* prop;
    Pala::BooleanProperty* bprop;

    m_qvoronoi_available = IrregularMode::checkForQVoronoi();

    addMode(new RectMode);
    addMode(new CairoMode);
    addMode(new HexMode);
    addMode(new RotrexMode);
    Pala::SlicerMode* irregularMode = 0;
    if (m_qvoronoi_available)
        addMode(irregularMode = new IrregularMode);

    prop = new Pala::IntegerProperty(i18n("Approx. piece count"));
    prop->setRange(2, 2000);
    prop->setDefaultValue(30);
    prop->setRepresentation(Pala::IntegerProperty::SpinBox);
    addProperty("020_PieceCount", prop);

    prop = new Pala::IntegerProperty(i18n("Flipped edge percentage"));
    prop->setRange(0, 100);
    prop->setDefaultValue(10);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("030_FlipThreshold", prop);

    prop = new Pala::IntegerProperty(i18n("Edge curviness"));
    prop->setRange(-100, 100);
    prop->setDefaultValue(-30);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("040_EdgeCurviness", prop);

    prop = new Pala::IntegerProperty(i18n("Plug size"));
    prop->setRange(-50, 50);
    prop->setDefaultValue(0);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("050_PlugSize", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of curviness"));
    prop->setRange(25, 100);
    prop->setDefaultValue(50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("055_SigmaCurviness", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of plug position"));
    prop->setRange(25, 100);
    prop->setDefaultValue(50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("056_SigmaBasepos", prop);
 
    prop = new Pala::IntegerProperty(i18n("Diversity of plugs"));
    prop->setRange(25, 100);
    prop->setDefaultValue(50);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    addProperty("057_SigmaPlugs", prop);

    prop = new Pala::IntegerProperty(i18n("Diversity of piece size"));
    prop->setRange(0, 30);
    prop->setDefaultValue(15);
    prop->setRepresentation(Pala::IntegerProperty::Slider);
    prop->setEnabled(false);
    if (irregularMode)
        irregularMode->setPropertyEnabled("058_IrrPieceSizeDiversity", true);
    addProperty("058_IrrPieceSizeDiversity", prop);

    bprop = new Pala::BooleanProperty(i18n("Draw piece outlines"));
    bprop->setDefaultValue(true);
    addProperty("060_Outlines", bprop);

    bprop = new Pala::BooleanProperty(i18n("Dump grid image"));
    bprop->setDefaultValue(false);
    addProperty("070_DumpGrid", bprop);

 }

bool GoldbergSlicer::run(Pala::SlicerJob* job) {
    //read job

    GoldbergEngine engine = GoldbergEngine(job);

    int piece_count = job->argument("020_PieceCount").toInt();
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

    engine.m_outlines = job->argument("060_Outlines").toBool();

    engine.set_dump_grid(job->argument("070_DumpGrid").toBool());

    engine.m_alternate_flip = (engine.m_flip_threshold > 50);
    engine.m_unresolved_collisions = 0;
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
