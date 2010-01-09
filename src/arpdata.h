#ifndef ARPDATA_H
#define ARPDATA_H

#include <QWidget>
#include <QDockWidget>
#include <QList>
#include <alsa/asoundlib.h>
#include "seqdriver.h"
#include "midiarp.h"
#include "arpwidget.h"
#include "midilfo.h"
#include "lfowidget.h"
#include "midiseq.h"
#include "seqwidget.h"

class ArpData : public QWidget  {
    
  Q_OBJECT

  private:
    QList<MidiArp *> midiArpList;
    QList<ArpWidget *> arpWidgetList;
    QList<QDockWidget *> moduleWindowList;
    QList<MidiLfo *> midiLfoList;
    QList<LfoWidget *> lfoWidgetList;
    QList<MidiSeq *> midiSeqList;
    QList<SeqWidget *> seqWidgetList;
    int portCount;
    bool modified;
    int mute_ccnumber, midiLearnID, midiLearnWindowID, midiLearnModuleID;
    bool midi_mutable, midiLearnFlag;
    
  public:
    SeqDriver *seqDriver;

    ArpData(QWidget* parent=0);
    ~ArpData();
    void registerPorts(int num);
    int getPortCount();
    bool isModified();


    void addModuleWindow(QDockWidget *moduleWindow);
    void removeModuleWindow(QDockWidget *moduleWindow);
    QDockWidget *moduleWindow(int index);
    int moduleWindowCount();
    void updateIDs(int curID);
    
    void addMidiArp(MidiArp *midiArp);
    void addArpWidget(ArpWidget *arpWidget);
    void removeMidiArp(MidiArp *midiArp);
    void removeArpWidget(ArpWidget *arpWidget);
    int midiArpCount();
    int arpWidgetCount();
    MidiArp *midiArp(int index);
    ArpWidget *arpWidget(int index);
    
    void addMidiLfo(MidiLfo *midiLfo);
    void addLfoWidget(LfoWidget *lfoWidget);
    void removeMidiLfo(MidiLfo *midiLfo);
    void removeLfoWidget(LfoWidget *lfoWidget);
    int midiLfoCount();
    int lfoWidgetCount();   
    MidiLfo *midiLfo(int index);
    LfoWidget *lfoWidget(int index);
    
    void addMidiSeq(MidiSeq *midiSeq);
    void addSeqWidget(SeqWidget *seqWidget);
    void removeMidiSeq(MidiSeq *midiSeq);
    void removeSeqWidget(SeqWidget *seqWidget);
    int midiSeqCount();
    int seqWidgetCount();   
    MidiSeq *midiSeq(int index);
    SeqWidget *seqWidget(int index);
    int getAlsaClientId();
    
  public slots:
    void runQueue(bool);
    void setModified(bool);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void handleController(int ccnumber, int channel, int value);
    void setMidiLearn(int moduleWindowID, int moduleID, int controlID);
};
                              
#endif
