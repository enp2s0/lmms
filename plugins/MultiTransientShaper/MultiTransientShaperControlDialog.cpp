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
	//setAutoFillBackground(true);
	//QPalette pal;
	//pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	//setPalette(pal);
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

	auto makeComboBox = [this](int x, int y, const QString& label, const QString& hintText, ComboBoxModel* model)
	{
		auto newBox = new ComboBox(this);
		newBox->move(x, y);
		newBox->setModel(model);
		return newBox;
	};
	auto makePixmapButton = [this](const QString& text, QWidget* parent, int x, int y, BoolModel* model, const QString& activeIcon, const QString& inactiveIcon, const QString& tooltip)
	{
		auto button = new PixmapButton(parent, text);
		button->move(x, y);
		button->setCheckable(true);
		if (model) { button->setModel(model); }
		button->setActiveGraphic(PLUGIN_NAME::getIconPixmap(activeIcon));
		button->setInactiveGraphic(PLUGIN_NAME::getIconPixmap(inactiveIcon));
		button->setToolTip(tooltip);
		return button;
	};

	makeKnob( 16,  10, tr("1/2 Split"), tr("Band 1/2 Split Frequency:"), "Hz", &controls->m_band12SplitModel, false);
	makeKnob( 57,  10, tr("2/3 Split"), tr("Band 2/3 Split Frequency:"), "Hz", &controls->m_band23SplitModel, false);
	makeKnob( 98,  10, tr("Out"), tr("Output Gain:"), "%", &controls->m_outGainModel, true);
	makeKnob(139,  10, tr("LKHD"), tr("Lookahead:"), "ms", &controls->m_lookaheadAmtModel, true);

	for(int i = 0; i < 3; i++)
	{
		auto y = 65 + (55 * i);
		makeKnob(    16 + (41 *  1), y, tr("G"), tr("Band ") + QString(i) + tr(" Gain:"), "%", controls->m_gainModels[0], true);
		makeKnob(    16 + (41 *  2), y, tr("SA"), tr("Band ") + QString(i) + tr(" Slow Attack: "), "s", controls->m_slowEnvAModels[i], false);
		makeKnob(    16 + (41 *  3), y, tr("IG"), tr("Band ") + QString(i) + tr(" Initial Gain"), "%", (controls->m_iGainModels)[i], true);
		makeKnob(    16 + (41 *  4), y, tr("MG"), tr("Band ") + QString(i) + tr(" Middle Gain"), "%", (controls->m_mGainModels)[i], true);
		makeKnob(    16 + (41 *  5), y, tr("TG"), tr("Band ") + QString(i) + tr(" Tail Gain"), "%", (controls->m_tGainModels)[i], true);
		makeKnob(    16 + (41 *  6), y, tr("IT"), tr("Band ") + QString(i) + tr(" Initial Time"), "ms", (controls->m_iTimeModels)[i], false);
		makeKnob(    16 + (41 *  7), y, tr("MT"), tr("Band ") + QString(i) + tr(" Middle Time"), "ms", (controls->m_mTimeModels)[i], false);
		makeKnob(    16 + (41 *  8), y, tr("TT"), tr("Band ") + QString(i) + tr(" Tail Time"), "ms", (controls->m_tTimeModels)[i], false);
		makeKnob(    16 + (41 *  9), y, tr("TOL"), tr("Band ") + QString(i) + tr(" Tolerance"), "", (controls->m_tolModels)[i], false);
		makeComboBox(16 + (41 * 10), y, tr("SRC"), tr("Trigger ") + QString(i) + tr(" Source"), (controls->m_tSources)[i]);
		makePixmapButton(tr("Band ") + QString(i) + tr("Mono Trigger"), this, 98 + (41 * 11), y, (controls->m_monoTriggers)[i], "crossover_led_green", "crossover_led_off", tr("Mono Trigger"));
	}

	// save the controls pointer so we can access it in paintEvent to render the meters.
	m_controls = controls;
}

