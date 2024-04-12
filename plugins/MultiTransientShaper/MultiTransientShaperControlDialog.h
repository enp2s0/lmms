/*
 * MultiTransientShaperControlDialog.h - control dialog for MultiTransientShaper effect
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

#ifndef LMMS_GUI_MultiTransientShaper_CONTROL_DIALOG_H
#define LMMS_GUI_MultiTransientShaper_CONTROL_DIALOG_H

#include "EffectControlDialog.h"
#include "embed.h"
#include "GuiApplication.h"
#include "Knob.h"
#include "ComboBox.h"
#include "PixmapButton.h"
#include "MainWindow.h"

namespace lmms
{

class MultiTransientShaperControls;
class FloatModel;

namespace gui
{

class Knob;

class MultiTransientShaperControlDialog : public EffectControlDialog
{
	Q_OBJECT
public:
	MultiTransientShaperControlDialog(MultiTransientShaperControls* controls);
	~MultiTransientShaperControlDialog() override = default;
	void paintEvent(QPaintEvent *event);

	MultiTransientShaperControls* m_controls;
	
private:
	void drawStereoBars(QPainter* p, QPen* pen, int startx, int starty, int width, int height, float* val, float vmin, float vmax);
	void drawMonoBars(QPainter* p, QPen* pen, int startx, int starty, int width, int height, float val, float vmin, float vmax);
	void drawEnv(QPainter* p, int startx, int starty, int width, int height, float lhd, float it, float mt, float tt, float ig, float mg, float tg, float time, float gain);

private slots:
	void updateDisplay();
};

} // namespace gui

} // namespace lmms

#endif // LMMS_GUI_MultiTransientShaper_CONTROL_DIALOG_H
