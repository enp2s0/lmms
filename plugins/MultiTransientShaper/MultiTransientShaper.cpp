/*
 * MultiTransientShaper.cpp - A native MultiTransientShaper effect plugin with sample-exact amplification
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

#include "MultiTransientShaper.h"

#include "embed.h"
#include "plugin_export.h"
#include <iostream>

namespace lmms
{

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT multitransientshaper_plugin_descriptor =
{
	LMMS_STRINGIFY(PLUGIN_NAME),
	"Multiband Transient Shaper",
	QT_TRANSLATE_NOOP("PluginBrowser", "A native 3-band transient shaper plugin."),
	"Noah Brecht <noahb2713/at/gmail/dot/com>",
	0x0100,
	Plugin::Type::Effect,
	new PluginPixmapLoader("logo"),
	nullptr,
	nullptr,
} ;

}


MultiTransientShaperEffect::MultiTransientShaperEffect(Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
	Effect(&multitransientshaper_plugin_descriptor, parent, key),
	m_mtsControls(this),
	m_sampleRate(Engine::audioEngine()->processingSampleRate()),
	m_lp12(m_sampleRate),
	m_lp23(m_sampleRate),
	m_hp12(m_sampleRate),
	m_hp23(m_sampleRate),
	m_ap(m_sampleRate)
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			m_fastEnv[i][j] = 0;
			m_slowEnv[i][j] = 0;
			m_bandState[i][j] = 0;
		}

		m_bandlimit[i] = new StereoLinkwitzRiley(m_sampleRate);
		m_bandlimit[i]->setLowpass(400);
	}

	// static 20ms of lookahead
	m_lookaheadLen = m_sampleRate * 0.02;
	m_lhb = new float[m_lookaheadLen * 3 * 2];
	for(int i = 0; i < m_lookaheadLen * 3 * 2; i++)
		m_lhb[i] = 0;
	m_lookaheadRead = 0;
	m_lookaheadWrite = 0;

	m_ap.setFilterType(BasicFilters<2>::FilterType::AllPass);
}

int MultiTransientShaperEffect::lhbIndex(int sample, int band, int channel)
{
	// 4 bands/sample and 2 channels/band;
	return (((sample * 3) + band) * 2) + channel;
}

bool MultiTransientShaperEffect::processAudioBuffer(sampleFrame* buf, const fpp_t frames)
{
	if (!isEnabled() || !isRunning()) { return false ; }

	double outSum = 0.0;
	const float d = dryLevel();
	const float w = wetLevel();

	// read parameters & controls
	const float band12Split = m_mtsControls.m_band12SplitModel.value();
	const float band23Split = m_mtsControls.m_band23SplitModel.value();
	const float outGain = m_mtsControls.m_outGainModel.value() * 0.01f;
	const int lookahead = m_mtsControls.m_lookaheadAmtModel.value() * m_sampleRate / 1000;
	auto bandGains = std::array{
		m_mtsControls.m_gainModels[0]->value() * 0.01f,
		m_mtsControls.m_gainModels[1]->value() * 0.01f,
		m_mtsControls.m_gainModels[2]->value() * 0.01f
	};
	auto bandTols = std::array{
		m_mtsControls.m_tolModels[0]->value(),
		m_mtsControls.m_tolModels[1]->value(),
		m_mtsControls.m_tolModels[2]->value()
	};
	auto bandEnvAttacks = std::array{
		m_mtsControls.m_slowEnvAModels[0]->value(),
		m_mtsControls.m_slowEnvAModels[1]->value(),
		m_mtsControls.m_slowEnvAModels[2]->value()
	};
	auto bandRespGains = std::array{
		std::array{
			m_mtsControls.m_iGainModels[0]->value() / 100,
			m_mtsControls.m_mGainModels[0]->value() / 100,
			m_mtsControls.m_tGainModels[0]->value() / 100
		},
		std::array{
			m_mtsControls.m_iGainModels[1]->value() / 100,
			m_mtsControls.m_mGainModels[1]->value() / 100,
			m_mtsControls.m_tGainModels[1]->value() / 100
		},
		std::array{
			m_mtsControls.m_iGainModels[2]->value() / 100,
			m_mtsControls.m_mGainModels[2]->value() / 100,
			m_mtsControls.m_tGainModels[2]->value() / 100
		}
	};
	auto bandRespTimes = std::array{
		std::array{
			m_mtsControls.m_iTimeModels[0]->value() * m_sampleRate / 1000,
			m_mtsControls.m_mTimeModels[0]->value() * m_sampleRate / 1000,
			m_mtsControls.m_tTimeModels[0]->value() * m_sampleRate / 1000
		},
		std::array{
			m_mtsControls.m_iTimeModels[1]->value() * m_sampleRate / 1000,
			m_mtsControls.m_mTimeModels[1]->value() * m_sampleRate / 1000,
			m_mtsControls.m_tTimeModels[1]->value() * m_sampleRate / 1000
		},
		std::array{
			m_mtsControls.m_iTimeModels[2]->value() * m_sampleRate / 1000,
			m_mtsControls.m_mTimeModels[2]->value() * m_sampleRate / 1000,
			m_mtsControls.m_tTimeModels[2]->value() * m_sampleRate / 1000
		}
	};
	auto bandMonoModes = std::array{
		m_mtsControls.m_monoTriggers[0]->value(),
		m_mtsControls.m_monoTriggers[1]->value(),
		m_mtsControls.m_monoTriggers[2]->value()
	};
	auto bandSources = std::array{
		m_mtsControls.m_tSources[0]->value(),
		m_mtsControls.m_tSources[1]->value(),
		m_mtsControls.m_tSources[2]->value()
	};

	// Set cutoff frequencies of band split filters.
	m_lp12.setLowpass(band12Split);
	m_hp12.setHighpass(band12Split);
	m_lp23.setLowpass(band23Split);
	m_hp23.setHighpass(band23Split);
	m_ap.calcFilterCoeffs(band23Split, 0.70710678118);

	// Adjust response times to be cumulative
	for(int b = 0; b < 3; b++)
	{
		bandRespTimes[b][0] += lookahead / m_sampleRate;
		bandRespTimes[b][1] += bandRespTimes[b][0];
		bandRespTimes[b][2] += bandRespTimes[b][1];
	}

	for (fpp_t f = 0; f < frames; ++f)
	{
		auto s = std::array{buf[f][0], buf[f][1]};
		m_lookaheadWrite = (m_lookaheadRead + lookahead) % m_lookaheadLen;

		// Split the input signal into 3 frequency bands.
		std::array<std::array<float, 2>, 3> bands = {{}};
		for(int i = 0; i < 2; i++)
		{
			// Low band is lowpassed at 1/2 freq.
			// It is also allpassed to add 180deg of phase shift to match
			// the other filters.
			bands[0][i] = m_lp12.update(s[i], i);
			bands[0][i] = m_ap.update(bands[0][i], i);

			// Mid band is highpassed at 1/2 freq and lowpassed at 2/3 freq.
			bands[1][i] = m_hp12.update(s[i], i);
			bands[1][i] = m_lp23.update(bands[1][i], i);

			// High band is highpassed at 2/3 freq.
			bands[2][i] = m_hp23.update(s[i], i);

			// write each band into the lookahead buffer
			for(int b = 0; b < 3; b++)
				m_lhb[lhbIndex(m_lookaheadWrite, b, i)] = bands[b][i];
		}

		// iterate through each filter band
		for(int b = 0; b < 3; b++)
		{
			float crestFactor[2];
			auto bs_l = std::array{m_lhb[lhbIndex(m_lookaheadRead, b, 0)], m_lhb[lhbIndex(m_lookaheadRead, b, 1)]};

			// update each envelope follower for this band
			for(int i = 0; i < 2; i++)
			{
				// choose the correct source to follow
				float v;
				if(bandSources[b] == 3) // special case for unfiltered input
					v = std::abs(s[i]);
				else
					v = std::abs(bands[bandSources[b]][i]);

				// convert and scale attack times from ms to per-sample decay
				float slowA = std::pow(0.01, 1.0 / (bandEnvAttacks[b] * m_sampleRate * 0.001));

				// step the followers forward
				m_fastEnv[b][i] = v; //fastA * (m_fastEnv[b][i] - v) + v;
				m_slowEnv[b][i] = slowA * (m_slowEnv[b][i] - v) + v;

				// calculate the difference between the fast and slow followers
				crestFactor[i] = std::clamp(m_fastEnv[b][i] / m_slowEnv[b][i], 0.0f, 50.0f);

				// if the difference exceeds the tolerance, we have a transient in this band,
				// so reset the band state counter.
				// don't reset if we're still in the attack phase to prevent spurious triggers
				if(crestFactor[i] > bandTols[b] && m_bandState[b][i] > (lookahead + bandRespTimes[b][0]))
				{
					if(bandMonoModes[b] == true)
					{
						// reset both channels if mono.
						m_bandState[b][0] = 0;
						m_bandState[b][1] = 0;
					}
					else
					{
						m_bandState[b][i] = 0;
					}
				}
				
				// define a helper function to map time ranges to gain ranges
				auto remap = [this](float x, float in_min, float in_max, float out_min, float out_max)
				{
					return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
				};

				float resGain;
				if(m_bandState[b][i] < lookahead)                resGain = remap(m_bandState[b][i], 0, lookahead, 1, bandRespGains[b][0]);
				else if(m_bandState[b][i] < bandRespTimes[b][0]) resGain = remap(m_bandState[b][i], lookahead, bandRespTimes[b][0], bandRespGains[b][0], bandRespGains[b][1]);
				else if(m_bandState[b][i] < bandRespTimes[b][1]) resGain = bandRespGains[b][1];
				else if(m_bandState[b][i] < bandRespTimes[b][2]) resGain = remap(m_bandState[b][i], bandRespTimes[b][1], bandRespTimes[b][2], bandRespGains[b][1], bandRespGains[b][2]);
				else                                             resGain = bandRespGains[b][2]; 

				// Lowpass the gain signal. Extremely rapid changes can cause
				// audible clicking in the output, so smooth them here.
				resGain = m_bandlimit[b]->update(resGain, i);

				// now we know the proper gain, but we need to apply it to the
				// sample in the lookahead buffer, not the one we're currently
				// working with.
				bs_l[i] *= resGain;

				// apply master band gain
				bs_l[i] *= (bandGains[b]);

				// step the state counter forward for the next sample
				m_bandState[b][i] += 1;

				// pass data back to the UI code
				m_mtsControls.m_r_envDiffs[b][i] = crestFactor[i];
				m_mtsControls.m_r_resGains[b][i] = resGain;
				m_mtsControls.m_r_times[b][i] = m_bandState[b][i] * 1000 / m_sampleRate;
			}

			bands[b][0] = bs_l[0];
			bands[b][1] = bs_l[1];
		}

		// apply output stage gain
		s[0] = (bands[0][0] + bands[1][0] + bands[2][0]) * outGain;
		s[1] = (bands[0][1] + bands[1][1] + bands[2][1]) * outGain;

		// advance the lookahead buffer
		m_lookaheadRead = (m_lookaheadRead + 1) % m_lookaheadLen;

		buf[f][0] = d * buf[f][0] + w * s[0];
		buf[f][1] = d * buf[f][1] + w * s[1];
		outSum += buf[f][0] * buf[f][0] + buf[f][1] * buf[f][1];
	}

	checkGate(outSum / frames);

	return isRunning();
}


extern "C"
{

// necessary for getting instance out of shared lib
PLUGIN_EXPORT Plugin* lmms_plugin_main(Model* parent, void* data)
{
	return new MultiTransientShaperEffect(parent, static_cast<const Plugin::Descriptor::SubPluginFeatures::Key*>(data));
}

}

} // namespace lmms
