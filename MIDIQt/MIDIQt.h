#pragma once

#include <vector>
#include <QtWidgets/QMainWindow>
#include <QKeyEvent>
#include <QFileDialog>
#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ui_MIDIQt.h"

#include "midi.h"
#include "instrument.h"
#include "fileCfg.h"
#include "replay.h"

#ifndef slots
#define slots Q_SLOTS
#endif

static HMIDIOUT hMidi;
const int scaleMapping[2][12] = {
	{
		note::MJ_C, note::MJ_CS, note::MJ_D, note::MJ_DS, note::MJ_E, note::MJ_F,
		note::MJ_FS, note::MJ_G, note::MJ_GS, note::MJ_A, note::MJ_AS, note::MJ_B,
	},
	{
		note::MN_C, note::MN_CS, note::MN_D,note::MN_DS, note::MN_E, note::MN_F,
		note::MN_FS, note::MN_G, note::MN_GS,note::MN_A, note::MN_AS, note::MN_B
	}
};

class MIDIQt : public QMainWindow
{
	Q_OBJECT

public:
	explicit MIDIQt(QWidget* parent = nullptr);
	~MIDIQt();
private:
	Ui::MIDIQtClass ui;
	void refreshUI();

protected:
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

public slots:
	void changeScale();
	void changeInsType();
	void changeIns();
	void changeVolume();
	void changeDelay();
	void saveCfg();
	void startRecord();
	void stopRecordSave();
	void stopRecordGiveUp();
	void openRecord();
	void startReplay();
	void stopReplay();
};
