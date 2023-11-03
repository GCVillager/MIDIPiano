#include "MIDIQt.h"
MIDIQt::MIDIQt(QWidget* parent)
	: QMainWindow(parent)
{
	//开始初始化UI
	ui.setupUi(this);
	this->setFixedSize(601, 338);
	//接下来一系列代码都是初始化下拉列表
	//写入大小调与各大小调式
	ui.comboMajorMinor->addItems({ "大调","小调" });
	ui.comboScale->addItems({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" });
	//写入乐器种类，由于激活了信号，乐器编号的框不需要在这里写加入
	for (int i = 0; i < insTypeCnt; i++)
		ui.comboInsType->addItems({ insType[i].second.c_str() });
	midiOutOpen(&hMidi, 0, 0, 0, 0);//打开midi设备，必须在使用hMidi之前！！！
	applyCfg(hMidi, "config.cfg");//读取文件配置并同步到内核
	refreshUI();//从内核同步到UI
	

	this->grabKeyboard();//开启键盘监听

}

MIDIQt::~MIDIQt()
{}

void MIDIQt::refreshUI()
{//从文件读取后，需要使用这个刷新一下GUI
	int scale = note::getScale();
	const int mappingTransform[24] = {
	0,1,2,3,4,5,6,7,8,9,10,11,
	9,10,11,0,1,2,3,4,5,6,7,8
	};//由于scale里的表和combo里的表不一样，需要映射
	//这是由curScale映射到combo
	ui.comboMajorMinor->setCurrentIndex(scale / 12);
	ui.comboScale->setCurrentIndex(mappingTransform[scale]);

	int ins = instrument::getIns();
	ui.comboInsType->setCurrentIndex(ins/ 8);//模8是第一个列表，乐器种类
	ui.comboIns->setCurrentIndex(ins % 8);//是第二个列表，乐器

	int volume = volume::getVolume100();
	ui.sliderVolume->setValue(volume);

	int delay = play::getDelay();
	ui.spinDelay->setValue(delay);
}

//slot functions start
void MIDIQt::changeScale()
{
	int MjMnIndex = ui.comboMajorMinor->currentIndex();//大小调对应的combo的index
	int scaleIndex = ui.comboScale->currentIndex();//调式对应combo的index
	int newScale = scaleMapping[MjMnIndex][scaleIndex];
	note::setScale(newScale);
}

void MIDIQt::changeInsType()
{
	ui.comboIns->clear();//清空上一组乐器
	int newInsType = ui.comboInsType->currentIndex();
	for (int i = insType[newInsType].first; i < insType[newInsType + 1].first; i++)
		ui.comboIns->addItems({ insList[i].second.c_str() });
	int newIns = newInsType * 8;//8个一组，刚好
	instrument::setIns(hMidi, newIns);
}

void MIDIQt::changeIns()
{
	int instrumentType = ui.comboInsType->currentIndex();
	int insIndex = ui.comboIns->currentIndex();
	instrument::setIns(hMidi, instrumentType * 8 + insIndex);//8个一组，x*8+y即是乐器编号
}

void MIDIQt::changeVolume()
{
	int volume = ui.sliderVolume->value();
	volume::setVolume(volume);
	ui.labelCurVolume->setText(("当前音量：" + to_string(volume)).c_str());
	//改变音量后显示在lable上
}

void MIDIQt::changeDelay()
{
	int delay = ui.spinDelay->value();
	play::setDelay(delay);
}

void MIDIQt::saveCfg()
{
	writeCfg("config.cfg");
}
//slot functions end


void MIDIQt::keyPressEvent(QKeyEvent* event)//重写键盘函数
{
	int key = event->key();
	//如果是被定义为琴键的字母按键且是第一次按下
	if (!event->isAutoRepeat() && key >= 'A' && key <= 'Z' &&
		key != 'I' && key != 'O' && key != 'P' && key != 'K' && key != 'L')
		play::playNote(hMidi, key, true);//播放这个声音
}

void MIDIQt::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	//如果是被定义为琴键的字母按键且是第一次按下
	if (!event->isAutoRepeat() && key >= 'A' && key <= 'Z' &&
		key != 'I' && key != 'O' && key != 'P' && key != 'K' && key != 'L')
		play::playNote(hMidi, key, false);//停止这个声音
}


