#ifndef SLIDER_H
#define SLIDER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qslider.h>
#include <qboxlayout.h>

class Slider : public QWidget

{
  Q_OBJECT

  private:
    QSlider *slider;
    QLabel *valueLabel, *minLabel, *maxLabel;
    
  public:
    Slider(int minValue, int maxValue, int pageStep, int value, 
           Qt::Orientation orientation, QWidget * parent);
    ~Slider();
    int value();
    
  signals:
    void valueChanged(int);
    
  public slots:
    void setValue(int);
    
  private slots:
    void updateLabel(int);  
};
  
#endif
