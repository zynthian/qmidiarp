/*!
 * @file lfowidget_lv2.cpp
 * @brief Implements the the LV2 GUI for the QMidiArp Lfo plugin.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */

#include "lfowidget_lv2.h"

#include <unistd.h>
#include <ctime>

#ifndef COMPACT_STYLE
#define COMPACT_STYLE "QLabel { font: 7pt; } \
    QComboBox { font: 7pt; max-height: 15px;} \
    QToolButton { max-height: 20px;} \
    QSpinBox { font: 7pt; max-height: 20px;} \
    QCheckBox { font: 7pt; max-height: 20px;} \
    QGroupBox { font: 7pt; }"

#endif

LfoWidgetLV2::LfoWidgetLV2 (
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function,
        const LV2_Feature *const *host_features)
        : LfoWidget(NULL, NULL, 1, true, true, true, "LFO-LV2", 0)
{
    m_controller = ct;
    writeFunction = write_function;

    /* Scan host features for URID map */

    LV2_URID_Map *urid_map;
    for (int i = 0; host_features[i]; ++i) {
        if (::strcmp(host_features[i]->URI, LV2_URID_URI "#map") == 0) {
            urid_map = (LV2_URID_Map *) host_features[i]->data;
            if (urid_map) {
                (void)urid_map->map(urid_map->handle, LV2_MIDI_EVENT_URI);
                break;
            }
        }
    }
    if (!urid_map) {
        qWarning("Host does not support urid:map.");
        return;
    }

    lv2_atom_forge_init(&forge, urid_map);

    /* Map URIS */
    QMidiArpURIs* const uris = &m_uris;
    map_uris(urid_map, uris);

    transportBox = new QCheckBox(this);
    QLabel *transportBoxLabel = new QLabel(tr("&Sync with Host"),this);
    transportBoxLabel->setBuddy(transportBox);
    transportBox->setToolTip(tr("Sync to Transport from Host"));
    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(120);
    tempoSpin->setKeyboardTracking(false);
    tempoSpin->setToolTip(tr("Tempo of internal clock"));
    connect(transportBox, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(transportBox, SIGNAL(toggled(bool)), tempoSpin, SLOT(setDisabled(bool)));
    transportBox->setChecked(false);

    inOutBox->layout()->addWidget(transportBoxLabel);
    inOutBox->layout()->addWidget(transportBox);
    inOutBox->layout()->addWidget(tempoSpin);

    connect(amplitude,          SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(offset,             SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(resBox,             SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(sizeBox,            SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(freqBox,            SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(waveFormBox,        SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(loopBox,            SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(channelOut,         SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(chIn,               SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(ccnumberBox,        SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(ccnumberInBox,      SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(tempoSpin,          SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));

    connect(muteOutAction,      SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableNoteOff,      SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigByKbd,    SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigLegato,   SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(recordAction,       SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));

    connect(this, SIGNAL(mouseSig(double, double, int, int))
            , this, SLOT(mapMouse(double, double, int, int)));

    setStyleSheet(COMPACT_STYLE);

    res = 4;
    size = 1;
    mouseXCur = 0.0;
    mouseYCur = 0.0;
    sendUIisUp(true);
}

LfoWidgetLV2::~LfoWidgetLV2()
{
    sendUIisUp(false);
}

void LfoWidgetLV2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{
    const QMidiArpURIs* uris = &m_uris;
    LV2_Atom* atom = (LV2_Atom*)buffer;

    if (!data.count()) sendUIisUp(true);

    if (format == uris->atom_eventTransfer
      && atom->type == uris->atom_Blank) {
        receiveWave(atom);
    }
    else if (format == 0 && buffer_size == sizeof(float)) {


        float fValue = *(float *) buffer;
        res = resBox->currentText().toInt();
        size = sizeBox->currentText().toInt();

        switch (port_index) {
            case AMPLITUDE:
                    amplitude->setValue(fValue);
            break;
            case OFFSET:
                    offset->setValue(fValue);
            break;
            case RESOLUTION:
                    resBox->setCurrentIndex(fValue);
            break;
            case SIZE:
                    sizeBox->setCurrentIndex(fValue);
            break;
            case FREQUENCY:
                    freqBox->setCurrentIndex(fValue);
            break;
            case CH_OUT:
                    channelOut->setCurrentIndex(fValue);
            break;
            case CH_IN:
                    chIn->setCurrentIndex(fValue);
            break;
            case CURSOR_POS:
                    cursor->updateNumbers(res, size);
                    cursor->updatePosition(fValue);
                    cursor->update();
            break;
            case WAVEFORM:
                    waveFormBox->setCurrentIndex(fValue);
            break;
            case LOOPMODE:
                    loopBox->setCurrentIndex(fValue);
            break;
            case MUTE:
                    muteOutAction->setChecked((bool)fValue);
                    screen->setMuted(fValue);
                    screen->update();
            break;
            case MOUSEX:
            case MOUSEY:
            case MOUSEBUTTON:
            case MOUSEPRESSED:
            break;
            case CC_OUT:
                    ccnumberBox->setValue(fValue);
            break;
            case CC_IN:
                    ccnumberInBox->setValue(fValue);
            break;
            case ENABLE_NOTEOFF:
                    enableNoteOff->setChecked((fValue > .5));
            break;
            case ENABLE_RESTARTBYKBD:
                    enableRestartByKbd->setChecked((bool)fValue);
            break;
            case ENABLE_TRIGBYKBD:
                    enableTrigByKbd->setChecked((bool)fValue);
            break;
            case ENABLE_TRIGLEGATO:
                    enableTrigLegato->setChecked((bool)fValue);
            break;
            case RECORD:
                    recordAction->setChecked((bool)fValue);
            break;
            case DEFER:
                    deferChangesAction->setChecked((bool)fValue);
            break;
            case TRANSPORT_MODE:
                    transportBox->setChecked((bool)fValue);
            break;
            case TEMPO:
                    tempoSpin->setValue((int)fValue);
            break;
            default:
            break;
        }
    }
}

void LfoWidgetLV2::sendUIisUp(bool on)
{
    const QMidiArpURIs* uris = &m_uris;
    uint8_t obj_buf[64];
    int state;

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);

    /* prepare forge buffer and initialize atom-sequence */
    lv2_atom_forge_set_buffer(&forge, obj_buf, 16);

    if (on) state = uris->ui_up; else state=uris->ui_down;

    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_blank(&forge, &frame, 1, state);

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &frame);
    writeFunction(m_controller, MidiIn, lv2_atom_total_size(msg), uris->atom_eventTransfer, msg);
}

void LfoWidgetLV2::receiveWave(LV2_Atom* atom)
{
    QMidiArpURIs* const uris = &m_uris;
    if (atom->type != uris->atom_Blank) return;

    /* cast the buffer to Atom Object */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
    LV2_Atom *a0 = NULL;
    lv2_atom_object_get(obj, uris->hex_customwave, &a0, NULL);
    if (obj->body.otype != uris->hex_customwave) return;

    /* handle wave' data vector */
    LV2_Atom_Vector* voi = (LV2_Atom_Vector*)LV2_ATOM_BODY(a0);
    /* check if atom is indeed a vector of the expected type*/
    if (voi->atom.type != uris->atom_Int) return;

    /* get number of elements in vector
    * = (raw 8bit data-length - header-length) / sizeof(expected data type:int) */
    const size_t n_elem = (a0->size - sizeof(LV2_Atom_Vector_Body)) / voi->atom.size;
    /* typecast, dereference pointer to vector */
    const int *recdata = (int*) LV2_ATOM_BODY(&voi->atom);
    int ofs = 127;
    for (uint l1 = 0; l1 < n_elem; l1++) {
        receiveWavePoint(l1, recdata[l1]);
        if ((recdata[l1] < ofs) && (l1 < n_elem - 1)) ofs = recdata[l1];
    }
    if (n_elem < (uint)data.count()) data.resize(res * size + 1);
    if (waveFormBox->currentIndex() == 5) {
        offset->valueChangedSignalSuppressed = true;
        offset->setValue(ofs);
        offset->valueChangedSignalSuppressed = false;
    }
    screen->updateData(data);
    screen->update();
}

void LfoWidgetLV2::receiveWavePoint(int index, int value)
{
    Sample sample;
    if (value < 0) {
        sample.muted = true;
        value = -value;
    }
    else sample.muted = false;
    sample.value = value;
    sample.tick = index * TPQN / res;
    if (index >= data.count()) data.append(sample);
    else data.replace(index, sample);
}

void LfoWidgetLV2::mapBool(bool on)
{
    float value = (float)on;
    if (muteOutAction == sender())              updateParam(MUTE, value);
    else if (enableNoteOff == sender())         updateParam(ENABLE_NOTEOFF, value);
    else if (enableRestartByKbd == sender())    updateParam(ENABLE_RESTARTBYKBD, value);
    else if (enableTrigByKbd == sender())       updateParam(ENABLE_TRIGBYKBD, value);
    else if (enableTrigLegato == sender())      updateParam(ENABLE_TRIGLEGATO, value);
    else if (recordAction == sender())          updateParam(RECORD, value);
    else if (deferChangesAction == sender())    updateParam(DEFER, value);
    else if (transportBox == sender())          updateParam(TRANSPORT_MODE, value);
}

void LfoWidgetLV2::mapMouse(double mouseX, double mouseY, int buttons, int pressed)
{
    updateParam(MOUSEX, mouseX);
    updateParam(MOUSEY, mouseY);
    updateParam(MOUSEBUTTON, buttons);
    updateParam(MOUSEPRESSED, pressed); //mouseMoved or pressed
    /* in case we get a mouse click and waveform is not "custom", the
     * backend will perform copyToCustom(), and we have to set the
     * index to custom here as well */
    if (pressed && waveFormBox->currentIndex() != 5) {
        waveFormBox->setCurrentIndex(5);
        updateWaveForm(5);
    }
    if (!pressed) updateParam(WAVEFORM, 5);
}

void LfoWidgetLV2::mapParam(int value)
{
    if (amplitude == sender())          updateParam(AMPLITUDE, value);
    else if (offset == sender())        updateParam(OFFSET, value);
    else if (resBox == sender())        updateParam(RESOLUTION, value);
    else if (sizeBox == sender())       updateParam(SIZE, value);
    else if (freqBox == sender())       updateParam(FREQUENCY, value);
    else if (channelOut == sender())    updateParam(CH_OUT, value);
    else if (chIn == sender())          updateParam(CH_IN, value);
    else if (waveFormBox == sender())   updateParam(WAVEFORM, value);
    else if (loopBox == sender())       updateParam(LOOPMODE, value);
    else if (ccnumberBox == sender())   updateParam(CC_OUT, value);
    else if (ccnumberInBox == sender()) updateParam(CC_IN, value);
    else if (tempoSpin == sender())     updateParam(TEMPO, value);
}

void LfoWidgetLV2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}