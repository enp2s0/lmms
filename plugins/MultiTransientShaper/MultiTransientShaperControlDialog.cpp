/*
 * MultiTransientShaperControlDialog.cpp - control dialog for MultiTransientShaper effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "MultiTransientShaperControlDialog.h"
#include "MultiTransientShaperControls.h"
#include "embed.h"
#include "Knob.h"

#include <QMouseEvent>
#include <QPainter>

namespace lmms::gui
{

MultiTransientShaperControlDialog::MultiTransientShaperControlDialog(MultiTransientShaperControls* controls) :
	EffectControlDialog(controls)
{
	setAutoFillBackground(true);
	QPalette pal;
	pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	setPalette(pal);
	setFixedSize(600, 600);

	connect(getGUI()->mainWindow(), SIGNAL(periodicUpdate()), this, SLOT(updateDisplay()));

	auto makeKnob = [this](int x, int y, const QString& label, const QString& hintText, const QString& unit, FloatModel* model, bool isVolume)
	{
		Knob* newKnob = new Knob(KnobType::Bright26, this);
		newKnob->move(x, y);
		newKnob->setModel(model);
		newKnob->setLabel(label);
		newKnob->setHintText(hintText, unit);
		newKnob->setVolumeKnob(isVolume);
		return newKnob;
	};

	makeKnob( 16,  10, tr("1/2 Split"), tr("Band 1/2 Split Frequency:"), "Hz", &controls->m_band12SplitModel, false);
	makeKnob( 57,  10, tr("2/3 Split"), tr("Band 2/3 Split Frequency:"), "Hz", &controls->m_band23SplitModel, false);
	makeKnob( 98,  10, tr("Out"), tr("Output Gain:"), "%", &controls->m_outGainModel, true);
	
	makeKnob( 16, 120, tr("Gain 1"), tr("Band 1 Gain:"), "%", &controls->m_gain1Model, true);
	makeKnob( 57, 120, tr("FA 1"), tr("Band 1 Fast Attack: "), "s", &controls->m_fastEnv1AModel, false);
	makeKnob( 98, 120, tr("SA 1"), tr("Band 1 Slow Attack: "), "s", &controls->m_slowEnv1AModel, false);

	makeKnob( 16, 185, tr("Gain 2"), tr("Band 2 Gain:"), "%", &controls->m_gain2Model, true);
	makeKnob( 57, 185, tr("FA 2"), tr("Band 2 Fast Attack: "), "s", &controls->m_fastEnv2AModel, false);
	makeKnob( 98, 185, tr("SA 2"), tr("Band 2 Slow Attack: "), "s", &controls->m_slowEnv2AModel, false);

	makeKnob( 16, 240, tr("Gain 3"), tr("Band 3 Gain:"), "%", &controls->m_gain3Model, true);
	makeKnob( 57, 240, tr("FA 3"), tr("Band 3 Fast Attack: "), "s", &controls->m_fastEnv3AModel, false);
	makeKnob( 98, 240, tr("SA 3"), tr("Band 3 Slow Attack: "), "s", &controls->m_slowEnv3AModel, false);

	for(int i = 0; i < 3; i++)
	{
		auto y = 120 + (55 * i);
		makeKnob(98 + (41 * 1), y, tr("IG"), tr("Band Initial Gain"), "%", (controls->m_iGainModels)[i], true);
		makeKnob(98 + (41 * 2), y, tr("MG"), tr("Band Middle Gain"), "%", (controls->m_mGainModels)[i], true);
		makeKnob(98 + (41 * 3), y, tr("TG"), tr("Band Tail Gain"), "%", (controls->m_tGainModels)[i], true);
		makeKnob(98 + (41 * 4), y, tr("IT"), tr("Band Initial Time"), "ms", (controls->m_iTimeModels)[i], false);
		makeKnob(98 + (41 * 5), y, tr("MT"), tr("Band Middle Time"), "ms", (controls->m_mTimeModels)[i], false);
		makeKnob(98 + (41 * 6), y, tr("TT"), tr("Band Tail Time"), "ms", (controls->m_tTimeModels)[i], false);
		makeKnob(98 + (41 * 7), y, tr("TOL"), tr("Band Tolerance"), "", (controls->m_tolModels)[i], false);
	}

	m_controls = controls;
}

void MultiTransientShaperControlDialog::paintEvent(QPaintEvent *event)
{
	if (!isVisible()) { return; }

	QPainter p;
	p.begin(this);

	QPen aPen(QColor(255, 255, 0, 255), 1);
	p.setPen(aPen);
	for(int b = 0; b < 3; b++)
	{
		for(int i = 0; i < 2; i++)
		{
			const int starty = 250 + (40 * b) + (18 * i);
			const int endy = starty + 18;
			const int x = 50 + (m_controls->m_r_resGains[b][i] * 300);
			p.drawLine(x, starty, x, endy);
		}
	}

	p.end();
}

void MultiTransientShaperControlDialog::updateDisplay()
{
	update();
}

} // namespace lmms::gui
