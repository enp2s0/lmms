/*
 * MultiTransientShaperControls.cpp - controls for MultiTransientShaper effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QDomElement>

#include "MultiTransientShaperControls.h"
#include "MultiTransientShaper.h"

namespace lmms
{

MultiTransientShaperControls::MultiTransientShaperControls(MultiTransientShaperEffect* effect) :
	EffectControls(effect),
	m_effect(effect),
	m_band12SplitModel(200.0f, 20.0f, 20000.0f, 1.0f, this, tr("Band 1/2 Split Frequency")),
	m_band23SplitModel(2500.0f, 20.0f, 20000.0f, 1.0f, this, tr("Bans 2/3 Split Frequency")),
	m_outGainModel(100.0f, 0.0f, 200.0f, 0.1f, this, tr("Output Gain")),
	m_lookaheadAmtModel(5.0f, 0.0f, 20.0f, 0.1f, this, tr("Lookahead"))
{
	for(int i = 0; i < 3; i++)
	{
		m_gainModels[i] = new FloatModel(100.0f, 0.0f, 200.0f, 0.1f, this, tr("Band ") + QString(i) + tr(" Gain"));
		m_slowEnvAModels[i] = new FloatModel(500.0f, 0.0f, 1000.0f, 0.1f, this, tr("Band ") + QString(i) + tr(" Envelope Follower Attack"));

		m_iGainModels[i] = new FloatModel(100.0f, 0.0f,  200.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Initial Gain"));
		m_mGainModels[i] = new FloatModel(100.0f, 0.0f,  200.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Middle Gain"));
		m_tGainModels[i] = new FloatModel(100.0f, 0.0f,  200.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Tail Gain"));
		m_iTimeModels[i] = new FloatModel( 50.0f, 0.0f, 1500.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Initial Time"));
		m_mTimeModels[i] = new FloatModel(200.0f, 0.0f, 3000.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Middle Time"));
		m_tTimeModels[i] = new FloatModel(500.0f, 0.0f, 3000.0f, 0.1, this, tr("Band ") + QString(i) + tr(" Tail Time"));
		m_tolModels[i] = new FloatModel(0.1f, 0.0f, 10.0f, 0.01f, this, tr("Band ") + QString(i) + tr(" Transient Tolerance"));

		m_tSources[i] = new ComboBoxModel(this, tr("Band ") + QString(i) + tr(" Trigger Source"));
		m_tSources[i]->addItem("Band 1");
		m_tSources[i]->addItem("Band 2");
		m_tSources[i]->addItem("Band 3");
		m_tSources[i]->addItem("Input");

		m_monoTriggers[i] = new BoolModel(this);
	}
}


void MultiTransientShaperControls::loadSettings(const QDomElement& parent)
{
	m_band12SplitModel.loadSettings(parent, "band12Split");
	m_band23SplitModel.loadSettings(parent, "band23Split");
	m_outGainModel.loadSettings(parent, "outGain");
}


void MultiTransientShaperControls::saveSettings(QDomDocument& doc, QDomElement& parent)
{
	m_band12SplitModel.saveSettings(doc, parent, "band12Split");
	m_band23SplitModel.saveSettings(doc, parent, "band23Split");
	m_outGainModel.saveSettings(doc, parent, "outGain");
}


} // namespace lmms
