/*
 * MultiTransientShaper.h - MultiTransientShaper-effect-plugin
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

#ifndef LMMS_MultiTransientShaper_H
#define LMMS_MultiTransientShaper_H

#include "Effect.h"
#include "MultiTransientShaperControls.h"
#include "BasicFilters.h"

namespace lmms
{

class MultiTransientShaperEffect : public Effect
{
public:
	MultiTransientShaperEffect(Model* parent, const Descriptor::SubPluginFeatures::Key* key);
	~MultiTransientShaperEffect() override = default;
	bool processAudioBuffer(sampleFrame* buf, const fpp_t frames) override;
	int lhbIndex(int sample, int band, int channel);


	EffectControls* controls() override
	{
		return &m_mtsControls;
	}

private:
	MultiTransientShaperControls m_mtsControls;

	float m_sampleRate;

	StereoLinkwitzRiley m_lp12;
	StereoLinkwitzRiley m_lp23;
	StereoLinkwitzRiley m_hp12;
	StereoLinkwitzRiley m_hp23;

	StereoLinkwitzRiley* m_bandlimit[3];

	float m_fastEnv[3][2];
	float m_slowEnv[3][2];
	float m_bandState[3][2];

	float* m_lhb;
	int m_lookaheadLen;
	int m_lookaheadRead;
	int m_lookaheadWrite;

	friend class MultiTransientShaperControls;
};

} // namespace lmms

#endif // LMMS_MultiTransientShaper_H
