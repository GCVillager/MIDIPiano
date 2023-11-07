#include "MIDIQt.h"
MIDIQt::MIDIQt(QWidget* parent)
	: QMainWindow(parent)
{
	if (access("record", 0) == -1)  //判断该文件夹是否存在
	{
		int ret=mkdir("record");
		//其实根本不想接收返回值，创建不了也没关系，但是它非得给我标警告
	}

	midiOutOpen(&hMidi, 0, 0, 0, 0);//打开midi设备，必须在使用hMidi之前！！！（惨痛）
	//开始初始化UI
	ui.setupUi(this);
	this->setFixedSize(601, 338);
	//接下来一系列代码都是初始化下拉列表
	//写入大小调与各大小调式
	ui.comboMajorMinor->addItems({ "大调","小调" });
	ui.comboScale->addItems({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" });
	//写入乐器种类，由于激活了信号，乐器编号的框不需要在这里写加入
	for (int i = 0; i < insTypeCnt; i++)
	{
		ui.comboInsType->addItems({ insType[i].second.c_str() });
	}
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

	int volumePlay = volume::getVolume100(channelPool::PLAY);
	ui.sliderVolume->setValue(volumePlay);

	int volumeReplay = volume::getVolume100(channelPool::REPLAY);
	ui.sliderReplayVolume->setValue(volumeReplay);

	int delay = play::getDelay(channelPool::PLAY);
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
	{
		ui.comboIns->addItems({ insList[i].second.c_str() });
	}
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
	//两个音量控制共用一个槽函数
	int volumePlay = ui.sliderVolume->value();
	int volumeReplay = ui.sliderReplayVolume->value();
	volume::setVolume(channelPool::PLAY,volumePlay);
	volume::setVolume(channelPool::REPLAY, volumeReplay);
	ui.labelCurVolume->setText(("当前音量：" + std::to_string(volumePlay)).c_str());
	ui.labelReplayVolume->setText(("当前音量：" + std::to_string(volumeReplay)).c_str());
	//改变音量后显示在lable上
}

void MIDIQt::changeDelay()
{
	int delay = ui.spinDelay->value();
	play::setDelay(channelPool::PLAY,delay);
	replay::setRecordDelay(delay);
}

void MIDIQt::saveCfg()
{
	writeCfg("config.cfg");
}

void MIDIQt::startRecord()
{
	//改变按键状态防止乱按
	ui.pushButtonStartRecord->setEnabled(false);
	ui.pushButtonStopGiveUp->setEnabled(true);
	ui.pushButtonStopSave->setEnabled(true);
	replay::setRecordStatus(true);
}
void MIDIQt::stopRecordSave()
{
	ui.pushButtonStartRecord->setEnabled(true);
	ui.pushButtonStopGiveUp->setEnabled(false);
	ui.pushButtonStopSave->setEnabled(false);
	//改变按键状态防止乱按
	replay::setRecordStatus(false);
	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());//获取当前时间
	std::stringstream ssTmp;
	ssTmp << std::put_time(std::localtime(&time), "%F_%H-%M-%S");//格式化时间
	std::string defaultName ="record/"+ssTmp.str() + ".midrec";//拿时间作为默认文件名
	QString QSNewName = QFileDialog::getSaveFileName(
		this,
		"保存记录文件",
		defaultName.c_str(),
		"midi电子琴记录文件(*.midrec)"
	);//打开保存窗口
	std::string newName = std::string(QSNewName.toLocal8Bit());
	if (newName == "")//如果被取消了
	{
		replay::clearWriteBuff();//清空缓存。相当于执行结束并放弃
		return;
	}
	replay::writeFile(newName);
}
void MIDIQt::stopRecordGiveUp()
{
	ui.pushButtonStartRecord->setEnabled(true);
	ui.pushButtonStopGiveUp->setEnabled(false);
	ui.pushButtonStopSave->setEnabled(false);
	replay::setRecordStatus(false);
	replay::clearWriteBuff();
	//改变按键状态，清除缓存
}
void MIDIQt::openRecord()
{
	QString filePath = QFileDialog::getOpenFileName(
		this,
		"打开记录文件",
		"record",
		"midi电子琴记录文件(*.midrec)"
	);//打开文件
	std::string SFilePath = std::string(filePath.toLocal8Bit());
	if (SFilePath == "")//如果没有打开就退出
	{
		return;
	}
	replay::stopReplay(hMidi);
	replay::readFile(SFilePath);
	QFileInfo fileInfo = QFileInfo(filePath);
	QString fileName = fileInfo.fileName();//获取文件名
	ui.labelNowPlaying->setText(QString("当前打开文件：")+fileName);
	ui.pushButtonStartReplay->setEnabled(true);
}
void MIDIQt::startReplay()
{
	ui.pushButtonStopReplay->setEnabled(true);
	std::thread(replay::startReplay, hMidi).detach();
}
void MIDIQt::stopReplay()
{
	replay::stopReplay(hMidi);
	ui.pushButtonStopReplay->setEnabled(false);
	//存在问题：播放结束后不会自动灰掉。不影响功能故暂不修复
}
//slot functions end


void MIDIQt::keyPressEvent(QKeyEvent* event)//重写键盘函数
{
	int key = event->key();
	//如果是被定义为琴键的字母按键且是第一次按下
	if (!event->isAutoRepeat() && key >= 'A' && key <= 'Z' &&
		key != 'I' && key != 'O' && key != 'P' && key != 'K' && key != 'L')
	{
		play::playNote(hMidi,channelPool::PLAY, key, true);//播放这个声音
	}
}

void MIDIQt::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	//如果是被定义为琴键的字母按键且是真的被松开
	if (!event->isAutoRepeat() && key >= 'A' && key <= 'Z' &&
		key != 'I' && key != 'O' && key != 'P' && key != 'K' && key != 'L')
	{
		play::playNote(hMidi, channelPool::PLAY,key, false);//停止这个声音
	}
}


