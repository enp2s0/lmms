/*
 * MultiTransientShaperControls.h - controls for bassboosterx -effect
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

#ifndef LMMS_MultiTransientShaper_CONTROLS_H
#define LMMS_MultiTransientShaper_CONTROLS_H

#include "EffectControls.h"
#include "MultiTransientShaperControlDialog.h"

namespace lmms
{

class MultiTransientShaperEffect;

namespace gui
{
class MultiTransientShaperControlDialog;
}

class MultiTransientShaperControls : public EffectControls
{
	Q_OBJECT
public:
	MultiTransientShaperControls(MultiTransientShaperEffect* effect);
	~MultiTransientShaperControls() override = default;

	void saveSettings(QDomDocument& doc, QDomElement& parent) override;
	void loadSettings(const QDomElement& parent) override;
	inline QString nodeName() const override
	{
		return "MultiTransientShaperControls";
	}
	gui::EffectControlDialog* createView() override
	{
		return new gui::MultiTransientShaperControlDialog(this);
	}
	int controlCount() override { return 4; }

private:
	MultiTransientShaperEffect* m_effect;
	FloatModel m_band12SplitModel;
	FloatModel m_band23SplitModel;
	FloatModel m_outGainModel;
	FloatModel m_lookaheadAmtModel;

	FloatModel* m_gainModels[3];
	FloatModel* m_slowEnvAModels[4];

	FloatModel* m_iGainModels[3];
	FloatModel* m_mGainModels[3];
	FloatModel* m_tGainModels[3];
	FloatModel* m_iTimeModels[3];
	FloatModel* m_mTimeModels[3];
	FloatModel* m_tTimeModels[3];
	FloatModel* m_tolModels[3];

	ComboBoxModel* m_tSources[3];
	BoolModel* m_monoTriggers[3];

	// GUI return paramters for UI updates
	float m_r_envDiffs[4][2];
	float m_r_resGains[4][2];
	float m_r_times[4][2];

	friend class gui::MultiTransientShaperControlDialog;
	friend class MultiTransientShaperEffect;
};

} // namespace lmms

#endif // LMMS_MultiTransientShaper_CONTROLS_H