void MultiTransientShaperControlDialog::paintEvent(QPaintEvent *event)
{
	if (!isVisible()) { return; }

	QPainter p;
	p.begin(this);

	QPen aPen(QColor(255, 255, 0, 255), 1);
	QPen cPen(QColor(255, 0, 255, 255), 1);
	QPen wPen(QColor(255, 255, 255, 200), 1);
	QPen bPen(QColor(0, 0, 255, 200), 1);
	for(int b = 0; b < 3; b++)
	{
		drawStereoBars(&p, &aPen, 50, 300 + (40 * b), 200, 36, m_controls->m_r_resGains[b], 0, 2);
		drawMonoBars(&p, &wPen, 50, 300 + (40 * b), 200, 36, 1, 0, 2);
		drawStereoBars(&p, &cPen, 50, 300 + (40 * b), 200, 36, m_controls->m_r_envDiffs[b], 0, 10);
		drawMonoBars(&p, &bPen, 50, 300 + (40 * b), 200, 36, m_controls->m_tolModels[b]->value(), 0, 10);

		drawEnv(&p, 50, 430 + (55 * b), 500, 50, 
			m_controls->m_lookaheadAmtModel.value(),
			m_controls->m_iTimeModels[b]->value(),
			m_controls->m_mTimeModels[b]->value(),
			m_controls->m_tTimeModels[b]->value(),
			m_controls->m_iGainModels[b]->value(),
			m_controls->m_mGainModels[b]->value(),
			m_controls->m_tGainModels[b]->value(),
			m_controls->m_r_times[b][0]);
	}

	p.end();
}

void MultiTransientShaperControlDialog::drawMonoBars(QPainter* p, QPen* pen, int startx, int starty, int width, int height, float val, float vmin, float vmax)
{
	float data[2];
	data[0] = val;
	data[1] = val;

	drawStereoBars(p, pen, startx, starty, width, height, data, vmin, vmax);
}

void MultiTransientShaperControlDialog::drawStereoBars(QPainter* p, QPen* pen, int startx, int starty, int width, int height, float* val, float vmin, float vmax)
{
	const float bar_height = height / 2;

	p->setPen(*pen);

	for(int i = 0; i < 2; i++)
	{
		const int y = starty + (bar_height * i);
		const int offset = ((val[i] - vmin) / vmax) * width;
		p->drawLine(startx + offset, y, startx + offset, y + bar_height);
	}
}

void MultiTransientShaperControlDialog::drawEnv(QPainter* p, int startx, int starty, int width, int height, float lhd, float it, float mt, float tt, float ig, float mg, float tg, float time)
{
	QPen yPen(QColor(255, 255, 0, 255), 1);
	QPen pPen(QColor(255, 0, 255, 255), 1);
	QPen wPen(QColor(255, 255, 255, 200), 1);
	QPen gPen(QColor(255, 255, 255, 100), 1);
	QPen bPen(QColor(0, 0, 255, 200), 1);

	it += lhd;
	mt += it;
	tt += mt;

	float gain_to_px = height / 2;
	float ms_to_px = width / tt;
	float zero_gain = starty + height;

	float p1_x = startx + (lhd * ms_to_px);
	float p2_x = startx + (it * ms_to_px);
	float p3_x = startx + (mt * ms_to_px);
	float p4_x = startx + (tt * ms_to_px);
	
	time *= ms_to_px;

	ig = ig / 100 * gain_to_px;
	mg = mg / 100 * gain_to_px;
	tg = tg / 100 * gain_to_px;

	p->setPen(yPen);
	p->drawLine(startx, zero_gain - gain_to_px, p1_x, zero_gain - ig);
	p->drawLine(p1_x, zero_gain - ig, p2_x, zero_gain - mg);
	p->drawLine(p2_x, zero_gain - mg, p3_x, zero_gain - mg);
	p->drawLine(p3_x, zero_gain - mg, p4_x, zero_gain - tg);

	p->setPen(wPen);
	p->drawLine(startx + time, starty, startx + time, starty + height);

	p->setPen(gPen);
	p->drawLine(startx, zero_gain - gain_to_px, startx + width, zero_gain - gain_to_px);
	p->drawLine(p1_x, starty, p1_x, starty + height);
}

void MultiTransientShaperControlDialog::updateDisplay()
{
	update();
}

} // namespace lmms::gui
